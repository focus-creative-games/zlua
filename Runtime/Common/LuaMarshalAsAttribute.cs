using System;

namespace ZLua
{
    [Flags]
    public enum LuaMarshalFlags
    {
        None = 0,
        OptionalField = 1,
    }

    public enum LuaMarshalType
    {
        Default,
        // Integer,
        // Number,
        UserData,
        Bytes,
        OpaqueLightUserData,
    }

    [AttributeUsage(AttributeTargets.Parameter | AttributeTargets.ReturnValue | AttributeTargets.Method | AttributeTargets.Field)]
    public sealed class LuaMarshalAsAttribute : Attribute
    {
        public LuaMarshalType LuaMarshalType { get; }

        public LuaMarshalFlags Flags { get; set; }

        public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default)
        {
            LuaMarshalType = luaMarshalType;
        }
    }
}
