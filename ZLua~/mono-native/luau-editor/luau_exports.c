/* C ABI exports. Implementations live in luau_editor_dll.cpp (C++). */
#include <stddef.h>
#include <stdint.h>

typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State* L);

#ifdef __cplusplus
extern "C" {
#endif

lua_State* zlua_luaL_newstate(void);
void zlua_luaL_openlibs(lua_State* L);
void zlua_lua_close(lua_State* L);
int zlua_lua_gettop(lua_State* L);
void zlua_lua_settop(lua_State* L, int idx);
void zlua_lua_remove(lua_State* L, int idx);
void zlua_lua_insert(lua_State* L, int idx);
void zlua_lua_replace(lua_State* L, int idx);
void zlua_lua_pushnil(lua_State* L);
void zlua_lua_pushvalue(lua_State* L, int idx);
void zlua_lua_pushinteger(lua_State* L, int64_t value);
void zlua_lua_pushnumber(lua_State* L, double value);
void zlua_lua_pushboolean(lua_State* L, int value);
const char* zlua_lua_pushstring(lua_State* L, const char* s);
const char* zlua_lua_pushlstring(lua_State* L, const char* s, size_t len);
void zlua_lua_pushlightuserdata(lua_State* L, void* p);
void zlua_lua_pushcclosure(lua_State* L, lua_CFunction fn, int nup);
void zlua_lua_getfield(lua_State* L, int idx, const char* k);
void zlua_lua_setfield(lua_State* L, int idx, const char* k);
void zlua_lua_rawgeti(lua_State* L, int idx, int n);
void zlua_lua_rawget(lua_State* L, int idx);
void zlua_lua_rawset(lua_State* L, int idx);
void zlua_lua_rawseti(lua_State* L, int idx, int n);
int zlua_lua_setmetatable(lua_State* L, int obj);
int zlua_lua_getmetatable(lua_State* L, int obj);
void zlua_lua_createtable(lua_State* L, int na, int nr);
void* zlua_lua_newuserdata(lua_State* L, size_t sz);
int zlua_luaL_ref(lua_State* L, int t);
void zlua_luaL_unref(lua_State* L, int t, int ref);
int zlua_lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);
int zlua_lua_error(lua_State* L);
void zlua_luaL_where(lua_State* L, int level);
void zlua_luaL_traceback(lua_State* L, lua_State* L1, const char* msg, int level);
int64_t zlua_lua_tointeger(lua_State* L, int idx);
double zlua_lua_tonumber(lua_State* L, int idx);
int zlua_lua_type(lua_State* L, int idx);
int zlua_lua_toboolean(lua_State* L, int idx);
void* zlua_lua_touserdata(lua_State* L, int idx);
const void* zlua_lua_topointer(lua_State* L, int idx);
int zlua_lua_iscfunction(lua_State* L, int idx);
const char* zlua_lua_getupvalue(lua_State* L, int func, int n);
const char* zlua_lua_tolstring(lua_State* L, int idx, size_t* len);
const char* zlua_luaL_tolstring(lua_State* L, int idx, size_t* len);
const char* zlua_luaL_checklstring(lua_State* L, int arg, size_t* len);
int zlua_luaL_loadstring(lua_State* L, const char* s);
int zlua_luaL_loadbuffer(lua_State* L, const char* buff, size_t sz, const char* name);

__declspec(dllexport) lua_State* luaL_newstate(void) { return zlua_luaL_newstate(); }
__declspec(dllexport) void luaL_openlibs(lua_State* L) { zlua_luaL_openlibs(L); }
__declspec(dllexport) void lua_close(lua_State* L) { zlua_lua_close(L); }
__declspec(dllexport) int lua_gettop(lua_State* L) { return zlua_lua_gettop(L); }
__declspec(dllexport) void lua_settop(lua_State* L, int idx) { zlua_lua_settop(L, idx); }
__declspec(dllexport) void lua_remove(lua_State* L, int idx) { zlua_lua_remove(L, idx); }
__declspec(dllexport) void lua_insert(lua_State* L, int idx) { zlua_lua_insert(L, idx); }
__declspec(dllexport) void lua_replace(lua_State* L, int idx) { zlua_lua_replace(L, idx); }
__declspec(dllexport) void lua_pushnil(lua_State* L) { zlua_lua_pushnil(L); }
__declspec(dllexport) void lua_pushvalue(lua_State* L, int idx) { zlua_lua_pushvalue(L, idx); }
__declspec(dllexport) void lua_pushinteger(lua_State* L, int64_t value) { zlua_lua_pushinteger(L, value); }
__declspec(dllexport) void lua_pushnumber(lua_State* L, double value) { zlua_lua_pushnumber(L, value); }
__declspec(dllexport) void lua_pushboolean(lua_State* L, int value) { zlua_lua_pushboolean(L, value); }
__declspec(dllexport) const char* lua_pushstring(lua_State* L, const char* s) { return zlua_lua_pushstring(L, s); }
__declspec(dllexport) const char* lua_pushlstring(lua_State* L, const char* s, size_t len) { return zlua_lua_pushlstring(L, s, len); }
__declspec(dllexport) void lua_pushlightuserdata(lua_State* L, void* p) { zlua_lua_pushlightuserdata(L, p); }
__declspec(dllexport) void lua_pushcclosure(lua_State* L, lua_CFunction fn, int nup) { zlua_lua_pushcclosure(L, fn, nup); }
__declspec(dllexport) void lua_getfield(lua_State* L, int idx, const char* k) { zlua_lua_getfield(L, idx, k); }
__declspec(dllexport) void lua_setfield(lua_State* L, int idx, const char* k) { zlua_lua_setfield(L, idx, k); }
__declspec(dllexport) void lua_rawgeti(lua_State* L, int idx, int n) { zlua_lua_rawgeti(L, idx, n); }
__declspec(dllexport) void lua_rawget(lua_State* L, int idx) { zlua_lua_rawget(L, idx); }
__declspec(dllexport) void lua_rawset(lua_State* L, int idx) { zlua_lua_rawset(L, idx); }
__declspec(dllexport) void lua_rawseti(lua_State* L, int idx, int n) { zlua_lua_rawseti(L, idx, n); }
__declspec(dllexport) int lua_setmetatable(lua_State* L, int obj) { return zlua_lua_setmetatable(L, obj); }
__declspec(dllexport) int lua_getmetatable(lua_State* L, int obj) { return zlua_lua_getmetatable(L, obj); }
__declspec(dllexport) void lua_createtable(lua_State* L, int na, int nr) { zlua_lua_createtable(L, na, nr); }
__declspec(dllexport) void* lua_newuserdata(lua_State* L, size_t sz) { return zlua_lua_newuserdata(L, sz); }
__declspec(dllexport) int luaL_ref(lua_State* L, int t) { return zlua_luaL_ref(L, t); }
__declspec(dllexport) void luaL_unref(lua_State* L, int t, int ref) { zlua_luaL_unref(L, t, ref); }
__declspec(dllexport) int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc) { return zlua_lua_pcall(L, nargs, nresults, errfunc); }
__declspec(dllexport) int lua_error(lua_State* L) { return zlua_lua_error(L); }
__declspec(dllexport) void luaL_where(lua_State* L, int level) { zlua_luaL_where(L, level); }
__declspec(dllexport) void luaL_traceback(lua_State* L, lua_State* L1, const char* msg, int level) { zlua_luaL_traceback(L, L1, msg, level); }
__declspec(dllexport) int64_t lua_tointeger(lua_State* L, int idx) { return zlua_lua_tointeger(L, idx); }
__declspec(dllexport) double lua_tonumber(lua_State* L, int idx) { return zlua_lua_tonumber(L, idx); }
__declspec(dllexport) int lua_type(lua_State* L, int idx) { return zlua_lua_type(L, idx); }
__declspec(dllexport) int lua_toboolean(lua_State* L, int idx) { return zlua_lua_toboolean(L, idx); }
__declspec(dllexport) void* lua_touserdata(lua_State* L, int idx) { return zlua_lua_touserdata(L, idx); }
__declspec(dllexport) const void* lua_topointer(lua_State* L, int idx) { return zlua_lua_topointer(L, idx); }
__declspec(dllexport) int lua_iscfunction(lua_State* L, int idx) { return zlua_lua_iscfunction(L, idx); }
__declspec(dllexport) const char* lua_getupvalue(lua_State* L, int func, int n) { return zlua_lua_getupvalue(L, func, n); }
__declspec(dllexport) const char* lua_tolstring(lua_State* L, int idx, size_t* len) { return zlua_lua_tolstring(L, idx, len); }
__declspec(dllexport) const char* luaL_tolstring(lua_State* L, int idx, size_t* len) { return zlua_luaL_tolstring(L, idx, len); }
__declspec(dllexport) const char* luaL_checklstring(lua_State* L, int arg, size_t* len) { return zlua_luaL_checklstring(L, arg, len); }
__declspec(dllexport) int luaL_loadstring(lua_State* L, const char* s) { return zlua_luaL_loadstring(L, s); }
__declspec(dllexport) int luaL_loadbuffer(lua_State* L, const char* buff, size_t sz, const char* name) { return zlua_luaL_loadbuffer(L, buff, sz, name); }

#ifdef __cplusplus
}
#endif
