using System.Reflection;

namespace ZLua
{
    public enum LuaMarshalDirection
    {
        LuaToCSharp,
        CSharpToLua,
    }

    /// <summary>
    /// Resolved marshal metadata for one parameter or return value (MARSHAL_SPEC §6.4).
    /// </summary>
    public sealed class LuaMarshalBinding
    {
        public LuaMarshalType MarshalType { get; }

        public LuaMarshalMemberBinding[] Members { get; }

        public LuaMarshalBinding(LuaMarshalType marshalType, LuaMarshalMemberBinding[] members = null)
        {
            MarshalType = marshalType;
            Members = members ?? System.Array.Empty<LuaMarshalMemberBinding>();
        }

        public static LuaMarshalBinding Default { get; } = new LuaMarshalBinding(LuaMarshalType.Default);
    }

    public sealed class LuaMarshalMemberBinding
    {
        public string ClrName { get; }

        public bool IsOptional { get; }

        public MemberInfo Member { get; }

        public LuaMarshalMemberBinding(string clrName, bool isOptional, MemberInfo member)
        {
            ClrName = clrName;
            IsOptional = isOptional;
            Member = member;
        }
    }
}
