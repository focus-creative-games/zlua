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

#include <cstring>
#include <string>
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
#include "metadata/GenericMetadata.h"
#include "metadata/GenericMethod.h"

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
    int32_t paramsParamIndex; // -1 if none
    bool isExtension;
};

/// Process-lifetime cache (MethodInfo* → shared marshal layout). Not cleared in ShutdownState:
/// entries point into LuaMetadataAlloc and are reused across Reset; see ShutdownState comment.
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
        if (!HasParamsArray(ctx) && minArity == ctx->luaArity)
            LuaException::ThrowFormat("zlua: argument mismatch: expected %d argument(s), got %d", minArity, available);
        if (HasParamsArray(ctx))
            LuaException::ThrowFormat("zlua: argument mismatch: expected at least %d argument(s), got %d", minArity, available);
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
        shared->paramsParamIndex = -1;

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

            // params is always the last CLR parameter (C# rule). Attribute check once at Bind.
            const int lastParam = method->parameters_count - 1;
            if (MetadataUtil::MethodParameterHasParamArrayAttribute(method, lastParam))
                shared->paramsParamIndex = lastParam;
        }
        shared->luaArity = luaArity;

        // params expand cannot use specialized stubs (they assume a single array slot).
        if (shared->paramsParamIndex >= 0)
            shared->lua2CsInvoker = MethodBridge::DefaultInvokeLuaMethod;

        // Trailing C# defaults → dense MethodDefaultArgs (Il2Cpp perf: no blob parse on call).
        // params T[] is always omitable (empty array); continue scanning defaults before it.
        if (method->parameters_count > 0)
        {
            const int paramStart = shared->isExtension ? 1 : 0;
            bool trailingHasDefault = false;
            for (int i = method->parameters_count - 1; i >= paramStart; --i)
            {
                if (MetadataUtil::MethodParameterHasParamArrayAttribute(method, i))
                    continue;
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
                    if (MetadataUtil::MethodParameterHasParamArrayAttribute(method, i))
                    {
                        omitSlots += shared->paramsMeta[i]->stackSlots > 0 ? shared->paramsMeta[i]->stackSlots : 1;
                        continue;
                    }

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
    ctx->paramsParamIndex = shared->paramsParamIndex;
    ctx->byVal = isByVal;
    ctx->sealed = shared->isExtension ? true : MetadataUtil::IsMethodSealed(method, isByVal);
    ctx->isExtension = shared->isExtension;
    return ctx;
}

struct GtapMethodCtx
{
    const TypeBinding* binding;
    bool isStatic;
    bool isByVal;
    int fallbackClosureRef; // LUA_NOREF when open-generic only
    size_t openCount;
    const MethodInfo** openMethods;
};

static bool TryPeekSystemTypeUserData(lua_State* L, int index, Il2CppClass** outKlass)
{
    if (lua_type(L, index) != LUA_TUSERDATA)
        return false;
    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, index);
    if (ud == nullptr || ud->header.kind != UserDataKind::ByObj || ud->obj == nullptr)
        return false;
    if (il2cpp::vm::Object::IsInst(ud->obj, il2cpp_defaults.systemtype_class) == nullptr)
        return false;
    *outKlass = il2cpp::vm::Class::FromSystemType(reinterpret_cast<Il2CppReflectionType*>(ud->obj));
    return *outKlass != nullptr;
}

static bool TryReadSystemTypePrefix(lua_State* L, int argStart, int count, std::vector<const Il2CppType*>& typeArgs)
{
    typeArgs.clear();
    const int top = lua_gettop(L);
    if (argStart + count - 1 > top)
        return false;
    typeArgs.reserve((size_t)count);
    for (int i = 0; i < count; ++i)
    {
        Il2CppClass* klass = nullptr;
        if (!TryPeekSystemTypeUserData(L, argStart + i, &klass))
        {
            typeArgs.clear();
            return false;
        }
        typeArgs.push_back(&klass->byval_arg);
    }
    return true;
}

static int CallClosureOnStack(lua_State* L)
{
    // Stack: [closure][args...]
    const int nargs = lua_gettop(L) - 1;
    lua_call(L, nargs, LUA_MULTRET);
    return lua_gettop(L);
}

static bool TryInvokeGtap(lua_State* L, const MethodInfo* const* openMethods, size_t openCount, const TypeBinding* binding, bool isStatic,
                          bool isByVal, int* outNrets)
{
    *outNrets = 0;
    if (openMethods == nullptr || openCount == 0 || binding == nullptr)
        return false;

    const int argStart = isStatic ? 1 : 2;
    const int top = lua_gettop(L);

    const MethodInfo* matchedOpen = nullptr;
    std::vector<const Il2CppType*> matchedTypeArgs;
    int matchedPrefixCount = 0;
    int matchCount = 0;

    for (size_t i = 0; i < openCount; ++i)
    {
        const MethodInfo* open = openMethods[i];
        if (open == nullptr)
            continue;
        const int n = MetadataUtil::GetMethodGenericParameterCount(open);
        if (n < 1)
            continue;

        std::vector<const Il2CppType*> typeArgs;
        if (!TryReadSystemTypePrefix(L, argStart, n, typeArgs))
            continue;
        if (!MetadataUtil::MethodGenericTypeArgsSatisfyConstraints(open, typeArgs.data(), (uint32_t)typeArgs.size()))
            continue;

        const MethodInfo* closedDef = open;
        const Il2CppGenericInst* methodInst =
            il2cpp::vm::MetadataCache::GetGenericInst(typeArgs.data(), (uint32_t)typeArgs.size());
        const MethodInfo* closed = nullptr;
        if (closedDef->is_inflated)
        {
            closed = il2cpp::metadata::GenericMethod::GetMethod(closedDef->genericMethod->methodDefinition,
                                                               closedDef->genericMethod->context.class_inst, methodInst);
        }
        else
        {
            closed = il2cpp::metadata::GenericMethod::GetMethod(closedDef, nullptr, methodInst);
        }
        if (closed == nullptr)
            continue;

        const int valueArgStart = argStart + n;
        int valueArgCount = top - valueArgStart + 1;
        if (valueArgCount < 0)
            valueArgCount = 0;

        MethodMarshalCtx* closedCtx = MetaBinding::CreateMethodMarshalCtx(L, closed, isByVal);
        const int minArity = GetMinLuaArity(closedCtx);
        const int maxArity = GetMaxLuaArity(closedCtx);
        if (valueArgCount < minArity || valueArgCount > maxArity)
            continue;

        // Zero-arg closed methods (e.g. GetComp<T>()) skip conversion scoring.
        if (closedCtx->luaArity > 0 || valueArgCount > 0 || HasParamsArray(closedCtx))
        {
            if (!MethodOverloadResolver::IsApplicable(L, closedCtx, valueArgStart, valueArgCount))
                continue;
        }

        matchCount++;
        if (matchCount == 1)
        {
            matchedOpen = open;
            matchedTypeArgs = std::move(typeArgs);
            matchedPrefixCount = n;
        }
    }

    if (matchCount != 1 || matchedOpen == nullptr)
        return false;

    for (int i = 0; i < matchedPrefixCount; ++i)
        lua_remove(L, argStart);

    const int closureRef = MetaBinding::InternClosedGenericMethod(L, matchedOpen, binding, isStatic, isByVal, matchedTypeArgs.data(),
                                                                  (uint32_t)matchedTypeArgs.size());
    LuaUtil::PushRef(L, closureRef);
    lua_insert(L, 1);
    *outNrets = CallClosureOnStack(L);
    return true;
}

static int InvokeGtapThenFallback(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const TypeBinding* binding = (const TypeBinding*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexBinding));
    const GtapMethodCtx* gtap = (const GtapMethodCtx*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    IL2CPP_ASSERT(gtap != nullptr);

    int nrets = 0;
    if (TryInvokeGtap(L, gtap->openMethods, gtap->openCount, binding, gtap->isStatic, gtap->isByVal, &nrets))
        return nrets;

    if (gtap->fallbackClosureRef != LUA_NOREF)
    {
        LuaUtil::PushRef(L, gtap->fallbackClosureRef);
        lua_insert(L, 1);
        return CallClosureOnStack(L);
    }

    const char* methodName = (gtap->openCount > 0 && gtap->openMethods[0] != nullptr) ? gtap->openMethods[0]->name : "?";
    LuaException::ThrowFormat(
        "zlua: cannot invoke open generic method %s without System.Type GTAP prefix "
        "(use zlua.inflate, or pass zlua.typeof(...) prefixes)",
        methodName);
    return 0;
    ZLUA_TRY_END()
}

static int InvokeMethodDirectGeneric(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const TypeBinding* binding = (const TypeBinding*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexBinding));
    const MethodMarshalCtx* ctx = (const MethodMarshalCtx*)lua_touserdata(L, lua_upvalueindex(kUpvalueIndexGroupsOrCtx));
    IL2CPP_ASSERT(ctx != nullptr && ctx->method != nullptr);

    const MethodInfo* openMethods[1] = { ctx->method };
    const bool isStatic = MetadataUtil::IsStaticMethod(ctx->method);
    int nrets = 0;
    if (TryInvokeGtap(L, openMethods, 1, binding, isStatic, ctx->byVal, &nrets))
        return nrets;

    LuaException::ThrowFormat(
        "zlua: cannot invoke open generic method %s without System.Type GTAP prefix "
        "(use zlua.inflate, or pass zlua.typeof(...) prefixes)",
        ctx->method->name);
    return 0;
    ZLUA_TRY_END()
}

static int CreateGtapWrapperClosureRef(lua_State* L, const TypeBinding* binding, const std::vector<const MethodInfo*>& openMethods,
                                       int fallbackClosureRef, bool isStatic, bool isByVal)
{
    IL2CPP_ASSERT(!openMethods.empty());
    GtapMethodCtx* gtap = LuaMetadataAlloc::MallocAnyZeroed<GtapMethodCtx>();
    gtap->binding = binding;
    gtap->isStatic = isStatic;
    gtap->isByVal = isByVal;
    gtap->fallbackClosureRef = fallbackClosureRef;
    gtap->openCount = openMethods.size();
    gtap->openMethods = LuaMetadataAlloc::CallocArray<const MethodInfo*>(openMethods.size());
    for (size_t i = 0; i < openMethods.size(); ++i)
        gtap->openMethods[i] = openMethods[i];

    lua_pushnumber(L, (lua_Number)(int)ClosureKind::Gtap);
    lua_pushlightuserdata(L, (void*)binding);
    lua_pushlightuserdata(L, gtap);
    lua_pushcclosure(L, InvokeGtapThenFallback, 3);
    return LuaUtil::ToLuaRef(L);
}

int MetaBinding::InternClosedGenericMethod(lua_State* L, const MethodInfo* openMethod, const TypeBinding* binding, bool isStatic, bool isByVal,
                                           const Il2CppType* const* typeArgs, uint32_t typeArgCount)
{
    IL2CPP_ASSERT(openMethod != nullptr && binding != nullptr && typeArgs != nullptr && typeArgCount > 0);

    const MethodInfo* method = openMethod;
    const Il2CppGenericInst* methodInst = il2cpp::vm::MetadataCache::GetGenericInst(typeArgs, typeArgCount);
    if (method->is_inflated)
    {
        method = il2cpp::metadata::GenericMethod::GetMethod(method->genericMethod->methodDefinition, method->genericMethod->context.class_inst,
                                                           methodInst);
    }
    else
    {
        method = il2cpp::metadata::GenericMethod::GetMethod(method, nullptr, methodInst);
    }
    if (method == nullptr)
        LuaException::Throw("zlua.inflate MakeGenericMethod failed: could not construct closed method");

    // Intern key includes isStatic/isByVal via separate NameMetaMaps (static / byobj / byval).
    std::string methodSignature = MetadataUtil::CreateMethodNameWithParametersAndGenericArguments(method);
    NameMetaMap& nmm = isStatic ? binding->staticMap : (isByVal ? binding->byvalInstanceMap : binding->byobjInstanceMap);
    auto it = nmm.find(methodSignature.c_str());
    if (it != nmm.end())
        return it->second.closureRef;

    int closureRef = CreateDirectMethodClosureRef(L, method, binding, MetadataUtil::IsStaticMethod(method), isByVal);
    MetaInfo newMeta = {};
    newMeta.kind = MetaKind::Method;
    newMeta.closureRef = closureRef;
    const char* key = zlua_strdup(methodSignature.c_str());
    nmm.insert({key, newMeta});
    return closureRef;
}

/// §3.7 key without method-generic angle brackets (align with Mono / inflate open keys).
static std::string BuildMethodSignatureKey(const MethodInfo* method)
{
    std::string name = method->name;
    name += '(';
    const int paramStart = MetadataUtil::IsExtensionMethod(method) ? 1 : 0;
    bool first = true;
    for (int i = paramStart; i < method->parameters_count; i++)
    {
        if (!first)
            name += ',';
        first = false;
        name += MetadataUtil::GetLuaFullName(method->parameters[i]);
    }
    name += ')';
    return name;
}

static bool ShouldSkipSignatureResolve(const MethodInfo* method)
{
    if (method == nullptr)
        return true;
    if (MetadataUtil::IsCtorOrCCtor(method))
        return true;
    const char* n = method->name;
    if (std::strncmp(n, "get_", 4) == 0 || std::strncmp(n, "set_", 4) == 0)
        return true;
    return false;
}

static const MethodInfo* FindMethodBySignatureKey(Il2CppClass* ownerKlass, const char* signatureKey, bool isStatic)
{
    IL2CPP_ASSERT(ownerKlass != nullptr && signatureKey != nullptr);
    for (Il2CppClass* walk = ownerKlass; walk != nullptr; walk = walk->parent)
    {
        if (walk == il2cpp_defaults.object_class || walk == il2cpp_defaults.value_type_class || walk == il2cpp_defaults.enum_class)
            break;

        MetadataUtil::EnsureMethods(walk);
        for (uint16_t i = 0; i < walk->method_count; ++i)
        {
            const MethodInfo* method = walk->methods[i];
            if (!MetadataUtil::IsPublicMethod(method))
                continue;
            if (ShouldSkipSignatureResolve(method))
                continue;
            if (MetadataUtil::IsStaticMethod(method) != isStatic)
                continue;
            if (BuildMethodSignatureKey(method) == signatureKey)
                return method;
        }
    }
    return nullptr;
}

int MetaBinding::TryResolveSignatureMethodAndPush(lua_State* L, TypeBinding* binding, const char* signatureKey, bool isStatic, bool isByVal)
{
    IL2CPP_ASSERT(binding != nullptr);
    if (signatureKey == nullptr || signatureKey[0] == '\0' || std::strchr(signatureKey, '(') == nullptr)
        return 0;

    NameMetaMap& map = isStatic ? binding->staticMap : (isByVal ? binding->byvalInstanceMap : binding->byobjInstanceMap);
    auto it = map.find(signatureKey);
    if (it != map.end() && it->second.kind == MetaKind::Method)
    {
        return LuaUtil::PushRef(L, it->second.closureRef);
    }

    const MethodInfo* method = FindMethodBySignatureKey(binding->klass, signatureKey, isStatic);
    if (method == nullptr)
        return 0;

    const int closureRef = CreateDirectMethodClosureRef(L, method, binding, isStatic, isByVal);
    MetaInfo newMeta = {};
    newMeta.kind = MetaKind::Method;
    newMeta.closureRef = closureRef;
    map[CopyRuntimeAliasKey(std::string(signatureKey))] = newMeta;

#if ZLUA_FAST_METATABLE
    LuaUtil::PushRef(L, closureRef);
    const int closureStackIndex = lua_gettop(L);
    FastMetatable::RawSetMethodOnIndexTable(L, binding->klass, isStatic, isByVal, signatureKey, closureStackIndex);
    // leave closure on stack for caller
#else
    return LuaUtil::PushRef(L, closureRef);
#endif
    return 1;
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
                                   std::vector<const MethodInfo*>& instanceMethods, std::unordered_set<uint32_t>& virtualSlotsSeen)
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
            if (method->slot != kInvalidIl2CppMethodSlot)
            {
                if (virtualSlotsSeen.find(method->slot) != virtualSlotsSeen.end())
                    continue;
                instanceMethods.push_back(method);
                virtualSlotsSeen.insert(method->slot);
            }
            else
            {
                instanceMethods.push_back(method);
            }
        }
    }
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
        const bool hasParams = HasParamsArray(ctx);
        // params: register into all small buckets + large (unbounded expand).
        const int32_t maxArity = hasParams ? static_cast<int32_t>(kMaxSmallArgCount) : ctx->luaArity;
        for (int32_t arity = GetMinLuaArity(ctx); arity <= maxArity; ++arity)
        {
            if (arity <= static_cast<int32_t>(kMaxSmallArgCount))
                methodGroups[arity].push_back(ctx);
        }
        if (hasParams || ctx->luaArity > static_cast<int32_t>(kMaxSmallArgCount))
            largeArgCountMethods.push_back(ctx);
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
        // Full-signature keys are lazy (§3.7); do not Bind-pre-register.
    }
}

static void RegisterGenericMethodGroup(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic, bool isByVal,
                                       NameMetaMap& map)
{
    IL2CPP_ASSERT(overloads.size() > 0 && name != nullptr);
    NameMetaMap::iterator existing = map.find(name);
    if (existing != map.end())
    {
        // Non-generic already owns short name (e.g. GetComp(Type)): GTAP first, then fallback (§3.8.2).
        existing->second.closureRef =
            CreateGtapWrapperClosureRef(L, binding, overloads, existing->second.closureRef, isStatic, isByVal);
    }
    else if (overloads.size() == 1)
    {
        // Single open generic: DirectGeneric closure (GTAP in InvokeMethodDirectGeneric).
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        info.closureRef = MetaBinding::CreateDirectMethodClosureRef(L, overloads[0], binding, isStatic, isByVal);
        map[name] = info;
    }
    else
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        info.closureRef = CreateGtapWrapperClosureRef(L, binding, overloads, LUA_NOREF, isStatic, isByVal);
        map[name] = info;
    }
    // Full-signature keys are lazy (§3.7); do not Bind-pre-register.
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
    std::unordered_set<uint32_t> virtualSlotsSeen;

    // only collect constructor methods from the current class
    CollectBindableMethods(klass, ctorMethods, staticMethods, instanceMethods, virtualSlotsSeen);
    for (Il2CppClass* current = klass->parent; current != nullptr; current = current->parent)
    {
        CollectBindableMethods(current, parentCtorMethods, staticMethods, instanceMethods, virtualSlotsSeen);
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

static std::string BuildOpenMethodSignatureKey(const MethodInfo* method)
{
    return BuildMethodSignatureKey(method);
}

struct OpenGenericMatchState
{
    const char* nameOrSignature;
    bool isSignature;
    int genericArity;
    const MethodInfo* match;
    bool matchStatic;
    bool matchByVal;
    int matchCount;
};

static void ConsiderOpenGenericCandidate(const MethodInfo* method, bool isStatic, bool isByVal, OpenGenericMatchState* st)
{
    if (method == nullptr || !IsOpenGenericMethod(method))
        return;
    if (MetadataUtil::GetMethodGenericParameterCount(method) != st->genericArity)
        return;
    if (st->isSignature)
    {
        if (BuildOpenMethodSignatureKey(method) != st->nameOrSignature)
            return;
    }
    if (st->match == method)
        return;
    st->matchCount++;
    if (st->matchCount == 1)
    {
        st->match = method;
        st->matchStatic = isStatic;
        st->matchByVal = isByVal;
    }
}

static void CollectOpenGenericFromMap(lua_State* L, NameMetaMap& map, bool isStatic, bool isByVal, OpenGenericMatchState* st)
{
    for (NameMetaMap::iterator it = map.begin(); it != map.end(); ++it)
    {
        MetaInfo& info = it->second;
        if (info.kind != MetaKind::Method)
            continue;

        if (!st->isSignature && std::strcmp(it->first, st->nameOrSignature) != 0)
            continue;

        LuaUtil::PushRef(L, info.closureRef);
        lua_getupvalue(L, -1, kUpvalueIndexClosureKind);
        ClosureKind kind = (ClosureKind)(int)lua_tonumber(L, -1);
        lua_pop(L, 1);

        if (kind == ClosureKind::Direct || kind == ClosureKind::DirectGeneric)
        {
            const MethodMarshalCtx* ctx = MetaBinding::GetMethodMarshalCtxFromClosure(L, -1);
            if (ctx != nullptr)
                ConsiderOpenGenericCandidate(ctx->method, isStatic, isByVal, st);
        }
        else if (kind == ClosureKind::Gtap)
        {
            lua_getupvalue(L, -1, kUpvalueIndexGroupsOrCtx);
            const GtapMethodCtx* gtap = (const GtapMethodCtx*)lua_touserdata(L, -1);
            lua_pop(L, 1);
            if (gtap != nullptr)
            {
                for (size_t i = 0; i < gtap->openCount; ++i)
                    ConsiderOpenGenericCandidate(gtap->openMethods[i], isStatic, isByVal, st);
            }
        }
        else if (kind == ClosureKind::Indirect)
        {
            lua_getupvalue(L, -1, kUpvalueIndexGroupsOrCtx);
            const MethodGroups* groups = (const MethodGroups*)lua_touserdata(L, -1);
            lua_pop(L, 1);
            if (groups != nullptr)
            {
                for (size_t i = 0; i <= kMaxSmallArgCount; ++i)
                {
                    const MethodGroup* group = groups->smallArgCountMethodGroups[i];
                    if (group == nullptr)
                        continue;
                    for (size_t j = 0; j < group->methodCount; ++j)
                    {
                        const MethodMarshalCtx* ctx = group->methods[j];
                        if (ctx != nullptr)
                            ConsiderOpenGenericCandidate(ctx->method, isStatic, isByVal, st);
                    }
                }
                for (size_t j = 0; j < groups->largeArgCountMethodCount; ++j)
                {
                    const MethodMarshalCtx* ctx = groups->largeArgCountMethods[j];
                    if (ctx != nullptr)
                        ConsiderOpenGenericCandidate(ctx->method, isStatic, isByVal, st);
                }
            }
        }
        lua_pop(L, 1);
    }
}

bool MetaBinding::TryFindOpenGenericMethod(lua_State* L, Il2CppClass* ownerKlass, const char* nameOrSignature, int genericArity,
                                           const MethodInfo** outMethod, bool* outIsStatic, bool* outIsByVal)
{
    *outMethod = nullptr;
    *outIsStatic = false;
    *outIsByVal = false;
    if (ownerKlass == nullptr || nameOrSignature == nullptr || nameOrSignature[0] == '\0' || genericArity < 1)
        return false;

    TypeBinding* binding = EnsureBinding(L, ownerKlass);
    OpenGenericMatchState st = {};
    st.nameOrSignature = nameOrSignature;
    st.isSignature = std::strchr(nameOrSignature, '(') != nullptr;
    st.genericArity = genericArity;

    CollectOpenGenericFromMap(L, binding->staticMap, true, false, &st);
    CollectOpenGenericFromMap(L, binding->byobjInstanceMap, false, false, &st);
    // Prefer byobj; byval duplicates the same MethodInfo and is deduped.

    if (st.matchCount != 1)
        return false;

    *outMethod = st.match;
    *outIsStatic = st.matchStatic;
    *outIsByVal = st.matchByVal;
    return true;
}

void MetaBinding::ShutdownState()
{
    // Method MetaInfo.closureRef values are luaL_ref numbers for the closing lua_State.
    // Drop TypeBindings only so EnsureBinding rebuilds closures against the next state.
    //
    // Do not clear s_methodMarshalShared or free *MarshalCtx: they are process-level,
    // keyed by stable MethodInfo*/metadata, allocated from LuaMetadataAlloc (bump pool with
    // no per-object Free). Reset must keep them (01-HOST-API / 10-LIFETIME: Il2Cpp process
    // Bridge/binding tables survive Reset). Pool chunks are released only when
    // LuaMetadataAlloc::Shutdown runs at process teardown.
    for (auto& kv : s_bindings)
        delete kv.second;
    s_bindings.clear();
}

} // namespace zlua
