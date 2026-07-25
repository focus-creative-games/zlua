#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
    class ZLuaLib
    {
    public:
        static void RegisterGlobals(lua_State* L);
    };
}
