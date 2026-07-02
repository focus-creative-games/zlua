#include "LuaAppDomain.h"

#include "BuiltinScripts.h"
#include "LuaEnv.h"

#include "generated/LuaInvokeSites.h"

#include "vm/InternalCalls.h"
#include "il2cpp-object-internals.h"

namespace zlua
{
    static void LuaIl2CppAppDomain_InitializeInternal(Il2CppDelegate* moduleLoader)
    {
        LuaAppDomain::InitializeFromManaged(moduleLoader);
    }

    static void LuaMethod_AddPendingRef(void* /*L*/, int32_t refIndex)
    {
        LuaEnv::AddPendingRef(refIndex);
    }

    static void LuaIl2CppAppDomain_ProcessPendingRefReleases()
    {
        LuaEnv::ProcessPendingRefReleases();
    }

    static void RegisterCoreInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add(
            "ZLua.LuaIl2CppAppDomain::InitializeInternal",
            (Il2CppMethodPointer)LuaIl2CppAppDomain_InitializeInternal);
        il2cpp::vm::InternalCalls::Add(
            "ZLua.LuaIl2CppAppDomain::ProcessPendingRefReleases",
            (Il2CppMethodPointer)LuaIl2CppAppDomain_ProcessPendingRefReleases);
        il2cpp::vm::InternalCalls::Add(
            "ZLua.LuaMethod::AddPendingRef",
            (Il2CppMethodPointer)LuaMethod_AddPendingRef);
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
        LuaEnv::RegisterZLuaApi();
        BuiltinScripts::LoadZLuaLib();
        InitLuaInvokeSites();
        LuaEnv::DoStringIgnoreResult("_G.__ZLUA_IL2CPP_PLAYER__=true");
        LuaEnv::DoStringIgnoreResult("if type(CSharp)=='table' and CSharp['ZLua.Tests'] then CSharp.T=CSharp['ZLua.Tests'] end");
    }
}
