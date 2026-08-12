#include <unordered_map>
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
#include "../utils/Collection.h"
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
#include "metadata/GenericMetadata.h"

#include <unordered_set>

namespace zlua
{
using NameMetaMap = AppendOnlyStringHashMap<MetaInfo>;

static HashMap<Il2CppClass*, TypeBinding*> s_bindings;

static AppendOnlyStringHashSet<> s_methodAliasKeys;

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
    if (available < ctx->luaArity)
    {
        LuaException::ThrowFormat("zlua: argument mismatch: expected %d argument(s), got %d", ctx->luaArity, available);
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

MethodMarshalCtx* MetaBinding::CreateMethodMarshalCtx(lua_State* L, const MethodInfo* method, bool isByVal)
{
    const bool isExtension = MetadataUtil::IsExtensionMethod(method);
    bool isStatic = MetadataUtil::IsStaticMethod(method);
    FnResolveMethodThis resolveThis =
        isExtension ? nullptr : InstanceTarget::GetResolveMethodThisFunc(method->klass, isStatic, isByVal);
    MethodMarshalCtx* ctx = LuaMetadataAlloc::MallocAnyZeroed<MethodMarshalCtx>();
    ctx->method = method;
    ctx->resolveThis = resolveThis;
    ctx->lua2CsInvoker = MethodBridge::ResolveMethodInvoker(method);
    ctx->byVal = isByVal;
    ctx->isExtension = isExtension;
    ctx->sealed = isExtension ? true : MetadataUtil::IsMethodSealed(method, isByVal);
    ctx->valueSize = MetadataUtil::GetValueSize(&method->klass->byval_arg);
    int32_t luaArity = 0;
    if (method->parameters_count > 0)
    {
        ctx->paramsMeta = LuaMetadataAlloc::CallocArray<const MarshalMetaInfo*>(method->parameters_count);
        int32_t totalParamsSize = 0;
        for (int i = 0; i < method->parameters_count; i++)
        {
            const MarshalMetaInfo* meta = MarshalMeta::Create(L, method, i);
            ctx->paramsMeta[i] = meta;
            totalParamsSize += meta->size;
            // Extension: luaArity excludes CLR param 0 (this) — matches IMT argStart=2.
            if (!isExtension || i >= 1)
                luaArity += meta->stackSlots > 0 ? meta->stackSlots : 1;
        }
        ctx->totalParamsSize = totalParamsSize;
    }
    else
    {
        ctx->paramsMeta = nullptr;
        ctx->totalParamsSize = 0;
    }
    ctx->luaArity = luaArity;
    if (MetadataUtil::IsVoidType(method->return_type))
    {
        ctx->retMeta = nullptr;
    }
    else
    {
        ctx->retMeta = MarshalMeta::Create(L, method, -1);
    }

    bool needsStackCursor = false;
    for (int i = 0; i < method->parameters_count; i++)
    {
        const MarshalMetaInfo* meta = ctx->paramsMeta[i];
        if (meta->marshalType == LuaMarshalType::Table || meta->marshalType == LuaMarshalType::UnpackedValues || meta->stackSlots != 1)
        {
            needsStackCursor = true;
            break;
        }
    }
    if (!needsStackCursor && ctx->retMeta != nullptr &&
        (ctx->retMeta->marshalType == LuaMarshalType::Table || ctx->retMeta->marshalType == LuaMarshalType::UnpackedValues ||
         ctx->retMeta->stackSlots != 1))
    {
        needsStackCursor = true;
    }
    if (needsStackCursor)
        ctx->lua2CsInvoker = MethodBridge::DefaultInvokeLuaMethod;

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

static const MethodMarshalCtx* FindBestMethod(lua_State* L, const TypeBinding* binding, const MethodGroups* groups, int argStart)
{
    MethodOverloadResolutionResult result = MethodOverloadResolver::Resolve(L, groups, argStart, lua_gettop(L) - argStart + 1);
    if (result.kind == MethodOverloadResolutionKind::None)
    {
        LuaException::ThrowFormat("zlua: no matching overload for type: %s", MetadataUtil::GetSignatureTypeName(binding->klass).c_str());
    }
    if (result.kind == MethodOverloadResolutionKind::Ambiguous)
    {
        LuaException::ThrowFormat("zlua: ambiguous method found for type: %s", MetadataUtil::GetSignatureTypeName(binding->klass).c_str());
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
    info.method.closureRef = closureRef;
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
    void* fieldIter = NULL;
    FieldInfo* field;
    while ((field = il2cpp::vm::Class::GetFields(klass, &fieldIter)) != NULL)
    {
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
        FieldMarshalCtx& ctx = info.field;
        ctx.field = field;
        ctx.meta = MarshalMeta::Create(L, field);
        SetupFieldAddressOrInstanceOffset(ctx, field, isStatic);
        map[field->name] = info;
    }
}

static void RegisterProperties(lua_State* L, Il2CppClass* klass, NameMetaMap& instanceMap, NameMetaMap& staticMap)
{
    void* propertyIter = NULL;
    const PropertyInfo* property;
    while ((property = il2cpp::vm::Class::GetProperties(klass, &propertyIter)) != NULL)
    {
        if (!MetadataUtil::IsPublicProperty(property) || !MetadataUtil::IsZeroParameterProperty(property))
            continue;

        const bool isStatic = MetadataUtil::IsStaticProperty(property);
        NameMetaMap& map = isStatic ? staticMap : instanceMap;
        if (map.find(property->name) != map.end())
            continue;

        MetaInfo info = {};
        info.kind = MetaKind::Property;

        PropertyMarshalCtx& pmCtx = info.property;
        pmCtx.property = property;
        const Il2CppType* type = MetadataUtil::GetPropertyReturnType(property);
        pmCtx.valueTypeKlass = il2cpp::vm::Class::FromIl2CppType(type, true);
        pmCtx.meta = MarshalMeta::Create(L, property);

        PropertyAccessor accessor = PropertyBridge::ResolvePropertyAccessor(property, isStatic);

        if (property->get != nullptr)
        {
            pmCtx.getter = accessor.getter;
            pmCtx.getterSealed = MetadataUtil::IsMethodSealed(property->get, /*byVal*/ false);
        }
        else
        {
            pmCtx.getter = nullptr;
            pmCtx.getterSealed = true;
        }
        if (property->set != nullptr)
        {
            pmCtx.setter = accessor.setter;
            pmCtx.setterSealed = MetadataUtil::IsMethodSealed(property->set, /*byVal*/ false);
        }
        else
        {
            pmCtx.setter = nullptr;
            pmCtx.setterSealed = true;
        }
        map[property->name] = info;
    }
}

static MethodGroups* CreateMethodGroups(lua_State* L, const std::vector<const MethodInfo*>& methods, bool isByVal)
{
    IL2CPP_ASSERT(methods.size() > 1);
    MethodGroups* groups = LuaMetadataAlloc::NewAny<MethodGroups>();

    std::vector<const MethodMarshalCtx*> methodGroups[kMaxSmallArgCount + 1];
    std::vector<const MethodMarshalCtx*> largeArgCountMethods;

    for (const MethodInfo* method : methods)
    {
        const MethodMarshalCtx* ctx = MetaBinding::CreateMethodMarshalCtx(L, method, isByVal);
        if (ctx->luaArity > static_cast<int32_t>(kMaxSmallArgCount))
        {
            largeArgCountMethods.push_back(ctx);
            continue;
        }
        methodGroups[ctx->luaArity].push_back(ctx);
    }

    for (size_t i = 0; i <= kMaxSmallArgCount; i++)
    {
        std::vector<const MethodMarshalCtx*>& group = methodGroups[i];
        if (group.empty())
        {
            groups->smallArgCountMethodGroups[i] = nullptr;
            continue;
        }
        MethodGroup* mg = LuaMetadataAlloc::NewAny<MethodGroup>();
        const MethodMarshalCtx** methodsArr = LuaMetadataAlloc::CallocArray<const MethodMarshalCtx*>(group.size());
        for (size_t j = 0; j < group.size(); j++)
        {
            methodsArr[j] = group[j];
        }
        mg->methods = methodsArr;
        mg->methodCount = group.size();
        groups->smallArgCountMethodGroups[i] = mg;
    }
    if (!largeArgCountMethods.empty())
    {
        const MethodMarshalCtx** methodsArr = LuaMetadataAlloc::CallocArray<const MethodMarshalCtx*>(largeArgCountMethods.size());
        for (size_t j = 0; j < largeArgCountMethods.size(); j++)
        {
            methodsArr[j] = largeArgCountMethods[j];
        }
        groups->largeArgCountMethods = methodsArr;
        groups->largeArgCountMethodCount = largeArgCountMethods.size();
    }

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

static void RegisterMethodsdWithSignature(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic,
                                          bool isByVal, NameMetaMap& map)
{
    for (const MethodInfo* method : overloads)
    {
        std::string methodSignature = MetadataUtil::CreateMethodNameWithParametersAndGenericArguments(method);
        if (map.find(methodSignature.c_str()) != map.end())
            continue;
        MetaInfo fullNameInfo = {};
        fullNameInfo.kind = MetaKind::Method;
        fullNameInfo.method.closureRef = MetaBinding::CreateDirectMethodClosureRef(L, method, binding, isStatic, isByVal);
        map[CopyRuntimeAliasKey(methodSignature)] = fullNameInfo;
    }
}

static void RegisterMethodGroup(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic,
                                bool isByVal, NameMetaMap& map)
{
    IL2CPP_ASSERT(overloads.size() > 0 && name != nullptr);
    if (overloads.size() == 1)
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        int closureRef = MetaBinding::CreateDirectMethodClosureRef(L, overloads[0], binding, isStatic, isByVal);
        info.method.closureRef = closureRef;
        map[name] = info;
    }
    else
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        MethodGroups* groups = CreateMethodGroups(L, overloads, isByVal);
        info.method.closureRef = CreateMethodDispatchClosureRef(L, binding, groups, isStatic ? InvokeStaticMethodDispatch : InvokeInstanceMethodDispatch);
        map[name] = info;

        RegisterMethodsdWithSignature(L, binding, name, overloads, isStatic, isByVal, map);
    }
}

static void RegisterGenericMethodGroup(lua_State* L, TypeBinding* binding, const char* name, const std::vector<const MethodInfo*>& overloads, bool isStatic,
                                       bool isByVal, NameMetaMap& map)
{
    IL2CPP_ASSERT(overloads.size() > 0 && name != nullptr);
    if (overloads.size() == 1 && map.find(name) == map.end())
    {
        MetaInfo info = {};
        info.kind = MetaKind::Method;
        int closureRef = MetaBinding::CreateDirectMethodClosureRef(L, overloads[0], binding, isStatic, isByVal);
        info.method.closureRef = closureRef;
        map[name] = info;
    }
    RegisterMethodsdWithSignature(L, binding, name, overloads, isStatic, isByVal, map);
}

static void AddToMethodGroup(const std::vector<const MethodInfo*>& methods, AppendOnlyStringHashMap<std::vector<const MethodInfo*>>& methodGroups,
                             AppendOnlyStringHashMap<std::vector<const MethodInfo*>>& genericMethods)
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
        il2cpp::vm::Class::Init(extKlass);
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

    AppendOnlyStringHashMap<std::vector<const MethodInfo*>> staticGroups;
    AppendOnlyStringHashMap<std::vector<const MethodInfo*>> instanceGroups;
    AppendOnlyStringHashMap<std::vector<const MethodInfo*>> instanceGenericMethods;
    AppendOnlyStringHashMap<std::vector<const MethodInfo*>> staticGenericMethods;

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
    HashMap<Il2CppClass*, TypeBinding*>::iterator it = s_bindings.find(klass);
    if (it != s_bindings.end())
        return it->second;

    TypeBinding* binding = new TypeBinding();
    binding->klass = klass;
    BuildBinding(L, binding);
    s_bindings[klass] = binding;
    return binding;
}

} // namespace zlua
