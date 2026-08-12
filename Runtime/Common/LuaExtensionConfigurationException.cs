using System;

namespace ZLua
{
    /// <summary>
    /// Configuration errors for LuaExtension XML / Attribute (spec 13-EXTENSION-METHODS).
    /// </summary>
    public sealed class LuaExtensionConfigurationException : Exception
    {
        public LuaExtensionConfigurationException(string message)
            : base(message)
        {
        }

        public LuaExtensionConfigurationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
