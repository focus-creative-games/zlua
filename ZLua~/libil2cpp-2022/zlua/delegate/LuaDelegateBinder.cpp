#include "LuaDelegateBinder.h"

#include "DelegateBridges.h"
#include "LuaMethodTarget.h"

#include "../MetadataUtil.h"

#include "il2cpp-object-internals.h"
#include "lua/lauxlib.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Type.h"

namespace zlua
{
static Il2CppClass* ResolveLuaMethodClass()
{
    static Il2CppClass* s_luaMethodClass = nullptr;
    if (s_luaMethodClass != nullptr)
        return s_luaMethodClass;

    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("ZLua.Il2Cpp");
    s_luaMethodClass = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaMethod");
    il2cpp::vm::Class::Init(s_luaMethodClass);
    IL2CPP_ASSERT(s_luaMethodClass->instance_size == sizeof(LuaMethodTarget));
    return s_luaMethodClass;
}

int LuaDelegateBinder::CreateFunctionRef(lua_State* L, int index)
{
    lua_pushvalue(L, index);
    const int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return funcRef == LUA_REFNIL ? LUA_NOREF : funcRef;
}

Il2CppDelegate* LuaDelegateBinder::Create(lua_State* L, Il2CppClass* delegateClass, int funcRef)
{
    if (funcRef == LUA_NOREF)
        return nullptr;

    il2cpp::vm::Class::Init(delegateClass);
    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
    void* bridge = DelegateBridges::Resolve(delegateClass);
    if (invokeMethod == nullptr || bridge == nullptr)
        return nullptr;

    Il2CppClass* luaMethodClass = ResolveLuaMethodClass();
    LuaMethodTarget* target = reinterpret_cast<LuaMethodTarget*>(il2cpp::vm::Object::New(luaMethodClass));
    target->disposed = false;
    target->L = L;
    target->funcRef = funcRef;

    Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(il2cpp::vm::Object::New(delegateClass));
    if (delegate == nullptr)
        return nullptr;

    il2cpp::vm::Type::ConstructClosedDelegate(delegate, reinterpret_cast<Il2CppObject*>(target), reinterpret_cast<Il2CppMethodPointer>(bridge), invokeMethod);
    return delegate;
}

Il2CppDelegate* LuaDelegateBinder::CreateFromStack(lua_State* L, int index, Il2CppClass* delegateClass)
{
    const int funcRef = CreateFunctionRef(L, index);
    if (funcRef == LUA_NOREF)
        return nullptr;

    Il2CppDelegate* delegate = Create(L, delegateClass, funcRef);
    if (delegate == nullptr)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
        return nullptr;
    }
    return delegate;
}
} // namespace zlua
