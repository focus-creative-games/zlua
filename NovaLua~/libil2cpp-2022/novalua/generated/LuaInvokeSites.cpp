#include "LuaInvokeSites.h"

#include "novalua/LuaEnv.h"
#include "novalua/LuaInvokeRuntime.h"

#include "vm/InternalCalls.h"

namespace novalua
{
    // Stub sites; Player build codegen overwrites this file.
    LuaInvokeSite kSite_Bootstrap_AppMain = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_Bootstrap_AppAdd = { LUA_NOREF, LUA_NOREF };

    void InitLuaInvokeSites()
    {
        kSite_Bootstrap_AppMain.moduleRef = LuaEnv::GetModuleRef("app");
        kSite_Bootstrap_AppMain.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("app", "main");

        kSite_Bootstrap_AppAdd.moduleRef = LuaEnv::GetModuleRef("app");
        kSite_Bootstrap_AppAdd.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("app", "add");
    }

    static void IC_Bootstrap_AppMain()
    {
        LuaInvokeRuntime::CallVoid(kSite_Bootstrap_AppMain);
    }

    static int32_t IC_Bootstrap_AppAdd(int32_t a, int32_t b)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_Bootstrap_AppAdd, a, b);
    }

    void RegisterGeneratedInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add("Bootstrap::AppMain", (Il2CppMethodPointer)IC_Bootstrap_AppMain);
        il2cpp::vm::InternalCalls::Add("Bootstrap::AppAdd", (Il2CppMethodPointer)IC_Bootstrap_AppAdd);
    }
}
