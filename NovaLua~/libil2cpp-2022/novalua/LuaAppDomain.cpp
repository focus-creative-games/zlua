#include "LuaAppDomain.h"

#include "vm/InternalCalls.h"
#include "il2cpp-object-internals.h"

namespace novalua
{
    static void LuaIl2CppAppDomain_InitializeInternal(Il2CppMulticastDelegate* luaLoader)
    {
        // Perform any necessary initialization for the Lua app domain here
    }

    static void RegisterInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add("NovaLua.LuaIl2CppAppDomain::InitializeInternal", (Il2CppMethodPointer)LuaIl2CppAppDomain_InitializeInternal);
    }

    void LuaAppDomain::Initialize()
    {
        RegisterInternalCalls();
    }
}