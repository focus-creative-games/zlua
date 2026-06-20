using System;

namespace NextLua
{
    public readonly struct LuaFunc
    {
        private readonly LuaMethod _method;

        public LuaFunc(LuaMethod method)
        {
            _method = method;
        }

        public R PCall<R>(Action<IntPtr> pushArg, Func<IntPtr, R> popRet)
        {
            IntPtr luaState = _method.LuaState;
            if (luaState == IntPtr.Zero)
            {
                throw new ObjectDisposedException(nameof(LuaEnv));
            }

            int oldTop = LuaDll.lua_gettop(luaState);
            LuaDll.lua_getglobal(luaState, "__nextluaErrorHandler");
            try
            {
                LuaDataType type = LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, _method.RefIndex);
                if (type != LuaDataType.Function)
                {
                    throw new InvalidOperationException("Lua function reference is invalid.");
                }

                pushArg?.Invoke(luaState);
                int nArgs = LuaDll.lua_gettop(luaState) - oldTop - 2;
                int err = LuaDll.lua_pcall(luaState, nArgs, 1, oldTop + 1);
                if (err != 0)
                {
                    throw new Exception(LuaDllExtension.tostring(luaState, -1));
                }

                return popRet(luaState);
            }
            finally
            {
                LuaDll.lua_settop(luaState, oldTop);
            }
        }

        public R Invoke<R>()
        {
            return PCall(_ => { }, LuaMarshal.PopAny<R>);
        }

        public R Invoke<A, R>(A a)
        {
            return PCall(
                l => LuaMarshal.PushAny(l, a),
                LuaMarshal.PopAny<R>);
        }

        public R Invoke<A, B, R>(A a, B b)
        {
            return PCall(
                l =>
                {
                    LuaMarshal.PushAny(l, a);
                    LuaMarshal.PushAny(l, b);
                },
                LuaMarshal.PopAny<R>);
        }
    }
}
