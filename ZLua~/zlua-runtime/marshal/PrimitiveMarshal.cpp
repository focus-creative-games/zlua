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

#include "PrimitiveMarshal.h"

#include "../utils/LuaException.h"

#include <cstdint>

namespace zlua
{
namespace
{
inline void ExpectLuaType(lua_State* L, int idx, int expectedType, const char* expectedName)
{
#if ZLUA_USE_LUAU
    if (expectedType == LUA_TNUMBER)
    {
        const int t = lua_type(L, idx);
        if (t == LUA_TNUMBER || t == LUA_TINTEGER)
            return;
    }
#endif
    if (ZLUA_UNLIKELY(lua_type(L, idx) != expectedType))
        LuaException::ThrowFormat("zlua argument mismatch: expected %s at index %d", expectedName, idx);
}

NORETURN IL2CPP_NO_INLINE static void ThrowIntegerOutOfRange(
    int idx, int64_t value, int64_t minV, int64_t maxV, const char* typeName)
{
    LuaException::ThrowFormat(
        "zlua argument mismatch: integer %lld out of %s range [%lld, %lld] at index %d",
        static_cast<long long>(value),
        typeName,
        static_cast<long long>(minV),
        static_cast<long long>(maxV),
        idx);
}
} // namespace

bool PrimitiveMarshal::PopBool(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TBOOLEAN, "boolean");
    return lua_toboolean(L, idx) != 0;
}

int8_t PrimitiveMarshal::PopInt8(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < INT8_MIN || value > INT8_MAX))
        ThrowIntegerOutOfRange(idx, value, INT8_MIN, INT8_MAX, "System.SByte");
    return static_cast<int8_t>(value);
}

uint8_t PrimitiveMarshal::PopUInt8(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < 0 || value > static_cast<int64_t>(UINT8_MAX)))
        ThrowIntegerOutOfRange(idx, value, 0, UINT8_MAX, "System.Byte");
    return static_cast<uint8_t>(value);
}

int16_t PrimitiveMarshal::PopInt16(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < INT16_MIN || value > INT16_MAX))
        ThrowIntegerOutOfRange(idx, value, INT16_MIN, INT16_MAX, "System.Int16");
    return static_cast<int16_t>(value);
}

uint16_t PrimitiveMarshal::PopUInt16(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < 0 || value > static_cast<int64_t>(UINT16_MAX)))
        ThrowIntegerOutOfRange(idx, value, 0, UINT16_MAX, "System.UInt16");
    return static_cast<uint16_t>(value);
}

int32_t PrimitiveMarshal::PopInt32(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < INT32_MIN || value > INT32_MAX))
        ThrowIntegerOutOfRange(idx, value, INT32_MIN, INT32_MAX, "System.Int32");
    return static_cast<int32_t>(value);
}

uint32_t PrimitiveMarshal::PopUInt32(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    const int64_t value = zlua_to_int64(L, idx);
    if (ZLUA_UNLIKELY(value < 0 || value > static_cast<int64_t>(UINT32_MAX)))
        ThrowIntegerOutOfRange(idx, value, 0, static_cast<int64_t>(UINT32_MAX), "System.UInt32");
    return static_cast<uint32_t>(value);
}

int64_t PrimitiveMarshal::PopInt64(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return zlua_to_int64(L, idx);
}

uint64_t PrimitiveMarshal::PopUInt64(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    if (lua_isinteger(L, idx))
    {
        const int64_t signedValue = zlua_to_int64(L, idx);
        if (ZLUA_UNLIKELY(signedValue < 0))
            ThrowIntegerOutOfRange(idx, signedValue, 0, INT64_MAX, "System.UInt64");
        return static_cast<uint64_t>(signedValue);
    }
    const lua_Number number = lua_tonumber(L, idx);
    if (ZLUA_UNLIKELY(number < 0))
        ThrowIntegerOutOfRange(idx, static_cast<int64_t>(number), 0, INT64_MAX, "System.UInt64");
    return static_cast<uint64_t>(number);
}

float PrimitiveMarshal::PopFloat(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (float)lua_tonumber(L, idx);
}

double PrimitiveMarshal::PopDouble(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return lua_tonumber(L, idx);
}

intptr_t PrimitiveMarshal::PopIntPtr(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (intptr_t)zlua_to_int64(L, idx);
}

uintptr_t PrimitiveMarshal::PopUIntPtr(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (uintptr_t)zlua_to_uint64(L, idx);
}

void* PrimitiveMarshal::PopPointer(lua_State* L, int idx)
{
    int type = lua_type(L, idx);
    if (type == LUA_TLIGHTUSERDATA)
    {
        return lua_touserdata(L, idx);
    }
    else if (type == LUA_TNIL)
    {
        return nullptr;
    }
    else
    {
        LuaException::ThrowFormat("zlua: expected userdata or nil at index %d", idx);
        return nullptr;
    }
}
} // namespace zlua
