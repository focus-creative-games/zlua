/*
** ZLua sealed-metatable support for PUC-Rio Lua 5.4 VM.
*/
#define LUA_CORE

#include "lprefix.h"

#include "lua.h"
#include "lauxlib.h"

#include "lapi.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"
#include "ltm.h"
#include "zlua_fastmt.h"

#if ZLUA_FAST_METATABLE

#define ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/*
** index2value is static in lapi.c; keep a local copy for seal.
*/
static TValue *zlua_index2value (lua_State *L, int idx) {
  CallInfo *ci = L->ci;
  if (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->top.p - (ci->func.p + 1), "unacceptable index");
    if (o >= L->top.p) return &G(L)->nilvalue;
    else return s2v(o);
  }
  else if (!ispseudo(idx)) {
    api_check(L, idx != 0 && -idx <= L->top.p - (ci->func.p + 1),
                 "invalid index");
    return s2v(L->top.p + idx);
  }
  else if (idx == LUA_REGISTRYINDEX)
    return &G(L)->l_registry;
  else {
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    if (ttisCclosure(s2v(ci->func.p))) {
      CClosure *func = clCvalue(s2v(ci->func.p));
      return (idx <= func->nupvalues) ? &func->upvalue[idx - 1]
                                      : &G(L)->nilvalue;
    }
    else {
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      return &G(L)->nilvalue;
    }
  }
}


LUA_API void zlua_mt_seal (lua_State *L, int mtIndex) {
  Table *mt;
  const TValue *idx;
  const TValue *nidx;
  TValue *o;
  lua_lock(L);
  o = zlua_index2value(L, mtIndex);
  api_check(L, ttistable(o), "table expected");
  mt = hvalue(o);
  idx = luaH_getshortstr(mt, G(L)->tmname[TM_INDEX]);
  nidx = luaH_getshortstr(mt, G(L)->tmname[TM_NEWINDEX]);
  if (l_unlikely(isempty(idx) || !ttistable(idx))) {
    lua_unlock(L);
    luaL_error(L, "zlua_mt_seal: __index must be a table");
  }
  if (l_unlikely(isempty(nidx) || !ttistable(nidx))) {
    lua_unlock(L);
    luaL_error(L, "zlua_mt_seal: __newindex must be a table");
  }
  setobj(L, &mt->zlua_index, idx);
  setobj(L, &mt->zlua_newindex, nidx);
  luaC_barrierback(L, obj2gco(mt), idx);
  luaC_barrierback(L, obj2gco(mt), nidx);
  mt->zlua_mt_kind = 1;
  lua_unlock(L);
}


LUA_API void zlua_vm_call_getter (lua_State *L, const TValue *f,
                                  const TValue *obj, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->top.p;
  setobj2s(L, func, f);
  setobj2s(L, func + 1, obj);
  L->top.p += 2;
  if (isLuacode(L->ci))
    luaD_call(L, func, 1);
  else
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->top.p);
}


LUA_API void zlua_vm_call_setter (lua_State *L, const TValue *f,
                                  const TValue *obj, const TValue *val) {
  StkId func = L->top.p;
  setobj2s(L, func, f);
  setobj2s(L, func + 1, obj);
  setobj2s(L, func + 2, val);
  L->top.p += 3;
  if (isLuacode(L->ci))
    luaD_call(L, func, 0);
  else
    luaD_callnoyield(L, func, 0);
}

#endif /* ZLUA_FAST_METATABLE */
