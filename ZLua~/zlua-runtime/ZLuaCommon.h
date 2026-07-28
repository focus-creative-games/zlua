#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "generated/ZLuaConf.inc"
#include "LuaCompatible.h"
#include "Il2CppCompatible.h"

#ifndef ZLUA_LUA_VERSION
#define ZLUA_LUA_VERSION LUA_VERSION_NUM
#endif

#define ZLUA_DEBUG IL2CPP_DEBUG
#define ZLUA_ASSERT(cond) IL2CPP_ASSERT(cond)

#if defined(_MSC_VER)
#define zlua_strdup _strdup
#else
#define zlua_strdup strdup
#endif

#if IL2CPP_SIZEOF_VOID_P == 8
#define ZLUA_ARCH_64 1
#else
#define ZLUA_ARCH_32 1
#endif

/*
 * ZLUA_FAST_METATABLE is defined in lua/luaconf.h (default 0).
 * Must not be redefined by ZLuaConf / Compatible (Table ABI vs VM).
 */
#ifndef ZLUA_FAST_METATABLE
#error "ZLUA_FAST_METATABLE must be defined by lua/luaconf.h"
#endif

#if ZLUA_USE_LUAJIT && ZLUA_FAST_METATABLE
#error "ZLUA_FAST_METATABLE must be 0 when ZLUA_USE_LUAJIT is enabled"
#endif
