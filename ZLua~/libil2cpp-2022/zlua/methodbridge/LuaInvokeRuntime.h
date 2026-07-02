#pragma once

#include "../marshal/Marshaling.h"
#include "../LuaEnv.h"

namespace zlua
{
    struct LuaInvokeSite
    {
        int moduleRef;
        int funcRef;
    };

    namespace LuaInvokeRuntime
    {
        template<typename Ret, typename... Args>
        IL2CPP_FORCE_INLINE Ret CallRet(const LuaInvokeSite& site, Args... args)
        {
            lua_State* L = LuaEnv::GetState();
            const int top = lua_gettop(L);
            const int errfunc = LuaEnv::PushErrorHandler(L);

            lua_rawgeti(L, LUA_REGISTRYINDEX, site.funcRef);
            (detail::PushDefault<Args>(L, args), ...);
            Marshaling::LuaPCall(L, (int)sizeof...(Args), 1, errfunc);

            Ret ret = detail::PopDefault<Ret>(L, -1);
            lua_settop(L, top);
            return ret;
        }

        template<typename... Args>
        IL2CPP_FORCE_INLINE void CallVoid(const LuaInvokeSite& site, Args... args)
        {
            lua_State* L = LuaEnv::GetState();
            const int top = lua_gettop(L);
            const int errfunc = LuaEnv::PushErrorHandler(L);

            lua_rawgeti(L, LUA_REGISTRYINDEX, site.funcRef);
            (detail::PushDefault<Args>(L, args), ...);
            Marshaling::LuaPCall(L, (int)sizeof...(Args), 0, errfunc);
            lua_settop(L, top);
        }
    } // namespace LuaInvokeRuntime
} // namespace zlua
