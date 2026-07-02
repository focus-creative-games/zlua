#include "TypeRegistryValueType.h"
#include "TypeRegistryCommon.h"
#include "MetaBinding.h"
#include "MetaTableCache.h"

#include "../LuaConsts.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaException.h"
#include "../marshal/MethodOverloadResolver.h"
#include "../marshal/StructMarshal.h"
#include "il2cpp-config.h"

namespace zlua
{

// TODO: optimize this
static int CreateValueTypeInstance(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
    Il2CppClass* klass = binding->klass;

    int argStartIdx = 2;
    int argCount = lua_gettop(L) - argStartIdx + 1;

    const MethodMarshalCtx* targetMethodCtx = nullptr;
    if (binding->uniqueCtorMethod != nullptr)
    {
        const MethodMarshalCtx* ctx = binding->uniqueCtorMethod;
        if (argCount != ctx->method->parameters_count)
        {
            LuaException::ThrowFormat("zlua: constructor has %d parameters, but %d were given", ctx->method->parameters_count, argCount);
        }
        targetMethodCtx = ctx;
    }
    else if (binding->ctorGroups != nullptr)
    {
        const MethodGroups* groups = binding->ctorGroups;
        MethodOverloadResolutionResult result = MethodOverloadResolver::Resolve(L, groups, argStartIdx, argCount);
        if (result.kind == MethodOverloadResolutionKind::Ambiguous)
        {
            LuaException::ThrowFormat("zlua: ambiguous constructor found for type: %s.%s", klass->namespaze, klass->name);
        }
        targetMethodCtx = result.method;
    }

    if (targetMethodCtx == nullptr)
    {
        LuaException::ThrowFormat("zlua: no constructor found for type: %s", klass->name);
    }
    void* payload = StructMarshal::PushZeroedValue(L, klass);
    int ret = targetMethodCtx->lua2CsInvoker(L, payload, argStartIdx, targetMethodCtx);
    // constructor should not return a value
    IL2CPP_ASSERT(ret == 0);
    return 1;
    ZLUA_TRY_END();
}

static int InvokeStructDefault(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));

    if (lua_gettop(L) != 0)
        LuaException::ThrowFormat("zlua: %s.%s expects no arguments", MetadataUtil::GetTypeFullName(klass), LuaConsts::Default);

    StructMarshal::PushZeroedValue(L, klass);
    return 1;
    ZLUA_TRY_END();
}

void AttachNullableStaticTypeMetatable(lua_State* L, int typeTableIndex, lua_CFunction callFn, TypeBinding* elementBinding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int smtIndex = stackGuard.GetTop();

    if (callFn != nullptr)
    {
        lua_pushlightuserdata(L, elementBinding);
        lua_pushcclosure(L, callFn, 1);
        lua_setfield(L, smtIndex, LuaConsts::MetaCall);
    }

    lua_pushcfunction(L, TypeRegistryCommon::TypeTableToString);
    lua_setfield(L, smtIndex, LuaConsts::MetaToString);

    lua_setmetatable(L, typeTableIndex);
}

void TypeRegistryValueType::CreateTypeTable(lua_State* L, Il2CppClass* klass)
{
    IL2CPP_ASSERT(!klass->nullabletype);
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int typeTableIndex = stackGuard.GetTop();
    TypeRegistryCommon::WriteCommonTypeFields(L, klass, typeTableIndex, binding);
    TypeRegistryCommon::RegisterStaticLiteralFields(L, klass, typeTableIndex);

    lua_pushboolean(L, 1);
    lua_setfield(L, typeTableIndex, LuaConsts::Struct);

    TypeRegistryCommon::AttachByValInstanceMetatable(L, klass, typeTableIndex, nullptr, binding);
    TypeRegistryCommon::AttachByObjInstanceMetatable(L, klass, typeTableIndex, TypeRegistryCommon::ObjectInstanceToString, binding);
    TypeRegistryCommon::AttachStaticTypeMetatable(L, klass, typeTableIndex, CreateValueTypeInstance, InvokeStructDefault, LuaConsts::Default, binding);
}

void TypeRegistryValueType::CreateNullableTypeTable(lua_State* L, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass->nullabletype);
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int typeTableIndex = stackGuard.GetTop();
    TypeRegistryCommon::WriteCommonTypeFields(L, klass, typeTableIndex, binding);

    lua_pushboolean(L, 1);
    lua_setfield(L, typeTableIndex, LuaConsts::Nullable);
    TypeBinding* elementBinding = MetaBinding::EnsureBinding(L, klass->element_class);
    AttachNullableStaticTypeMetatable(L, typeTableIndex, CreateValueTypeInstance, elementBinding);
}

void TypeRegistryValueType::CreateEnumTypeTable(lua_State* L, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass->enumtype);
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);

    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int typeTableIndex = stackGuard.GetTop();
    TypeRegistryCommon::WriteCommonTypeFields(L, klass, typeTableIndex, binding);

    lua_pushboolean(L, 1);
    lua_setfield(L, typeTableIndex, LuaConsts::Enum);

    // RegisterEnumConstants(L, klass, typeTableIndex);
    TypeRegistryCommon::RegisterStaticLiteralFields(L, klass, typeTableIndex);

    // TypeRegistryCommon::AttachByValInstanceMetatable(L, klass, typeTableIndex, EnumInstanceToString, binding);
    TypeRegistryCommon::AttachByObjInstanceMetatable(L, klass, typeTableIndex, TypeRegistryCommon::ObjectInstanceToString, binding);
    TypeRegistryCommon::AttachStaticTypeMetatable(L, klass, typeTableIndex, nullptr, nullptr, nullptr, binding);
}
} // namespace zlua
