#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class LuaUtil
{
  public:
    // Lua number is integer-valued (integer tag, or number with no fractional part).
    static bool IsStrictLuaInteger(lua_State* L, int index, lua_Integer& value);
    static int PushRef(lua_State* L, int refIndex);
    static int PCallClosureRefAt(lua_State* L, int closureRef, const int* argStackIndices, int argCount, int resultCount);

    static void PCall(lua_State* L, int nargs, int nresults, int errfunc);

    static int ToLuaRef(lua_State* L)
    {
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }

    static int ToLuaRef(lua_State* L, int index)
    {
        lua_pushvalue(L, index);
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }
};
} // namespace zlua
