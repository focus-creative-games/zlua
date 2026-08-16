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

#include "../ZLuaCommon.h"

struct Il2CppType;

namespace zlua
{
class PrimitiveMarshal
{
  public:
    static void PushBool(lua_State* L, bool v)
    {
        lua_pushboolean(L, v ? 1 : 0);
    }

    static bool PopBool(lua_State* L, int idx);

    static void PushInt8(lua_State* L, int8_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static int8_t PopInt8(lua_State* L, int idx);

    static void PushUInt8(lua_State* L, uint8_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static uint8_t PopUInt8(lua_State* L, int idx);

    static void PushInt16(lua_State* L, int16_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static int16_t PopInt16(lua_State* L, int idx);

    static void PushUInt16(lua_State* L, uint16_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static uint16_t PopUInt16(lua_State* L, int idx);

    static void PushInt32(lua_State* L, int32_t v)
    {
        lua_pushinteger(L, v);
    }

    static int32_t PopInt32(lua_State* L, int idx);

    static void PushInt64(lua_State* L, int64_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static int64_t PopInt64(lua_State* L, int idx);

    static void PushUInt32(lua_State* L, uint32_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static uint32_t PopUInt32(lua_State* L, int idx);

    static void PushUInt64(lua_State* L, uint64_t v)
    {
        if (v <= (uint64_t)LUA_MAXINTEGER)
            lua_pushinteger(L, (lua_Integer)v);
        else
            lua_pushnumber(L, (lua_Number)v);
    }

    static uint64_t PopUInt64(lua_State* L, int idx);

    static void PushFloat(lua_State* L, float v)
    {
        lua_pushnumber(L, v);
    }

    static float PopFloat(lua_State* L, int idx);

    static void PushDouble(lua_State* L, double v)
    {
        lua_pushnumber(L, v);
    }

    static double PopDouble(lua_State* L, int idx);

    static void PushIntPtr(lua_State* L, intptr_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static intptr_t PopIntPtr(lua_State* L, int idx);

    static void PushUIntPtr(lua_State* L, uintptr_t v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    static uintptr_t PopUIntPtr(lua_State* L, int idx);

    static void PushPointer(lua_State* L, void* v)
    {
        lua_pushlightuserdata(L, v);
    }

    static void* PopPointer(lua_State* L, int idx);

    // static bool CanConvert(lua_State* L, int index, const Il2CppType* type);
    // static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize);
    // static int PushBoxedReturn(lua_State* L, const Il2CppType* returnType, void* boxedData);
};
} // namespace zlua
