using System;

namespace ZLua.Emit
{
    /// <summary>
    /// Bind-time failure when a member cannot be Expression-emitted (REWRITE D3).
    /// </summary>
    public sealed class EmitException : Exception
    {
        public EmitException(string message)
            : base(message)
        {
        }

        public EmitException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        public static EmitException ForMember(Type type, string memberName, string reason)
        {
            string typeName = type?.FullName ?? "?";
            return new EmitException($"zlua: cannot emit {typeName}.{memberName}: {reason}");
        }
    }
}
