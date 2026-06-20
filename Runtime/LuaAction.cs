using System;

namespace NextLua
{
    public readonly struct LuaAction
    {
        private readonly LuaMethod _method;

        public LuaAction(LuaMethod method)
        {
            _method = method;
        }

        public void PCall(Action<IntPtr> pushArg)
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
                int err = LuaDll.lua_pcall(luaState, nArgs, 0, oldTop + 1);
                if (err != 0)
                {
                    throw new Exception(LuaDllExtension.tostring(luaState, -1));
                }
            }
            finally
            {
                LuaDll.lua_settop(luaState, oldTop);
            }
        }

        public void Invoke()
        {
            PCall(null);
        }

        public void Invoke<A>(A a)
        {
            PCall(l => LuaMarshal.PushAny(l, a));
        }

        public void Invoke<A, B>(A a, B b)
        {
            PCall(l =>
            {
                LuaMarshal.PushAny(l, a);
                LuaMarshal.PushAny(l, b);
            });
        }
    }
}
