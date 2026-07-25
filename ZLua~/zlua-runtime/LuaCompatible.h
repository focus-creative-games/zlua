#pragma once

/*
 * Multi-Lua / LuaJIT API shim (spec 11-MULTI-VERSION §12).
 * Assumes generated/ZLuaConf.inc was included by ZLuaCommon.h first.
 */

#ifndef ZLUA_LUA_API_FAMILY
#error "ZLUA_LUA_API_FAMILY must be defined by generated/ZLuaConf.inc"
#endif

#if ZLUA_USE_LUAJIT
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#elif ZLUA_LUA_API_FAMILY < 502
// Lua 5.1: upstream has no lua.hpp (added in 5.2).
extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}
#else
#include "lua/lua.hpp"
#endif

#include <stdint.h>

/* Lua 5.2+: LUA_OK. 5.1 documents success as status 0 without the name. */
#if !defined(LUA_OK)
#define LUA_OK 0
#endif

/* Lua 5.3+: LUA_MAXINTEGER / LUA_MININTEGER. 5.1/5.2/JIT: lua_Integer ~= ptrdiff_t. */
#if !defined(LUA_MAXINTEGER)
#define LUA_MAXINTEGER ((lua_Integer)INTPTR_MAX)
#define LUA_MININTEGER ((lua_Integer)INTPTR_MIN)
#endif

/* Lua 5.3+: lua_isinteger. Approximate: number equal to its integer truncation. */
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 503)
#if !defined(lua_isinteger)
inline int zlua_lua_isinteger(lua_State* L, int idx)
{
    if (lua_type(L, idx) != LUA_TNUMBER)
        return 0;
    const lua_Number n = lua_tonumber(L, idx);
    return n == (lua_Number)(lua_Integer)n;
}
#define lua_isinteger(L, i) zlua_lua_isinteger((L), (i))
#endif
#endif

/* Lua 5.2+: lua_absindex. */
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 502)
#if !defined(lua_absindex)
inline int zlua_lua_absindex(lua_State* L, int idx)
{
    return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
}
#define lua_absindex(L, i) zlua_lua_absindex((L), (i))
#endif
#endif

/*
 * Lua 5.3+: lua_getfield / lua_getglobal return the type of the pushed value.
 * 5.1/5.2/JIT: void (5.1 getglobal is a macro). Wrap so callers can compare to LUA_T*.
 */
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 503)
inline int zlua_lua_getfield(lua_State* L, int idx, const char* k)
{
    (lua_getfield)(L, idx, k);
    return lua_type(L, -1);
}
#undef lua_getfield
#define lua_getfield(L, idx, k) zlua_lua_getfield((L), (idx), (k))

#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 502)
#undef lua_getglobal
#define lua_getglobal(L, s) lua_getfield((L), LUA_GLOBALSINDEX, (s))
#else
inline int zlua_lua_getglobal(lua_State* L, const char* name)
{
    (lua_getglobal)(L, name);
    return lua_type(L, -1);
}
#undef lua_getglobal
#define lua_getglobal(L, s) zlua_lua_getglobal((L), (s))
#endif
#endif

/* Lua 5.2+: luaL_tolstring (used by print). */
#if !defined(luaL_tolstring)
inline const char* zlua_luaL_tolstring(lua_State* L, int idx, size_t* len)
{
    if (!luaL_callmeta(L, idx, "__tostring"))
    {
        switch (lua_type(L, idx))
        {
        case LUA_TNUMBER:
        case LUA_TSTRING:
            lua_pushvalue(L, idx);
            break;
        case LUA_TBOOLEAN:
            lua_pushstring(L, lua_toboolean(L, idx) ? "true" : "false");
            break;
        case LUA_TNIL:
            lua_pushliteral(L, "nil");
            break;
        default:
            lua_pushfstring(L, "%s: %p", luaL_typename(L, idx), lua_topointer(L, idx));
            break;
        }
    }
    return lua_tolstring(L, -1, len);
}
#define luaL_tolstring(L, i, len) zlua_luaL_tolstring((L), (i), (len))
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
