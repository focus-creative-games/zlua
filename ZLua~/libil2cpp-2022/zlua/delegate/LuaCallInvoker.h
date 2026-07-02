#pragma once

#include "../LuaEnv.h"
#include "../marshal/Marshaling.h"
#include "../marshal/PrimitiveMarshal.h"

#include "lua/lauxlib.h"

#include <functional>

namespace zlua
{
    namespace LuaCallInvoker
    {
        inline bool CallVoid(int funcRef, const std::function<void(lua_State*)>& pushArgs, int argCount)
        {
            lua_State* L = LuaEnv::GetState();
            const int restoreTop = lua_gettop(L);
            const int errfunc = LuaEnv::PushErrorHandler(L);

            lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
            if (!lua_isfunction(L, -1))
            {
                lua_settop(L, restoreTop);
                return false;
            }

            if (pushArgs)
                pushArgs(L);

            const int callResult = lua_pcall(L, argCount, 0, errfunc);
            lua_settop(L, restoreTop);
            return callResult == LUA_OK;
        }

        inline bool CallRetInt32(int funcRef, int32_t arg0, int32_t* outRet)
        {
            if (outRet == nullptr)
                return false;

            lua_State* L = LuaEnv::GetState();
            const int restoreTop = lua_gettop(L);
            const int errfunc = LuaEnv::PushErrorHandler(L);

            lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
            if (!lua_isfunction(L, -1))
            {
                lua_settop(L, restoreTop);
                return false;
            }

            lua_pushinteger(L, arg0);
            const int callResult = lua_pcall(L, 1, 1, errfunc);
            if (callResult != LUA_OK)
            {
                lua_settop(L, restoreTop);
                return false;
            }

            if (!lua_isinteger(L, -1))
            {
                lua_settop(L, restoreTop);
                return false;
            }

            *outRet = (int32_t)lua_tointeger(L, -1);
            lua_settop(L, restoreTop);
            return true;
        }
    } // namespace LuaCallInvoker
} // namespace zlua
