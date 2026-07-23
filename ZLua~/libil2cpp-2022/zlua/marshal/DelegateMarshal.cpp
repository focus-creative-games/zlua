#include "DelegateMarshal.h"

#include "ObjectRegistry.h"
#include "MarshalDefs.h"
#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaUtil.h"
#include "../bridge/DelegateBridge.h"

#include "vm/Runtime.h"
#include "vm/Object.h"

namespace zlua
{
void DelegateMarshal::Push(lua_State* L, Il2CppDelegate* delegate, int metatableRefIndex)
{
    Il2CppObject* target = delegate->target;
    if (target != nullptr && target->klass == MetadataUtil::GetLuaMethodClass())
    {
        LuaMethod* luaMethod = (LuaMethod*)target;
        LuaUtil::PushRef(L, luaMethod->funcRef);
        return;
    }
    Il2CppObject* obj = reinterpret_cast<Il2CppObject*>(delegate);
    ObjectRegistry::Push(L, obj, obj->klass, metatableRefIndex);
}

Il2CppDelegate* Create(lua_State* L, Il2CppClass* delegateClass, int funcRef)
{
    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
    Il2CppMethodPointer bridge = DelegateBridge::Resolve(delegateClass);
    if (bridge == nullptr)
        LuaException::ThrowFormat("zlua: delegate bridge resolve failed, class:%s", MetadataUtil::GetTypeFullName(delegateClass));

    Il2CppClass* luaMethodClass = MetadataUtil::GetLuaMethodClass();
    LuaMethod* target = reinterpret_cast<LuaMethod*>(il2cpp::vm::Object::New(luaMethodClass));
    target->disposed = false;
    target->L = L;
    target->funcRef = funcRef;

    Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(il2cpp::vm::Object::New(delegateClass));
    il2cpp::vm::Type::ConstructClosedDelegate(delegate, reinterpret_cast<Il2CppObject*>(target), bridge, invokeMethod);
    return delegate;
}

Il2CppDelegate* DelegateMarshal::Pop(lua_State* L, int delegateIndex, Il2CppClass* delegateClass)
{
    Il2CppClass* luaMethodClass = MetadataUtil::GetLuaMethodClass();
    int type = lua_type(L, delegateIndex);
    if (type == LUA_TFUNCTION)
    {
        int funcRef = LuaUtil::ToLuaRef(L, delegateIndex);
        return Create(L, delegateClass, funcRef);
    }
    else if (type == LUA_TUSERDATA)
    {
        Il2CppObject* obj = ObjectRegistry::Pop(L, delegateIndex);
        if (obj == nullptr || obj->klass != delegateClass)
        {
            LuaException::Throw("zlua: argument mismatch: expected delegate");
        }
        return (Il2CppDelegate*)obj;
    }
    LuaException::Throw("zlua: argument mismatch: expected delegate");
    return nullptr;
}
}