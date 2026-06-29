using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Lua C 回调边界：仅在此处（第 1 层）将异常转为 <c>lua_error</c>。
    /// </summary>
    internal static class LuaCallbackBoundary
    {
        internal static int ToLuaError(IntPtr luaState, Exception ex)
        {
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

        internal static void Throw(string message)
        {
            throw new LuaScriptException(message);
        }
    }
}
