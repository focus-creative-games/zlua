#pragma once

#include "zlua/LuaInvokeRuntime.h"

namespace zlua
{
    extern LuaInvokeSite kSite_Bootstrap_AppAdd;
    extern LuaInvokeSite kSite_Bootstrap_AppMain;

    void InitLuaInvokeSites();
    void RegisterGeneratedInternalCalls();
}
