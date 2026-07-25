#pragma once

/*
 * Multi-Lua / LuaJIT API shim (spec 11-MULTI-VERSION §12).
 * Assumes generated/ZLuaConf.inc was included by ZLuaCommon.h first.
 */

#if ZLUA_USE_LUAJIT
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#else
#include "lua/lua.hpp"
#endif

#ifndef ZLUA_LUA_API_FAMILY
#error "ZLUA_LUA_API_FAMILY must be defined by generated/ZLuaConf.inc"
#endif

namespace zlua
{

/// <summary>Lua 5.4+ userdata with 0 uservalues; older API families use lua_newuserdata.</summary>
inline void* LuaNewUserData(lua_State* L, size_t size)
{
#if ZLUA_USE_LUAJIT
    return lua_newuserdata(L, size);
#elif (ZLUA_LUA_API_FAMILY >= 504) || (LUA_VERSION_NUM >= 504)
    return lua_newuserdatauv(L, size, 0);
#else
    return lua_newuserdata(L, size);
#endif
}

} // namespace zlua
