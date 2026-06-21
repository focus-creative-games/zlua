#pragma once

#include "novalua/LuaInvokeRuntime.h"

namespace novalua
{
    extern LuaInvokeSite kSite_Bootstrap_AppMain;
    extern LuaInvokeSite kSite_Bootstrap_AppAdd;

    void InitLuaInvokeSites();
    void RegisterGeneratedInternalCalls();
}
