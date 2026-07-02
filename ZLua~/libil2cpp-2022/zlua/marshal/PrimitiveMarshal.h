#pragma once

#include <cstdint>

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

        static bool PopBool(lua_State* L, int idx)
        {
            return lua_toboolean(L, idx) != 0;
        }

        static void PushInt32(lua_State* L, int32_t v)
        {
            lua_pushinteger(L, v);
        }

        static int32_t PopInt32(lua_State* L, int idx)
        {
            return (int32_t)lua_tointeger(L, idx);
        }

        static void PushInt64(lua_State* L, int64_t v)
        {
            lua_pushinteger(L, (lua_Integer)v);
        }

        static int64_t PopInt64(lua_State* L, int idx)
        {
            return (int64_t)lua_tointeger(L, idx);
        }

        static void PushUInt32(lua_State* L, uint32_t v)
        {
            lua_pushinteger(L, (lua_Integer)v);
        }

        static uint32_t PopUInt32(lua_State* L, int idx)
        {
            return (uint32_t)lua_tointeger(L, idx);
        }

        static void PushUInt64(lua_State* L, uint64_t v)
        {
            if (v <= (uint64_t)LUA_MAXINTEGER)
                lua_pushinteger(L, (lua_Integer)v);
            else
                lua_pushnumber(L, (lua_Number)v);
        }

        static uint64_t PopUInt64(lua_State* L, int idx)
        {
            if (lua_isinteger(L, idx))
                return (uint64_t)lua_tointeger(L, idx);
            return (uint64_t)lua_tonumber(L, idx);
        }

        static void PushFloat(lua_State* L, float v)
        {
            lua_pushnumber(L, v);
        }

        static float PopFloat(lua_State* L, int idx)
        {
            return (float)lua_tonumber(L, idx);
        }

        static void PushDouble(lua_State* L, double v)
        {
            lua_pushnumber(L, v);
        }

        static double PopDouble(lua_State* L, int idx)
        {
            return lua_tonumber(L, idx);
        }

        static bool CanConvert(lua_State* L, int index, const Il2CppType* type);
        static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize);
        static int PushBoxedReturn(lua_State* L, const Il2CppType* returnType, void* boxedData);
    };
}
