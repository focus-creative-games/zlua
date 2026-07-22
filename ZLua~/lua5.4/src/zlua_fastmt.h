/*
** ZLua sealed-metatable helpers (see Docs/OPTIMIZATION.md).
** Only active when ZLUA_FAST_METATABLE != 0 (defined in luaconf.h).
*/
#ifndef zlua_fastmt_h
#define zlua_fastmt_h

#include "luaconf.h"
#include "lua.h"
#include "lobject.h"

#if ZLUA_FAST_METATABLE

/* Tagged field/property accessors; VM compares CClosure->f to these. */
LUA_API int zlua_index_getter (lua_State *L);  /* (obj) -> value */
LUA_API int zlua_index_setter (lua_State *L);  /* (obj, value) -> 0 */

/* After __index/__newindex tables are set on mt, cache them and mark sealed. */
LUA_API void zlua_mt_seal (lua_State *L, int mtIndex);

/* VM helpers: call tagged closures without going through __index. */
LUA_API void zlua_vm_call_getter (lua_State *L, const TValue *f,
                                  const TValue *obj, StkId res);
LUA_API void zlua_vm_call_setter (lua_State *L, const TValue *f,
                                  const TValue *obj, const TValue *val);

#define zlua_is_getter(o) \
  (ttisCclosure(o) && clCvalue(o)->f == zlua_index_getter)

#define zlua_is_setter(o) \
  (ttisCclosure(o) && clCvalue(o)->f == zlua_index_setter)

#endif /* ZLUA_FAST_METATABLE */

#endif
