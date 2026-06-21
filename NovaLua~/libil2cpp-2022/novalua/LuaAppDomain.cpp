#include "LuaAppDomain.h"

#include "BuiltinScripts.h"
#include "LuaEnv.h"
#include "LuaInteropManager.h"

#include "generated/LuaInvokeSites.h"

#include "vm/InternalCalls.h"
#include "il2cpp-object-internals.h"

namespace novalua
{
    static void LuaIl2CppAppDomain_InitializeInternal(Il2CppDelegate* moduleLoader)
    {
        LuaAppDomain::InitializeFromManaged(moduleLoader);
    }

    static void RegisterCoreInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add(
            "NovaLua.LuaIl2CppAppDomain::InitializeInternal",
            (Il2CppMethodPointer)LuaIl2CppAppDomain_InitializeInternal);
    }

    void LuaAppDomain::Initialize()
    {
        RegisterCoreInternalCalls();
        RegisterGeneratedInternalCalls();
    }

    void LuaAppDomain::InitializeFromManaged(void* moduleLoaderDelegate)
    {
        LuaEnv::RegisterRoots();
        LuaEnv::Create((Il2CppDelegate*)moduleLoaderDelegate);
        BuiltinScripts::LoadGlobals();
        LuaInteropManager::RegisterNovaLuaApi();
        BuiltinScripts::LoadNovaLuaLib();
        InitLuaInvokeSites();
    }
}
