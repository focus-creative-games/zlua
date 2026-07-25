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
        ParamsTable,
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
        /// Elements are CLR field or property names (may mix). Trailing '?' marks optional Table keys (Lua→C#).
        /// </summary>
        public string[] Members { get; set; }

        public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default)
        {
            LuaMarshalType = luaMarshalType;
        }
    }
}
