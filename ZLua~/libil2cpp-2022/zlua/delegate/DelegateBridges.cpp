#include "DelegateBridges.h"

#include "LuaCallInvoker.h"
#include "LuaMethodTarget.h"

#include "il2cpp-api-types.h"
#include "il2cpp-object-internals.h"
#include "il2cpp-tabledefs.h"
#include "vm/Runtime.h"

namespace zlua
{
    static void Bridge_Action_int32(Il2CppObject* target, int32_t arg0, const MethodInfo* method)
    {
        (void)method;
        LuaMethodTarget* luaMethod = reinterpret_cast<LuaMethodTarget*>(target);
        if (luaMethod == nullptr)
            return;
        LuaCallInvoker::CallVoid(luaMethod->funcRef, [arg0](lua_State* L) { lua_pushinteger(L, arg0); }, 1);
    }

    static int32_t Bridge_Func_int32__int32(Il2CppObject* target, int32_t arg0, const MethodInfo* method)
    {
        (void)method;
        LuaMethodTarget* luaMethod = reinterpret_cast<LuaMethodTarget*>(target);
        if (luaMethod == nullptr)
            return 0;
        int32_t ret = 0;
        if (!LuaCallInvoker::CallRetInt32(luaMethod->funcRef, arg0, &ret))
            return 0;
        return ret;
    }

    static bool MatchesInvokeSignature(const MethodInfo* invokeMethod, Il2CppTypeEnum returnType, int paramCount, Il2CppTypeEnum param0Type)
    {
        if (invokeMethod == nullptr)
            return false;
        if (invokeMethod->parameters_count != paramCount)
            return false;
        if (invokeMethod->return_type == nullptr || invokeMethod->return_type->type != returnType)
            return false;
        if (paramCount == 0)
            return true;
        if (invokeMethod->parameters[0] == nullptr || invokeMethod->parameters[0]->type != param0Type)
            return false;
        return true;
    }

    void* DelegateBridges::Resolve(Il2CppClass* delegateClass)
    {
        if (delegateClass == nullptr)
            return nullptr;

        const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
        if (MatchesInvokeSignature(invokeMethod, IL2CPP_TYPE_VOID, 1, IL2CPP_TYPE_I4))
            return reinterpret_cast<void*>(Bridge_Action_int32);
        if (MatchesInvokeSignature(invokeMethod, IL2CPP_TYPE_I4, 1, IL2CPP_TYPE_I4))
            return reinterpret_cast<void*>(Bridge_Func_int32__int32);
        return nullptr;
    }
}
