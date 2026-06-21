#pragma once

#include "novalua/LuaInvokeRuntime.h"

namespace novalua
{
    extern LuaInvokeSite kSite_Bootstrap_AppAdd;
    extern LuaInvokeSite kSite_Bootstrap_AppMain;

    void InitLuaInvokeSites();
    void RegisterGeneratedInternalCalls();
}
