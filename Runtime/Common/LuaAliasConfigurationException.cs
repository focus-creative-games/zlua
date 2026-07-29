using System;

namespace ZLua
{
    /// <summary>
    /// Configuration errors for LuaAlias XML (spec 04-METHOD-OVERLOAD §5.4).
    /// </summary>
    public sealed class LuaAliasConfigurationException : Exception
    {
        public LuaAliasConfigurationException(string message)
            : base(message)
        {
        }

        public LuaAliasConfigurationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
