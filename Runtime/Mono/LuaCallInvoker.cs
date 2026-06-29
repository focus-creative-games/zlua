using System;
using System.Reflection;

namespace ZLua
{
    internal static class LuaCallInvoker
    {
        internal static void InvokeVoid(LuaMethod method, Action<IntPtr> pushArgs)
        {
            InvokeCore(method, pushArgs, typeof(void));
        }

        internal static object Invoke(LuaMethod method, Action<IntPtr> pushArgs, Type returnType)
        {
            return InvokeCore(method, pushArgs, returnType);
        }

        internal static void InvokeVoidWithArgs(LuaMethod method, Type[] parameterTypes, object[] args)
        {
            InvokeWithArgs(method, null, typeof(void), parameterTypes, args);
        }

        internal static object InvokeWithArgs(LuaMethod method, Type returnType, Type[] parameterTypes, object[] args)
        {
            return InvokeWithArgs(method, null, returnType, parameterTypes, args);
        }

        internal static object InvokeWithArgs(
            LuaMethod method,
            MethodInfo invokeMethod,
            Type returnType,
            Type[] parameterTypes,
            object[] args)
        {
            return InvokeCore(
                method,
                luaState =>
                {
                    ParameterInfo[] parameters = invokeMethod?.GetParameters();
                    for (int i = 0; i < parameterTypes.Length; i++)
                    {
                        if (invokeMethod != null && parameters != null && i < parameters.Length)
                        {
                            LuaInvokeMarshaling.PushArgument(luaState, args[i], parameters, invokeMethod, i);
                        }
                        else
                        {
                            TypeMethodRegistration.PushArgumentValue(luaState, args[i], parameterTypes[i]);
                        }
                    }
                },
                invokeMethod,
                returnType);
        }

        private static object InvokeCore(LuaMethod method, Action<IntPtr> pushArgs, Type returnType)
        {
            return InvokeCore(method, pushArgs, null, returnType);
        }

        private static object InvokeCore(LuaMethod method, Action<IntPtr> pushArgs, MethodInfo invokeMethod, Type returnType)
        {
            if (method == null)
            {
                throw new ArgumentNullException(nameof(method));
            }

            IntPtr luaState = method.LuaState;
            if (luaState == IntPtr.Zero)
            {
                throw new ObjectDisposedException(nameof(LuaEnv));
            }

            int oldTop = LuaDll.lua_gettop(luaState);
            method.Env.PushErrorHandler(luaState);
            LuaPrintBuffer.EnterManagedPcall();
            try
            {
                try
                {
                    LuaDataType functionType = LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, method.RefIndex);
                    if (functionType != LuaDataType.Function)
                    {
                        throw new InvalidOperationException("Lua function reference is invalid.");
                    }

                    pushArgs?.Invoke(luaState);
                    int nArgs = LuaDll.lua_gettop(luaState) - oldTop - 2;
                    int nRet = returnType == typeof(void) ? 0 : 1;
                    int err = LuaDll.lua_pcall(luaState, nArgs, nRet, oldTop + 1);
                    if (err != 0)
                    {
                        throw new Exception(LuaDllExtension.tostring(luaState, -1));
                    }

                    if (returnType == typeof(void))
                    {
                        return null;
                    }

                    return LuaInvokeMarshaling.PopReturn(luaState, invokeMethod, returnType, -1);
                }
                finally
                {
                    LuaDll.lua_settop(luaState, oldTop);
                }
            }
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }
    }
}
