#include "PrimitiveMarshal.h"

#include "../utils/LuaException.h"

namespace zlua
{
namespace
{
inline void ExpectLuaType(lua_State* L, int idx, int expectedType, const char* expectedName)
{
    if (lua_type(L, idx) != expectedType)
        LuaException::ThrowFormat("zlua argument mismatch: expected %s at index %d", expectedName, idx);
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
    return (int8_t)lua_tointeger(L, idx);
}

uint8_t PrimitiveMarshal::PopUInt8(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (uint8_t)lua_tointeger(L, idx);
}

int16_t PrimitiveMarshal::PopInt16(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (int16_t)lua_tointeger(L, idx);
}

uint16_t PrimitiveMarshal::PopUInt16(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (uint16_t)lua_tointeger(L, idx);
}

int32_t PrimitiveMarshal::PopInt32(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (int32_t)lua_tointeger(L, idx);
}

uint32_t PrimitiveMarshal::PopUInt32(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (uint32_t)lua_tointeger(L, idx);
}

int64_t PrimitiveMarshal::PopInt64(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (int64_t)lua_tointeger(L, idx);
}

uint64_t PrimitiveMarshal::PopUInt64(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    if (lua_isinteger(L, idx))
        return (uint64_t)lua_tointeger(L, idx);
    return (uint64_t)lua_tonumber(L, idx);
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
    return (intptr_t)lua_tointeger(L, idx);
}

uintptr_t PrimitiveMarshal::PopUIntPtr(lua_State* L, int idx)
{
    ExpectLuaType(L, idx, LUA_TNUMBER, "number");
    return (uintptr_t)lua_tointeger(L, idx);
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
