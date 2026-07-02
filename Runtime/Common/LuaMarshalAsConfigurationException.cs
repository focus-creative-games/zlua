using System;

namespace ZLua
{
    /// <summary>
    /// Raised when a <see cref="LuaMarshalAsAttribute"/> configuration violates MARSHAL_SPEC §6.3.2 (bind-time failure).
    /// </summary>
    public sealed class LuaMarshalAsConfigurationException : Exception
    {
        public LuaMarshalAsConfigurationException(string message)
            : base(message)
        {
        }
    }
}
