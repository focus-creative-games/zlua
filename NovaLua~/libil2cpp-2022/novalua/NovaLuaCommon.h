#pragma once

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

#include "il2cpp-config.h"
#include "il2cpp-object-internals.h"
#include "il2cpp-class-internals.h"

namespace novalua
{
    inline bool LuaTypeIsInteger(lua_State* L, int idx)
    {
        return lua_isinteger(L, idx) != 0;
    }
}
