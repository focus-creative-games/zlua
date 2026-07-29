using System;

namespace ZLua
{
    public enum LuaMarshalType
    {
        Default,
        UserData,
        Bytes,
        OpaqueValue,
        UnpackedValues,
        Table,
    }

    [AttributeUsage(
        AttributeTargets.Parameter
        | AttributeTargets.ReturnValue
        | AttributeTargets.Field
        | AttributeTargets.Property
        | AttributeTargets.Class
        | AttributeTargets.Struct)]
    public sealed class LuaMarshalAsAttribute : Attribute
    {
        public LuaMarshalType LuaMarshalType { get; }

        /// <summary>
        /// Required for <see cref="LuaMarshalType.Table"/> / <see cref="LuaMarshalType.UnpackedValues"/>.
        /// Elements are CLR field or property names on the underlying struct (Nullable unwraps to T).
        /// Trailing '?' marks optional Table keys (Lua→C#). UnpackedValues does not support '?'.
        /// </summary>
        public string[] Members { get; set; }

        public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default)
        {
            LuaMarshalType = luaMarshalType;
        }
    }
}
