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

        /// <summary>
        /// Lua stack slots occupied by this parameter/return (UnpackedValues = Members.Length; else 1).
        /// </summary>
        public int StackSlots =>
            MarshalType == LuaMarshalType.UnpackedValues ? Members.Length : 1;

        public bool IsComposite =>
            MarshalType == LuaMarshalType.Table || MarshalType == LuaMarshalType.UnpackedValues;

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
