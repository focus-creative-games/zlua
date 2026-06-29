using System;

namespace ZLua
{
    /// <summary>
    /// 预期内的 Lua 脚本互操作错误。由 Lua 回调第 1 层捕获并转为 <c>lua_error</c>。
    /// </summary>
    public sealed class LuaScriptException : Exception
    {
        public LuaScriptException(string message)
            : base(message)
        {
        }

        public LuaScriptException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
