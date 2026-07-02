using System;

namespace ZLua
{
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
