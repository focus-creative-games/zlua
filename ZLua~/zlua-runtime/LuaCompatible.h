// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

/*
 * Read a Lua number as int64/uint64.
 * Lua 5.1/5.2: lua_tointeger uses lua_number2integer; with LUA_IEEE754TRICK that
 * macro only materializes a 32-bit lane even when lua_Integer is 64-bit (Win64/LP64).
 * Values outside int32 then become 0 / garbage. Use tonumber cast instead.
 * Lua 5.3+ has a real integer subtype; lua_tointeger is correct there.
 */
inline int64_t zlua_to_int64(lua_State* L, int idx)
{
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 503)
    return (int64_t)lua_tonumber(L, idx);
#else
    return (int64_t)lua_tointeger(L, idx);
#endif
}

inline uint64_t zlua_to_uint64(lua_State* L, int idx)
{
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 503)
    return (uint64_t)lua_tonumber(L, idx);
#else
    if (lua_isinteger(L, idx))
        return (uint64_t)lua_tointeger(L, idx);
    return (uint64_t)lua_tonumber(L, idx);
#endif
}

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

/*
 * Lua 5.2+ / LuaJIT: luaL_traceback.
 * Official Lua 5.1 lacks it — polyfill via debug.traceback (openlibs provides debug).
 * level is the same as luaL_traceback / luaL_where (1 = first Lua caller of the current C function).
 */
#if !ZLUA_USE_LUAJIT && (ZLUA_LUA_API_FAMILY < 502)
#if !defined(luaL_traceback)
inline void zlua_luaL_traceback(lua_State* L, lua_State* L1, const char* msg, int level)
{
    if (L != L1)
    {
        lua_pushstring(L, msg != nullptr ? msg : "");
        return;
    }

    const int top = lua_gettop(L);
    lua_getglobal(L, "debug");
    if (lua_type(L, -1) != LUA_TTABLE)
    {
        lua_settop(L, top);
        lua_pushstring(L, msg != nullptr ? msg : "");
        return;
    }

    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TFUNCTION)
    {
        lua_settop(L, top);
        lua_pushstring(L, msg != nullptr ? msg : "");
        return;
    }

    if (msg != nullptr)
        lua_pushstring(L, msg);
    else
        lua_pushnil(L);
    /* lua_pcall(debug.traceback) inserts one frame; bump level to match native luaL_traceback. */
    lua_pushinteger(L, level + 1);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK)
    {
        lua_settop(L, top);
        lua_pushstring(L, msg != nullptr ? msg : "");
    }
}
#define luaL_traceback(L, L1, msg, level) zlua_luaL_traceback((L), (L1), (msg), (level))
#endif
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
