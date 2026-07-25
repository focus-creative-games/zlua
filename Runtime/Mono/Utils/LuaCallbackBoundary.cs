using System;
using System.Reflection;
using ZLua.Lvm;

namespace ZLua.Utils
{
    /// <summary>
    /// Lua C 回调边界：仅在此处将托管异常转为 <c>lua_error</c>。
    /// </summary>
    internal static class LuaCallbackBoundary
    {
        [ThreadStatic]
        private static int s_callbackDepth;

        internal static bool IsInsideCallback => s_callbackDepth > 0;

        internal static void Enter()
        {
            s_callbackDepth++;
        }

        internal static void Leave()
        {
            if (s_callbackDepth > 0)
            {
                s_callbackDepth--;
            }
        }

        /// <summary>
        /// Clear before <c>lua_error</c> longjmp (Mono may skip finally).
        /// </summary>
        internal static void ResetDepth()
        {
            s_callbackDepth = 0;
        }

        internal static int ToLuaError(IntPtr luaState, Exception ex)
        {
            ResetDepth();
            return LuaDllExtension.error(luaState, FormatMessage(ex));
        }

        internal static string FormatMessage(Exception ex)
        {
            ex = Unwrap(ex);
            return ex.Message;
        }

        internal static Exception Unwrap(Exception ex)
        {
            while (ex is TargetInvocationException tie && tie.InnerException != null)
            {
                ex = tie.InnerException;
            }

            return ex;
        }

        /// <summary>
        /// Signal a Lua-facing error. Under Lua→C# reverse P/Invoke, must not
        /// <c>throw</c> (Tuanjie Mono SIGSEGVs in exception first-pass while an outer
        /// <c>lua_pcall</c> is active); use <c>lua_error</c> instead.
        /// </summary>
        internal static void Throw(string message)
        {
            if (IsInsideCallback)
            {
                LuaEnv env = LuaEnv.Active;
                if (env != null)
                {
                    IntPtr L = env.L;
                    ResetDepth();
                    LuaDllExtension.error(L, message);
                }
            }

            throw new LuaScriptException(message);
        }
    }
}
