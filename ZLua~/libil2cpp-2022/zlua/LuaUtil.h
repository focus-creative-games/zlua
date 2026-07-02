#pragma once

#include "ZLuaCommon.h"

namespace zlua
{
    class LuaUtil
    {
    public:
        // Lua number is integer-valued (integer tag, or number with no fractional part).
        static bool IsStrictLuaInteger(lua_State* L, int index);
      static int RaiseAsLuaError(lua_State* L, const char* msg, Il2CppException* ex);
    };
}
