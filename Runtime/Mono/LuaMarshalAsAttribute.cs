using System;

namespace NovaLua
{
    public enum LuaMarshalType
    {
        Default,
        Integer,
        Number,
        CString,
        UserData,
        LightUserData,
    }

    [AttributeUsage(AttributeTargets.Parameter | AttributeTargets.ReturnValue | AttributeTargets.Method)]
    public sealed class LuaMarshalAsAttribute : Attribute
    {
        public LuaMarshalType LuaMarshalType { get; }

        public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default)
        {
            LuaMarshalType = luaMarshalType;
        }
    }
}
