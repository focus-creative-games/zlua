// Windows Editor DLL. Exports a PUC-5.1-shaped C ABI over Luau (C++ linkage).
// Unity Mono P/Invoke cannot bind Luau's mangled symbols.

#include "lua.h"
#include "lualib.h"
#include "luacode.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace
{
using ErrorFn = void (*)(lua_State*);
using GetFieldFn = int (*)(lua_State*, int, const char*);
using RawGetFn = int (*)(lua_State*, int);
using RawGetIFn = int (*)(lua_State*, int, int);
using TypeFn = int (*)(lua_State*, int);
using ToNumberFn = double (*)(lua_State*, int, int*);
using ToInteger64Fn = int64_t (*)(lua_State*, int, int*);

ErrorFn kLuaError = ::lua_error;
GetFieldFn kGetField = ::lua_getfield;
RawGetFn kRawGet = ::lua_rawget;
RawGetIFn kRawGetI = ::lua_rawgeti;
TypeFn kType = ::lua_type;
ToNumberFn kToNumber = ::lua_tonumberx;
ToInteger64Fn kToInteger64 = ::lua_tointeger64;

const int kTagByObj = 1;
const int kTagByVal = 2;
lua_CFunction g_release[3];

void CallRelease(lua_State* L, void* data, int tag)
{
    lua_CFunction fn = g_release[tag];
    if (fn == nullptr || data == nullptr)
        return;
    int top = ::lua_gettop(L);
    ::lua_pushlightuserdata(L, data);
    fn(L);
    ::lua_settop(L, top);
}

void DtorByObj(lua_State* L, void* data) { CallRelease(L, data, kTagByObj); }
void DtorByVal(lua_State* L, void* data) { CallRelease(L, data, kTagByVal); }

int MapType(int t)
{
    if (t == LUA_TNONE)
        return -1;
    if (t <= LUA_TNUMBER)
        return t;
    if (t == LUA_TINTEGER)
        return LUA_TNUMBER;
    if (t == LUA_TSTRING)
        return 4;
    if (t == LUA_TTABLE)
        return 5;
    if (t == LUA_TFUNCTION)
        return 6;
    if (t == LUA_TUSERDATA)
        return 7;
    if (t == LUA_TTHREAD)
        return 8;
    return t;
}

int LoadBuffer(lua_State* L, const char* buff, size_t sz, const char* name)
{
    lua_setsafeenv(L, LUA_ENVIRONINDEX, 0);
    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(buff, sz, nullptr, &bytecodeSize);
    if (bytecode == nullptr)
        return LUA_ERRMEM;
    int status = luau_load(L, name != nullptr ? name : "=zlua", bytecode, bytecodeSize, 0);
    free(bytecode);
    return status;
}

void InstallHost(lua_State* L)
{
    lua_pushcclosurek(L, [](lua_State* state) -> int {
        size_t len = 0;
        const char* source = ::luaL_checklstring(state, 1, &len);
        const char* chunkName = luaL_optstring(state, 2, "=loadstring");
        if (LoadBuffer(state, source, len, chunkName) == LUA_OK)
            return 1;
        ::lua_pushnil(state);
        ::lua_insert(state, -2);
        return 2;
    }, "loadstring", 0, nullptr);
    lua_setglobal(L, "loadstring");
    lua_getglobal(L, "loadstring");
    lua_setglobal(L, "load");

    const char* bootstrap =
        "package = { loaded = {}, searchers = {} }\n"
        "function require(modname)\n"
        "    local loaded = package.loaded[modname]\n"
        "    if loaded ~= nil then return loaded end\n"
        "    local searchers = package.searchers\n"
        "    for i = 1, #searchers do\n"
        "        local loader = searchers[i](modname)\n"
        "        if type(loader) == \"function\" then\n"
        "            local result = loader(modname)\n"
        "            if result == nil then result = true end\n"
        "            package.loaded[modname] = result\n"
        "            return result\n"
        "        end\n"
        "    end\n"
        "    error(\"module '\" .. tostring(modname) .. \"' not found\", 2)\n"
        "end\n";
    if (LoadBuffer(L, bootstrap, strlen(bootstrap), "=zlua.host") != LUA_OK || ::lua_pcall(L, 0, 0, 0) != LUA_OK)
        lua_pop(L, 1);
}
} // namespace

extern "C"
{

lua_State* zlua_luaL_newstate(void)
{
    return ::luaL_newstate();
}

void zlua_luaL_openlibs(lua_State* L)
{
    ::luaL_openlibs(L);
    InstallHost(L);
}

void zlua_lua_close(lua_State* L) { ::lua_close(L); }
int zlua_lua_gettop(lua_State* L) { return ::lua_gettop(L); }
void zlua_lua_settop(lua_State* L, int idx) { ::lua_settop(L, idx); }
void zlua_lua_remove(lua_State* L, int idx) { ::lua_remove(L, idx); }
void zlua_lua_insert(lua_State* L, int idx) { ::lua_insert(L, idx); }
void zlua_lua_replace(lua_State* L, int idx) { ::lua_replace(L, idx); }
void zlua_lua_pushnil(lua_State* L) { ::lua_pushnil(L); }
void zlua_lua_pushvalue(lua_State* L, int idx) { ::lua_pushvalue(L, idx); }

void zlua_lua_pushinteger(lua_State* L, int64_t value)
{
    ::lua_pushnumber(L, (double)value);
}

void zlua_lua_pushnumber(lua_State* L, double value) { ::lua_pushnumber(L, value); }
void zlua_lua_pushboolean(lua_State* L, int value) { ::lua_pushboolean(L, value); }
const char* zlua_lua_pushstring(lua_State* L, const char* s)
{
    ::lua_pushstring(L, s);
    return ::lua_tolstring(L, -1, nullptr);
}
const char* zlua_lua_pushlstring(lua_State* L, const char* s, size_t len)
{
    ::lua_pushlstring(L, s, len);
    return ::lua_tolstring(L, -1, nullptr);
}
void zlua_lua_pushlightuserdata(lua_State* L, void* p) { ::lua_pushlightuserdata(L, p); }

void zlua_lua_pushcclosure(lua_State* L, lua_CFunction fn, int nup)
{
    ::lua_pushcclosurek(L, fn, "", nup, nullptr);
}

void zlua_lua_getfield(lua_State* L, int idx, const char* k) { kGetField(L, idx, k); }
void zlua_lua_setfield(lua_State* L, int idx, const char* k) { ::lua_setfield(L, idx, k); }
void zlua_lua_rawgeti(lua_State* L, int idx, int n) { kRawGetI(L, idx, n); }
void zlua_lua_rawget(lua_State* L, int idx) { kRawGet(L, idx); }
void zlua_lua_rawset(lua_State* L, int idx) { ::lua_rawset(L, idx); }
void zlua_lua_rawseti(lua_State* L, int idx, int n) { ::lua_rawseti(L, idx, n); }
int zlua_lua_setmetatable(lua_State* L, int obj) { return ::lua_setmetatable(L, obj); }
int zlua_lua_getmetatable(lua_State* L, int obj) { return ::lua_getmetatable(L, obj); }
void zlua_lua_createtable(lua_State* L, int na, int nr) { ::lua_createtable(L, na, nr); }

void* zlua_lua_newuserdata(lua_State* L, size_t sz)
{
    void* p = ::lua_newuserdatatagged(L, sz, 0);
    if (p != nullptr && sz > 0)
        memset(p, 0, sz);
    return p;
}

__declspec(dllexport) void* zlua_luau_newuserdata(lua_State* L, size_t sz, int tag)
{
    void* p = ::lua_newuserdatatagged(L, sz, tag);
    if (p != nullptr && sz > 0)
        memset(p, 0, sz);
    return p;
}

__declspec(dllexport) void zlua_luau_set_dtor(lua_State* L, int tag, lua_CFunction fn)
{
    if (tag == kTagByObj || tag == kTagByVal)
        g_release[tag] = fn;
    ::lua_setuserdatadtor(L, tag, tag == kTagByVal ? DtorByVal : DtorByObj);
}

int zlua_luaL_ref(lua_State* L, int t)
{
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        return -1;
    }
    t = ::lua_absindex(L, t);
    kRawGetI(L, t, 0);
    int ref = (int)kToInteger64(L, -1, nullptr);
    lua_pop(L, 1);
    if (ref != 0)
    {
        kRawGetI(L, t, ref);
        ::lua_rawseti(L, t, 0);
    }
    else
    {
        ref = (int)::lua_objlen(L, t) + 1;
    }
    ::lua_rawseti(L, t, ref);
    return ref;
}

void zlua_luaL_unref(lua_State* L, int t, int ref)
{
    if (ref <= 0)
        return;
    t = ::lua_absindex(L, t);
    kRawGetI(L, t, 0);
    ::lua_rawseti(L, t, ref);
    ::lua_pushinteger(L, ref);
    ::lua_rawseti(L, t, 0);
}

int zlua_lua_pcall(lua_State* L, int nargs, int nresults, int errfunc)
{
    return ::lua_pcall(L, nargs, nresults, errfunc);
}

int zlua_lua_error(lua_State* L)
{
    kLuaError(L);
    return 0;
}

void zlua_luaL_where(lua_State* L, int level) { ::luaL_where(L, level); }
void zlua_luaL_traceback(lua_State* L, lua_State* L1, const char* msg, int level)
{
    ::luaL_traceback(L, L1, msg, level);
}

int64_t zlua_lua_tointeger(lua_State* L, int idx)
{
    if (::lua_type(L, idx) == LUA_TINTEGER)
        return kToInteger64(L, idx, nullptr);
    return (int64_t)kToNumber(L, idx, nullptr);
}

double zlua_lua_tonumber(lua_State* L, int idx)
{
    if (::lua_type(L, idx) == LUA_TINTEGER)
        return (double)kToInteger64(L, idx, nullptr);
    return kToNumber(L, idx, nullptr);
}

int zlua_lua_type(lua_State* L, int idx) { return MapType(kType(L, idx)); }
int zlua_lua_toboolean(lua_State* L, int idx) { return ::lua_toboolean(L, idx); }
void* zlua_lua_touserdata(lua_State* L, int idx) { return ::lua_touserdata(L, idx); }
const void* zlua_lua_topointer(lua_State* L, int idx) { return ::lua_topointer(L, idx); }
int zlua_lua_iscfunction(lua_State* L, int idx) { return ::lua_iscfunction(L, idx); }
const char* zlua_lua_getupvalue(lua_State* L, int func, int n) { return ::lua_getupvalue(L, func, n); }
const char* zlua_lua_tolstring(lua_State* L, int idx, size_t* len) { return ::lua_tolstring(L, idx, len); }
const char* zlua_luaL_tolstring(lua_State* L, int idx, size_t* len) { return ::luaL_tolstring(L, idx, len); }
const char* zlua_luaL_checklstring(lua_State* L, int arg, size_t* len) { return ::luaL_checklstring(L, arg, len); }

int zlua_luaL_loadstring(lua_State* L, const char* s)
{
    size_t n = s != nullptr ? strlen(s) : 0;
    return LoadBuffer(L, s != nullptr ? s : "", n, "=zlua");
}

int zlua_luaL_loadbuffer(lua_State* L, const char* buff, size_t sz, const char* name)
{
    return LoadBuffer(L, buff != nullptr ? buff : "", sz, name);
}

} // extern "C"
