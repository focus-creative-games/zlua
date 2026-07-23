using System;

namespace ZLua.Bridge
{
    /// <summary>
    /// Nested LuaInvoke errors cannot <c>throw</c> on Unity Mono (stack capture during
    /// an outer <c>lua_pcall</c> SIGSEGVs). Stash the message for the Lua C callback Wrap to
    /// convert via <c>lua_error</c>.
    /// </summary>
    internal static class LuaInvokePendingError
    {
        [ThreadStatic]
        private static string s_message;

        internal static void Set(string message)
        {
            s_message = string.IsNullOrEmpty(message) ? "lua pcall failed" : message;
        }

        internal static bool HasPending => s_message != null;

        internal static bool TryTake(out string message)
        {
            message = s_message;
            s_message = null;
            return message != null;
        }
    }
}
