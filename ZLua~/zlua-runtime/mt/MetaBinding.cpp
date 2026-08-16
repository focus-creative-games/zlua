// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "MetaBinding.h"
#include "InstanceTarget.h"
#include "TypeRegistryCommon.h"
#if ZLUA_FAST_METATABLE
#include "FastMetatable.h"
#endif

#include "../LuaConsts.h"
#include "../lvm/LuaEnv.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaException.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../utils/CsStringHash.h"
#include "../bridge/PropertyBridge.h"
#include "../bridge/MethodBridge.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/MethodOverloadResolver.h"
#include "AliasXmlTable.h"
#include "ExtensionXmlTable.h"

#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/MetadataCache.h"
#include "vm/Runtime.h"
#include "vm/Method.h"
#include "vm/Type.h"
#include "vm/Object.h"
#include "vm/Parameter.h"
#include "gc/GarbageCollector.h"
#include "il2cpp-tabledefs.h"

#include <cstring>
#include "metadata/GenericMetadata.h"

namespace zlua
{
using MethodGroupMap = std::unordered_map<const char*, std::vector<const MethodInfo*>, CsStringHash, CsStringEqual>;

static std::unordered_map<Il2CppClass*, TypeBinding*> s_bindings;

static std::unordered_set<const char*, CsStringHash, CsStringEqual> s_methodAliasKeys;

/// Method-scoped marshal payload shared across byObj/byVal (and duplicate CreateMethodMarshalCtx calls).
struct MethodMarshalShared
{
    const MarshalMetaInfo** paramsMeta;
    const MarshalMetaInfo* retMeta;
    const MethodDefaultArgs* defaults;
    FnLua2CsInvoker lua2CsInvoker;
    int32_t luaArity;
    bool isExtension;
};

static std::unordered_map<const MethodInfo*, MethodMarshalShared*> s_methodMarshalShared;

static const char* CopyRuntimeAliasKey(const std::string& key)
{
    auto it = s_methodAliasKeys.find(key.c_str());
    if (it != s_methodAliasKeys.end())
        return *it;
    const char* newKey = zlua_strdup(key.c_str());
    s_methodAliasKeys.insert(newKey);
    return newKey;
}

static int CreateMethodDispatchClosureRef(lua_State* L, TypeBinding* binding, MethodGroups* groups, lua_CFunction dispatchFunc)
{
    lua_pushnumber(L, (lua_Number)(int)ClosureKind::Indirect);
    lua_pushlightuserdata(L, binding);
    lua_pushlightuserdata(L, groups);
    lua_pushcclosure(L, dispatchFunc, 3);
    return LuaUtil::ToLuaRef(L);
}

constexpr int kUpvalueIndexClosureKind = 1;
constexpr int kUpvalueIndexBinding = 2;
constexpr int kUpvalueIndexGroupsOrCtx = 3;

static int InvokeStaticMethodDirect(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const MethodMarshalCtx* ctx = (const MethodMarshalCtx*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    return MethodBridge::InvokeLua2Cs(L, nullptr, 1, ctx);
    ZLUA_TRY_END();
}

static int InvokeInstanceMethodDirect(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const MethodMarshalCtx* ctx = (const MethodMarshalCtx*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    void* target = ctx->resolveThis(L, 1);
    return MethodBridge::InvokeLua2Cs(L, target, 2, ctx);
    ZLUA_TRY_END();
}

/// Extension static-as-instance: validate arity as instance (after self), invoke as static with argStart=1.
static int InvokeExtensionMethodCore(lua_State* L, const MethodMarshalCtx* ctx)
{
    const int top = lua_gettop(L);
    const int available = top >= 2 ? (top - 2 + 1) : 0;
    const int32_t minArity = GetMinLuaArity(ctx);
    if (available < minArity)
    {
        if (minArity == ctx->luaArity)
            LuaException::ThrowFormat("zlua: argument mismatch: expected %d argument(s), got %d", minArity, available);
        LuaException::ThrowFormat("zlua: argument mismatch: expected %d..%d argument(s), got %d", minArity, ctx->luaArity, available);
    }
    const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->method, nullptr, /*sealed*/ true);
    return ctx->lua2CsInvoker(L, nullptr, /*argStart*/ 1, method, ctx);
}

static int InvokeExtensionMethodDirect(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const MethodMarshalCtx* ctx = (const MethodMarshalCtx*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    return InvokeExtensionMethodCore(L, ctx);
    ZLUA_TRY_END();
}

static bool TryMaterializeDefaultParam(const MethodInfo* method, int paramIndex, const MarshalMetaInfo* meta, void** outValueSlot, Il2CppObject** outObjectSlot)
{
    *outValueSlot = nullptr;
    *outObjectSlot = nullptr;
    if ((method->parameters[paramIndex]->attrs & PARAM_ATTRIBUTE_HAS_DEFAULT) == 0)
        return false;

    bool isExplicitNull = false;
    Il2CppObject* obj = il2cpp::vm::Parameter::GetDefaultParameterValueObject(method, paramIndex, &isExplicitNull);
    if (obj == nullptr && !isExplicitNull)
        return false;

    if (meta->passByValue)
    {
        *outObjectSlot = obj;
        return true;
    }

    void* buf = LuaMetadataAlloc::MallocZeroed(static_cast<size_t>(meta->size));
    if (obj != nullptr)
    {
        void* unboxed = ZLuaObjectUnbox(obj);
        std::memcpy(buf, unboxed, static_cast<size_t>(meta->size));
    }
    *outValueSlot = buf;
    return true;
}

MethodMarshalCtx* MetaBinding::CreateMethodMarshalCtx(lua_State* L, const MethodInfo* method, bool isByVal)
{
    // paramsMeta / retMeta / defaults / luaArity / invoker / isExtension 与 byVal 无关，按 MethodInfo* 共享。
    // byVal / resolveThis / sealed 仍每份 MethodMarshalCtx 独立（struct byobj+byval 双建时只分叉薄壳）。
    MethodMarshalShared* shared = nullptr;
    auto sharedIt = s_methodMarshalShared.find(method);
    if (sharedIt != s_methodMarshalShared.end())
    {
        shared = sharedIt->second;
    }
    else
    {
        shared = LuaMetadataAlloc::MallocAnyZeroed<MethodMarshalShared>();
        shared->isExtension = MetadataUtil::IsExtensionMethod(method);
        shared->lua2CsInvoker = MethodBridge::ResolveMethodInvoker(method);
        shared->defaults = nullptr;
        shared->retMeta = nullptr;
        shared->paramsMeta = nullptr;
        shared->luaArity = 0;

        int32_t luaArity = 0;
        if (method->parameters_count > 0)
        {
            shared->paramsMeta = LuaMetadataAlloc::CallocArray<const MarshalMetaInfo*>(method->parameters_count);
            for (int i = 0; i < method->parameters_count; i++)
            {
                const MarshalMetaInfo* meta = MarshalMeta::Create(L, method, i);
                shared->paramsMeta[i] = meta;
                // Extension: luaArity excludes CLR param 0 (this) — matches IMT argStart=2.
                if (!shared->isExtension || i >= 1)
                    luaArity += meta->stackSlots > 0 ? meta->stackSlots : 1;
            }
        }
        shared->luaArity = luaArity;

        // Trailing C# defaults → dense MethodDefaultArgs (Il2Cpp perf: no blob parse on call).
        if (method->parameters_count > 0)
        {
            const int paramStart = shared->isExtension ? 1 : 0;
            bool trailingHasDefault = false;
            for (int i = method->parameters_count - 1; i >= paramStart; --i)
            {
                if ((method->parameters[i]->attrs & PARAM_ATTRIBUTE_HAS_DEFAULT) == 0)
                    break;
                bool isExplicitNull = false;
                const Il2CppType* defaultType = nullptr;
                const char* data = il2cpp::vm::Method::GetParameterDefaultValue(method, i, &defaultType, &isExplicitNull);
                if (data == nullptr && !isExplicitNull)
                    break;
                trailingHasDefault = true;
                break;
            }

            if (trailingHasDefault)
            {
                void** tempValues = (void**)alloca(method->parameters_count * sizeof(void*));
                Il2CppObject** tempObjects = (Il2CppObject**)alloca(method->parameters_count * sizeof(Il2CppObject*));
                std::memset(tempValues, 0, method->parameters_count * sizeof(void*));
                std::memset(tempObjects, 0, method->parameters_count * sizeof(Il2CppObject*));

                int omitSlots = 0;
                int firstDefault = -1;
                bool anyObjectDefault = false;
                for (int i = method->parameters_count - 1; i >= paramStart; --i)
                {
                    void* valueSlot = nullptr;
                    Il2CppObject* objectSlot = nullptr;
                    if (!TryMaterializeDefaultParam(method, i, shared->paramsMeta[i], &valueSlot, &objectSlot))
                        break;

                    firstDefault = i;
                    tempValues[i] = valueSlot;
                    tempObjects[i] = objectSlot;
                    if (shared->paramsMeta[i]->passByValue)
                        anyObjectDefault = true;
                    omitSlots += shared->paramsMeta[i]->stackSlots > 0 ? shared->paramsMeta[i]->stackSlots : 1;
                }

                IL2CPP_ASSERT(firstDefault >= 0);
                const uint8_t defaultCount = static_cast<uint8_t>(method->parameters_count - firstDefault);
                void** valueSlots = LuaMetadataAlloc::CallocArray<void*>(defaultCount);
                Il2CppObject** objectSlots = nullptr;
                if (anyObjectDefault)
                    objectSlots = (Il2CppObject**)il2cpp::gc::GarbageCollector::AllocateFixed(defaultCount * sizeof(Il2CppObject*), nullptr);

                for (uint8_t di = 0; di < defaultCount; ++di)
                {
                    const int paramIndex = firstDefault + di;
                    valueSlots[di] = tempValues[paramIndex];
                    if (objectSlots != nullptr)
                        objectSlots[di] = tempObjects[paramIndex];
                }

                MethodDefaultArgs* defaults = LuaMetadataAlloc::MallocAnyZeroed<MethodDefaultArgs>();
                defaults->minLuaArity = luaArity - omitSlots;
                if (defaults->minLuaArity < 0)
                    defaults->minLuaArity = 0;
                defaults->firstDefaultParamIndex = static_cast<uint8_t>(firstDefault);
                defaults->defaultParamCount = defaultCount;
                defaults->defaultValueSlots = valueSlots;
                defaults->defaultObjectSlots = objectSlots;
                shared->defaults = defaults;
                shared->lua2CsInvoker = MethodBridge::DefaultInvokeLuaMethod;
            }
        }

        if (!MetadataUtil::IsVoidType(method->return_type))
            shared->retMeta = MarshalMeta::Create(L, method, -1);

        bool needsStackCursor = false;
        for (int i = 0; i < method->parameters_count; i++)
        {
            const MarshalMetaInfo* meta = shared->paramsMeta[i];
            if (meta->marshalType == LuaMarshalType::Table || meta->marshalType == LuaMarshalType::UnpackedValues || meta->stackSlots != 1)
            {
                needsStackCursor = true;
                break;
            }
        }
        if (!needsStackCursor && shared->retMeta != nullptr &&
            (shared->retMeta->marshalType == LuaMarshalType::Table || shared->retMeta->marshalType == LuaMarshalType::UnpackedValues || shared->retMeta->stackSlots != 1))
        {
            needsStackCursor = true;
        }
        if (needsStackCursor)
            shared->lua2CsInvoker = MethodBridge::DefaultInvokeLuaMethod;

        s_methodMarshalShared.insert({method, shared});
    }

    const bool isStatic = MetadataUtil::IsStaticMethod(method);
    FnResolveMethodThis resolveThis = shared->isExtension ? nullptr : InstanceTarget::GetResolveMethodThisFunc(method->klass, isStatic, isByVal);
    MethodMarshalCtx* ctx = LuaMetadataAlloc::MallocAnyZeroed<MethodMarshalCtx>();
    ctx->method = method;
    ctx->resolveThis = resolveThis;
    ctx->lua2CsInvoker = shared->lua2CsInvoker;
    ctx->paramsMeta = shared->paramsMeta;
    ctx->retMeta = shared->retMeta;
    ctx->luaArity = shared->luaArity;
    ctx->defaults = shared->defaults;
    ctx->byVal = isByVal;
    ctx->sealed = shared->isExtension ? true : MetadataUtil::IsMethodSealed(method, isByVal);
    ctx->isExtension = shared->isExtension;
    return ctx;
}

static int InvokeMethodDirectGeneric(lua_State* L)
{
    return luaL_error(L, "zlua: static generic methods are not supported yet");
}

int MetaBinding::CreateDirectMethodClosureRef(lua_State* L, const MethodInfo* method, const TypeBinding* binding, bool isStatic, bool isByVal)
{
    MethodMarshalCtx* ctx = CreateMethodMarshalCtx(L, method, isByVal);
    lua_pushnumber(L, (lua_Number)(int)(method->is_generic ? ClosureKind::DirectGeneric : ClosureKind::Direct));
    lua_pushlightuserdata(L, (void*)binding);
    lua_pushlightuserdata(L, ctx);
    lua_CFunction dispatchFunc;
    if (method->is_generic)
        dispatchFunc = InvokeMethodDirectGeneric;
    else if (ctx->isExtension)
        dispatchFunc = InvokeExtensionMethodDirect;
    else if (isStatic)
        dispatchFunc = InvokeStaticMethodDirect;
    else
        dispatchFunc = InvokeInstanceMethodDirect;
    lua_pushcclosure(L, dispatchFunc, 3);
    return LuaUtil::ToLuaRef(L);
}

bool MetaBinding::IsDirectMethodClosure(lua_State* L, int closureStackIndex)
{
    lua_getupvalue(L, closureStackIndex, kUpvalueIndexClosureKind);
    ClosureKind kind = (ClosureKind)(int)lua_tonumber(L, -1);
    lua_pop(L, 1);
    return kind == ClosureKind::Direct || kind == ClosureKind::DirectGeneric;
}

const MethodMarshalCtx* MetaBinding::GetMethodMarshalCtxFromClosure(lua_State* L, int closureStackIndex)
{
    lua_getupvalue(L, closureStackIndex, kUpvalueIndexGroupsOrCtx);
    MethodMarshalCtx* ctx = (MethodMarshalCtx*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return ctx;
}

const TypeBinding* MetaBinding::GetTypeBindingFromClosure(lua_State* L, int closureStackIndex)
{
    lua_getupvalue(L, closureStackIndex, kUpvalueIndexBinding);
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return binding;
}

static const char* PeekAnyMethodName(const MethodGroups* groups)
{
    for (size_t i = 0; i <= kMaxSmallArgCount; ++i)
    {
        const MethodGroup* group = groups->smallArgCountMethodGroups[i];
        if (group != nullptr && group->methodCount > 0 && group->methods[0] != nullptr && group->methods[0]->method != nullptr)
            return group->methods[0]->method->name;
    }
    if (groups->largeArgCountMethodCount > 0 && groups->largeArgCountMethods[0] != nullptr && groups->largeArgCountMethods[0]->method != nullptr)
        return groups->largeArgCountMethods[0]->method->name;
    return nullptr;
}

static const MethodMarshalCtx* FindBestMethod(lua_State* L, const TypeBinding* binding, const MethodGroups* groups, int argStart)
{
    MethodOverloadResolutionResult result = MethodOverloadResolver::Resolve(L, groups, argStart, lua_gettop(L) - argStart + 1);
    if (result.kind == MethodOverloadResolutionKind::None)
    {
        std::string typeName = MetadataUtil::GetSignatureTypeName(binding->klass);
        const char* methodName = PeekAnyMethodName(groups);
        if (methodName != nullptr)
            LuaException::ThrowFormat("zlua: no matching overload of %s.%s", typeName.c_str(), methodName);
        LuaException::ThrowFormat("zlua: no matching overload for type: %s", typeName.c_str());
    }
    if (result.kind == MethodOverloadResolutionKind::Ambiguous)
    {
        IL2CPP_ASSERT(result.method != nullptr && result.method->method != nullptr);
        std::string typeName = MetadataUtil::GetSignatureTypeName(binding->klass);
        LuaException::ThrowFormat("zlua: ambiguous method found: %s.%s", typeName.c_str(), result.method->method->name);
    }
    const MethodMarshalCtx* ctx = result.method;
    IL2CPP_ASSERT(ctx != nullptr);
    return ctx;
}

static int InvokeStaticMethodDispatch(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const TypeBinding* binding = (const TypeBinding*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexBinding));
    const MethodGroups* groups = (const MethodGroups*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    const int argStart = 1;
    const MethodMarshalCtx* ctx = FindBestMethod(L, binding, groups, argStart);

    return MethodBridge::InvokeLua2Cs(L, nullptr, argStart, ctx);
    ZLUA_TRY_END();
}

static int InvokeInstanceMethodDispatch(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const TypeBinding* binding = (const TypeBinding*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexBinding));
    const MethodGroups* groups = (const MethodGroups*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    const int argStart = 2;
    const MethodMarshalCtx* ctx = FindBestMethod(L, binding, groups, argStart);

    if (ctx->isExtension)
        return InvokeExtensionMethodCore(L, ctx);

    void* target = ctx->resolveThis(L, 1);
    return MethodBridge::InvokeLua2Cs(L, target, argStart, ctx);
    ZLUA_TRY_END();
}

static void CollectBindableMethods(Il2CppClass* klass, std::vector<const MethodInfo*>& ctorMethods, std::vector<const MethodInfo*>& staticMethods,
                                   std::vector<const MethodInfo*>& instanceMethods)
{
    MetadataUtil::EnsureMethods(klass);
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const MethodInfo* method = klass->methods[i];
        if (!MetadataUtil::IsPublicMethod(method))
            continue;
        if (MetadataUtil::IsCCtor(method))
        {
            continue;
        }
        if (MetadataUtil::IsCtor(method))
        {
            ctorMethods.push_back(method);
            continue;
        }
        if (MetadataUtil::IsStaticMethod(method))
        {
            staticMethods.push_back(method);
        }
        else
        {
            instanceMethods.push_back(method);
        }
    }
}

bool MetaBinding::TryRegisterMethodAlias(lua_State* L, Il2CppClass* klass, bool isStatic, const std::string& aliasName, int closureStackIndex)
{
    if (aliasName.empty())
        return false;

    TypeBinding* binding = EnsureBinding(L, klass);
    NameMetaMap& map = isStatic ? binding->staticMap : binding->byobjInstanceMap;
    if (map.find(aliasName.c_str()) != map.end())
        return false;

#if ZLUA_FAST_METATABLE
    FastMetatable::RawSetMethodOnIndexTable(L, klass, isStatic, /*isByVal*/ false, aliasName.c_str(), closureStackIndex);
#endif

    lua_pushvalue(L, closureStackIndex);
    const int closureRef = LuaUtil::ToLuaRef(L);

    MetaInfo info = {};
    info.kind = MetaKind::Method;
    info.closureRef = closureRef;
    map[CopyRuntimeAliasKey(aliasName)] = info;
    return true;
}

static bool IsPublicField(const FieldInfo* field)
{
    return (il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) == FIELD_ATTRIBUTE_PUBLIC;
}

static void SetupFieldAddressOrInstanceOffset(FieldMarshalCtx& ctx, const FieldInfo* field, bool isStatic)
{
    if (isStatic)
        ctx.staticAddress = (uint8_t*)field->parent->static_fields + field->offset;
    else
        ctx.instanceOffsetIncludingHeader = field->offset;
}

static void RegisterFields(lua_State* L, Il2CppClass* klass, NameMetaMap& instanceMap, NameMetaMap& staticMap)
{
    MetadataUtil::EnsureFields(klass);
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        FieldInfo* field = klass->fields + i;
        if (!IsPublicField(field))
            continue;
        const bool isStatic = !il2cpp::vm::Field::IsInstance(field);
        NameMetaMap& map = isStatic ? staticMap : instanceMap;
        if (isStatic && !il2cpp::vm::Field::IsNormalStatic(field))
            continue;
        if (map.find(field->name) != map.end())
            continue;

        MetaInfo info = {};
        info.kind = MetaKind::Field;
        FieldMarshalCtx* ctx = LuaMetadataAlloc::MallocAnyZeroed<FieldMarshalCtx>();
        ctx->meta = MarshalMeta::Create(L, field);
        SetupFieldAddressOrInstanceOffset(*ctx, field, isStatic);
        info.field = ctx;
        map[field->name] = info;
    }
}

static void RegisterProperties(lua_State* L, Il2CppClass* klass, NameMetaMap& instanceMap, NameMetaMap& staticMap)
{
    // Tuanjie: klass->properties is PropertyInfo**; use GetProperties (SetupProperties + walk).
    void* propertyIter = nullptr;
    while (const PropertyInfo* property = il2cpp::vm::Class::GetProperties(klass, &propertyIter))
    {
        if (!MetadataUtil::IsPublicProperty(property) || !MetadataUtil::IsZeroParameterProperty(property))
            continue;

        const bool isStatic = MetadataUtil::IsStaticProperty(property);
        NameMetaMap& map = isStatic ? staticMap : instanceMap;
        if (map.find(property->name) != map.end())
            continue;

        MetaInfo info = {};
        info.kind = MetaKind::Property;

        PropertyMarshalCtx* pmCtx = LuaMetadataAlloc::MallocAnyZeroed<PropertyMarshalCtx>();
        pmCtx->property = property;
        const Il2CppType* type = MetadataUtil::GetPropertyReturnType(property);
        pmCtx->valueTypeKlass = il2cpp::vm::Class::FromIl2CppType(type, true);
        pmCtx->meta = MarshalMeta::Create(L, property);

        PropertyAccessor accessor = PropertyBridge::ResolvePropertyAccessor(property, isStatic);

        if (property->get != nullptr)
        {
            pmCtx->getter = accessor.getter;
            pmCtx->getterSealed = MetadataUtil::IsMethodSealed(property->get, /*byVal*/ false);
        }
        else
        {
            pmCtx->getter = nullptr;
            pmCtx->getterSealed = true;
        }
        if (property->set != nullptr)
        {
            pmCtx->setter = accessor.setter;
            pmCtx->setterSealed = MetadataUtil::IsMethodSealed(property->set, /*byVal*/ false);
        }
        else
        {
            pmCtx->setter = nullptr;
            pmCtx->setterSealed = true;
        }
        info.property = pmCtx;
        map[property->name] = info;
    }
}

static MethodGroups* CreateMethodGroups(lua_State* L, const std::vector<const MethodInfo*>& methods, bool isByVal)
{
    IL2CPP_ASSERT(methods.size() > 1);

    std::vector<const MethodMarshalCtx*> methodGroups[kMaxSmallArgCount + 1];
    std::vector<const MethodMarshalCtx*> largeArgCountMethods;

    for (const MethodInfo* method : methods)
    {
        const MethodMarshalCtx* ctx = MetaBinding::CreateMethodMarshalCtx(L, method, isByVal);
        bool addedLarge = false;
        for (int32_t arity = GetMinLuaArity(ctx); arity <= ctx->luaArity; ++arity)
        {
            if (arity <= static_cast<int32_t>(kMaxSmallArgCount))
            {
                methodGroups[arity].push_back(ctx);
            }
            else if (!addedLarge)
            {
                largeArgCountMethods.push_back(ctx);
                addedLarge = true;
            }
        }
    }

    size_t methodGroupCount = 0;
    size_t totalPtrCount = largeArgCountMethods.size();
    for (size_t i = 0; i <= kMaxSmallArgCount; i++)
    {
        if (!methodGroups[i].empty())
        {
            methodGroupCount++;
            totalPtrCount += methodGroups[i].size();
        }
    }

    // Single allocation: MethodGroups header + MethodGroup structs + all MethodMarshalCtx* arrays.
    const size_t blockSize = sizeof(MethodGroups) + methodGroupCount * sizeof(MethodGroup) + totalPtrCount * sizeof(const MethodMarshalCtx*);
    uint8_t* block = static_cast<uint8_t*>(LuaMetadataAlloc::MallocZeroed(blockSize));
    MethodGroups* groups = reinterpret_cast<MethodGroups*>(block);
    uint8_t* cursor = block + sizeof(MethodGroups);

    for (size_t i = 0; i <= kMaxSmallArgCount; i++)
    {
        std::vector<const MethodMarshalCtx*>& group = methodGroups[i];
        if (group.empty())
        {
            groups->smallArgCountMethodGroups[i] = nullptr;
            continue;
        }

        MethodGroup* mg = reinterpret_cast<MethodGroup*>(cursor);
        cursor += sizeof(MethodGroup);
        const MethodMarshalCtx** methodsArr = reinterpret_cast<const MethodMarshalCtx**>(cursor);
        cursor += group.size() * sizeof(const MethodMarshalCtx*);
        for (size_t j = 0; j < group.size(); j++)
            methodsArr[j] = group[j];
        mg->methods = methodsArr;
        mg->methodCount = group.size();
        groups->smallArgCountMethodGroups[i] = mg;
    }

    if (!largeArgCountMethods.empty())
    {
        const MethodMarshalCtx** methodsArr = reinterpret_cast<const MethodMarshalCtx**>(cursor);
        cursor += largeArgCountMethods.size() * sizeof(const MethodMarshalCtx*);
        for (size_t j = 0; j < largeArgCountMethods.size(); j++)
            methodsArr[j] = largeArgCountMethods[j];
        groups->largeArgCountMethods = methodsArr;
        groups->largeArgCountMethodCount = largeArgCountMethods.size();
    }

    IL2CPP_ASSERT(static_cast<size_t>(cursor - block) == blockSize);
    return groups;
}

static void SetupCtorMethod(lua_State* L, TypeBinding* binding, const std::vector<const MethodInfo*>& ctorMethods, bool isByVal)
{
    switch (ctorMethods.size())
    {
    case 0:
        binding->uniqueCtorMethod = nullptr;
        binding->ctorGroups = nullptr;
        break;
    case 1:
        binding->uniqueCtorMethod = MetaBinding::CreateMethodMarshalCtx(L, ctorMethods[0], isByVal);
        binding->ctorGroups = nullptr;
        break;
    default:
        binding->uniqueCtorMethod = nullptr;
        binding->ctorGroups = CreateMethodGroups(L, ctorMethods, isByVal);
        break;
    }
}

static void RegisterMethodsdWithSignature(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic, bool isByVal,
                                          NameMetaMap& map)
{
    for (const MethodInfo* method : overloads)
    {
        std::string methodSignature = MetadataUtil::CreateMethodNameWithParametersAndGenericArguments(method);
        if (map.find(methodSignature.c_str()) != map.end())
            continue;
        MetaInfo fullNameInfo = {};
        fullNameInfo.kind = MetaKind::Method;
        fullNameInfo.closureRef = MetaBinding::CreateDirectMethodClosureRef(L, method, binding, isStatic, isByVal);
        map[CopyRuntimeAliasKey(methodSignature)] = fullNameInfo;
    }
}

static void RegisterMethodGroup(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic, bool isByVal,
                                NameMetaMap& map)
{
    IL2CPP_ASSERT(overloads.size() > 0 && name != nullptr);
    if (overloads.size() == 1)
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        int closureRef = MetaBinding::CreateDirectMethodClosureRef(L, overloads[0], binding, isStatic, isByVal);
        info.closureRef = closureRef;
        map[name] = info;
    }
    else
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        MethodGroups* groups = CreateMethodGroups(L, overloads, isByVal);
        info.closureRef = CreateMethodDispatchClosureRef(L, binding, groups, isStatic ? InvokeStaticMethodDispatch : InvokeInstanceMethodDispatch);
        map[name] = info;

        RegisterMethodsdWithSignature(L, binding, name, overloads, isStatic, isByVal, map);
    }
}

static void RegisterGenericMethodGroup(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic, bool isByVal,
                                       NameMetaMap& map)
{
    IL2CPP_ASSERT(overloads.size() > 0 && name != nullptr);
    if (overloads.size() == 1 && map.find(name) == map.end())
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        int closureRef = MetaBinding::CreateDirectMethodClosureRef(L, overloads[0], binding, isStatic, isByVal);
        info.closureRef = closureRef;
        map[name] = info;
    }
    RegisterMethodsdWithSignature(L, binding, name, overloads, isStatic, isByVal, map);
}

static void AddToMethodGroup(const std::vector<const MethodInfo*>& methods, MethodGroupMap& methodGroups, MethodGroupMap& genericMethods)
{
    std::string alias;
    for (const MethodInfo* method : methods)
    {
        auto& mg = il2cpp::vm::Method::IsGeneric(method) ? genericMethods : methodGroups;

        // Spec: alias (Attribute, else XML) replaces MethodInfo.Name — do not also keep the default name.
        alias.clear();
        if (MetadataUtil::TryReadLuaAlias(method, alias) && !alias.empty())
        {
            mg[CopyRuntimeAliasKey(alias)].push_back(method);
        }
        else if (AliasXmlTable::TryGetAlias(method, alias) && !alias.empty())
        {
            mg[CopyRuntimeAliasKey(alias)].push_back(method);
        }
        else
        {
            mg[method->name].push_back(method);
        }
    }
}

static bool IsOpenGenericMethod(const MethodInfo* method)
{
    if (method->is_generic)
        return true;
    if (il2cpp::metadata::GenericMetadata::ContainsGenericParameters(method))
        return true;
    return false;
}

static void CollectExtensionMethods(Il2CppClass* klass, std::vector<const MethodInfo*>& outMethods)
{
    std::vector<Il2CppClass*> extensionClasses;
    std::unordered_set<Il2CppClass*> seen;

    for (Il2CppClass* walk = klass; walk != nullptr; walk = walk->parent)
    {
        if (walk == il2cpp_defaults.object_class || walk == il2cpp_defaults.value_type_class || walk == il2cpp_defaults.enum_class)
            break;

        std::vector<Il2CppClass*> attrTypes;
        if (MetadataUtil::TryReadLuaExtensionTypes(walk, attrTypes))
        {
            for (size_t i = 0; i < attrTypes.size(); ++i)
            {
                Il2CppClass* ext = attrTypes[i];
                if (ext != nullptr && seen.insert(ext).second)
                    extensionClasses.push_back(ext);
            }
        }

        std::vector<Il2CppClass*> xmlTypes;
        if (ExtensionXmlTable::TryGetExtensionClasses(walk, xmlTypes))
        {
            for (size_t i = 0; i < xmlTypes.size(); ++i)
            {
                Il2CppClass* ext = xmlTypes[i];
                if (ext != nullptr && seen.insert(ext).second)
                    extensionClasses.push_back(ext);
            }
        }
    }

    for (size_t c = 0; c < extensionClasses.size(); ++c)
    {
        Il2CppClass* extKlass = extensionClasses[c];
        MetadataUtil::EnsureMethods(extKlass);
        for (uint16_t i = 0; i < extKlass->method_count; ++i)
        {
            const MethodInfo* method = extKlass->methods[i];
            if (!MetadataUtil::IsPublicMethod(method) || !MetadataUtil::IsStaticMethod(method))
                continue;
            if (MetadataUtil::IsCtorOrCCtor(method))
                continue;
            if (!MetadataUtil::IsExtensionMethod(method))
                continue;
            if (IsOpenGenericMethod(method))
                continue;
            if (method->parameters_count < 1)
                continue;

            const Il2CppType* p0Type = method->parameters[0];
            Il2CppClass* p0Klass = il2cpp::vm::Class::FromIl2CppType(p0Type);
            if (p0Klass == nullptr)
                continue;
            // P0.IsAssignableFrom(T) — Class::IsAssignableFrom(to, from)
            if (!il2cpp::vm::Class::IsAssignableFrom(p0Klass, klass))
                continue;

            outMethods.push_back(method);
        }
    }
}

static void BuildBinding(lua_State* L, TypeBinding* binding)
{
    Il2CppClass* klass = binding->klass;
    il2cpp::vm::Class::Init(klass);

    // if the class is generic or contains generic parameters, we don't need to build the binding
    if (klass->is_generic || il2cpp::metadata::GenericMetadata::ContainsGenericParameters(klass))
    {
        return;
    }

    bool isByVal = MetadataUtil::IsValueTypeClass(klass) && !klass->enumtype && !klass->nullabletype;

    std::vector<const MethodInfo*> staticMethods;
    std::vector<const MethodInfo*> instanceMethods;
    std::vector<const MethodInfo*> ctorMethods;
    std::vector<const MethodInfo*> parentCtorMethods;

    // only collect constructor methods from the current class
    CollectBindableMethods(klass, ctorMethods, staticMethods, instanceMethods);
    for (Il2CppClass* current = klass->parent; current != nullptr; current = current->parent)
    {
        CollectBindableMethods(current, parentCtorMethods, staticMethods, instanceMethods);
    }

    for (Il2CppClass* current = klass; current != nullptr; current = current->parent)
    {
        RegisterProperties(L, current, binding->byobjInstanceMap, binding->staticMap);
        RegisterFields(L, current, binding->byobjInstanceMap, binding->staticMap);
    }
    if (isByVal)
    {
        binding->byvalInstanceMap = binding->byobjInstanceMap;
    }

    SetupCtorMethod(L, binding, ctorMethods, isByVal);

    MethodGroupMap staticGroups;
    MethodGroupMap instanceGroups;
    MethodGroupMap instanceGenericMethods;
    MethodGroupMap staticGenericMethods;

    AddToMethodGroup(staticMethods, staticGroups, staticGenericMethods);
    AddToMethodGroup(instanceMethods, instanceGroups, instanceGenericMethods);

    std::vector<const MethodInfo*> extensionMethods;
    CollectExtensionMethods(klass, extensionMethods);
    AddToMethodGroup(extensionMethods, instanceGroups, instanceGenericMethods);

    for (const auto& kv : staticGroups)
    {
        RegisterMethodGroup(L, binding, kv.first, kv.second, true, false, binding->staticMap);
    }

    for (const auto& kv : instanceGroups)
    {
        RegisterMethodGroup(L, binding, kv.first, kv.second, false, false, binding->byobjInstanceMap);
        if (isByVal)
        {
            RegisterMethodGroup(L, binding, kv.first, kv.second, false, true, binding->byvalInstanceMap);
        }
    }

    for (const auto& kv : staticGenericMethods)
    {
        RegisterGenericMethodGroup(L, binding, kv.first, kv.second, true, false, binding->staticMap);
    }
    for (const auto& kv : instanceGenericMethods)
    {
        RegisterGenericMethodGroup(L, binding, kv.first, kv.second, false, false, binding->byobjInstanceMap);
        if (isByVal)
        {
            RegisterGenericMethodGroup(L, binding, kv.first, kv.second, false, true, binding->byvalInstanceMap);
        }
    }
}

TypeBinding* MetaBinding::EnsureBinding(lua_State* L, Il2CppClass* klass)
{
    auto it = s_bindings.find(klass);
    if (it != s_bindings.end())
        return it->second;

    TypeBinding* binding = new TypeBinding();
    binding->klass = klass;
    BuildBinding(L, binding);
    s_bindings[klass] = binding;
    return binding;
}

} // namespace zlua
