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

/*
 * Editor Mono: lua_error must not run on a managed reverse-P/Invoke frame
 * (LuaJIT Win64 SEH; also unsafe with Unity Mono stack capture / longjmp edge cases).
 *
 * Gate: Lua calls this C function; it calls the managed lua_CFunction stored in
 * upvalue 1 (lightuserdata). Managed never calls lua_error -- on failure it
 * pushes the error object and returns ZLUA_CALLBACK_ERROR_SENTINEL. The gate
 * then calls lua_error after the managed frame has returned.
 *
 * Extra upvalues (2..) are visible to managed via lua_upvalueindex because the
 * managed body is invoked as a plain C call while the gate CClosure is still
 * the current CallInfo -- so managed must use upvalue index = logical + 1.
 *
 * upvalue_pseudo_base: LUA_GLOBALSINDEX (-10002) for Lua 5.1 / LuaJIT;
 *   LUA_REGISTRYINDEX for Lua 5.2+ (same as lua_upvalueindex macro base).
 *
 * Built as a separate DLL (does not patch Lua / LuaJIT sources).
 */
#include <stdint.h>

#define ZLUA_CALLBACK_ERROR_SENTINEL ((int)0xFFFF5A11)

typedef struct lua_State lua_State;
typedef int (*zlua_lua_CFunction)(lua_State* L);
typedef void* (*zlua_lua_touserdata_fn)(lua_State* L, int idx);
typedef int (*zlua_lua_error_fn)(lua_State* L);

static zlua_lua_touserdata_fn s_touserdata;
static zlua_lua_error_fn s_lua_error;
static int s_upvalue_pseudo_base = -10002; /* Lua 5.1 default until init */

#if defined(_WIN32)
#define ZLUA_GATE_EXPORT __declspec(dllexport)
#else
#define ZLUA_GATE_EXPORT
#endif

static int zlua_upvalueindex(int i)
{
    return s_upvalue_pseudo_base - i;
}

ZLUA_GATE_EXPORT void zlua_gate_init(
    zlua_lua_touserdata_fn touserdata,
    zlua_lua_error_fn lua_error_fn,
    int upvalue_pseudo_base)
{
    s_touserdata = touserdata;
    s_lua_error = lua_error_fn;
    s_upvalue_pseudo_base = upvalue_pseudo_base;
}

static int zlua_callback_gate(lua_State* L)
{
    zlua_lua_CFunction fn = (zlua_lua_CFunction)s_touserdata(L, zlua_upvalueindex(1));
    int n = fn(L);
    if (n == ZLUA_CALLBACK_ERROR_SENTINEL)
    {
        return s_lua_error(L);
    }

    return n;
}

ZLUA_GATE_EXPORT void* zlua_get_callback_gate(void)
{
    return (void*)&zlua_callback_gate;
}

ZLUA_GATE_EXPORT int zlua_callback_error_sentinel(void)
{
    return ZLUA_CALLBACK_ERROR_SENTINEL;
}
