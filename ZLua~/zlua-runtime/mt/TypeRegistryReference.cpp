#include "TypeRegistryReference.h"
#include "MetaBinding.h"
#include "MetaTableCache.h"
#include "TypeRegistryCommon.h"

#include "../LuaConsts.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/MethodOverloadResolver.h"  
#include "../bridge/MethodBridge.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaException.h"

#include "vm/Type.h"
#include "vm/Object.h"

namespace zlua
{

// TODO: optimize this
int CreateReferenceTypeInstance(lua_State* L)
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
        if (argCount != ctx->luaArity)
        {
            LuaException::ThrowFormat("zlua: argument mismatch: constructor expects %d argument(s), but %d were given", ctx->luaArity, argCount);
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
    Il2CppObject* obj = il2cpp::vm::Object::New(klass);
    ObjectMarshal::Push(L, obj, klass);
    int ret = MethodBridge::InvokeLua2Cs(L, obj, argStartIdx, targetMethodCtx);
    IL2CPP_ASSERT(ret == 0);
    return 1;
    ZLUA_TRY_END();
}

void TypeRegistryReference::CreateTypeTable(lua_State* L, Il2CppClass* klass)
{
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    IL2CPP_ASSERT(il2cpp::vm::Type::IsReference(&klass->byval_arg));

    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int typeTableIndex = stackGuard.GetTop();

    TypeRegistryCommon::WriteCommonTypeFields(L, klass, typeTableIndex, binding);
    TypeRegistryCommon::RegisterStaticLiteralFields(L, klass, typeTableIndex);
    TypeRegistryCommon::AttachReferenceInstanceMetatable(L, klass, typeTableIndex, binding);
    TypeRegistryCommon::AttachStaticTypeMetatable(L, klass, typeTableIndex, CreateReferenceTypeInstance, nullptr, nullptr, binding);
}
} // namespace zlua
