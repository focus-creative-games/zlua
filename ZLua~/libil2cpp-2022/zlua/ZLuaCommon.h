#pragma once

#include <cstdint>
#include <string>

#include "lua/lua.hpp"

#include "il2cpp-config.h"
#include "il2cpp-api-types.h"
#include "il2cpp-object-internals.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-tabledefs.h"
#include "gc/WriteBarrier.h"


#define ZLUA_DEBUG IL2CPP_DEBUG
#define ZLUA_ASSERT(cond) IL2CPP_ASSERT(cond)

#if IL2CPP_SIZEOF_VOID_P == 8
#define ZLUA_ARCH_64 1
#else
#define ZLUA_ARCH_32 1
#endif

/*
 * ZLUA_FAST_METATABLE — defined in lua/luaconf.h (default 0).
 * 0: legacy Dispatch* C __index/__newindex
 * 1: OPTIMIZATION.md scheme B (sealed mt + VM fast path)
 * Do not redefine here; keep Lua core and zlua runtime in sync (Table ABI).
 */
#ifndef ZLUA_FAST_METATABLE
#error "ZLUA_FAST_METATABLE must be defined by lua/luaconf.h"
#endif

namespace zlua
{



} // namespace zlua
