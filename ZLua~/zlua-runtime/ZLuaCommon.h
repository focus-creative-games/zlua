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
