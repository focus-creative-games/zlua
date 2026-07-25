using System;

namespace ZLua
{
    public enum LuaMarshalAsXmlTargetKind : byte
    {
        Type = 0,
        Field = 1,
        Property = 2,
        Param = 3,
        Return = 4,
    }

    /// <summary>
    /// One MarshalAs rule from external XML (spec marshal/02-MARSHAL-AS §9).
    /// </summary>
    public sealed class LuaMarshalAsXmlRule
    {
        public string SourcePath { get; set; }

        public string AssemblyName { get; set; }

        public string TypeFullName { get; set; }

        public LuaMarshalAsXmlTargetKind Kind { get; set; }

        public string MemberName { get; set; }

        public string MethodName { get; set; }

        public string Signature { get; set; }

        public int ParamIndex { get; set; } = -1;

        public LuaMarshalType MarshalType { get; set; }

        public string[] Members { get; set; }

        public string DuplicateKey
        {
            get
            {
                switch (Kind)
                {
                    case LuaMarshalAsXmlTargetKind.Type:
                        return AssemblyName + "|" + TypeFullName + "|Type";
                    case LuaMarshalAsXmlTargetKind.Field:
                        return AssemblyName + "|" + TypeFullName + "|Field|" + MemberName;
                    case LuaMarshalAsXmlTargetKind.Property:
                        return AssemblyName + "|" + TypeFullName + "|Property|" + MemberName;
                    case LuaMarshalAsXmlTargetKind.Param:
                        return AssemblyName + "|" + TypeFullName + "|" + MethodName + "|" + Signature + "|Param|" + ParamIndex;
                    case LuaMarshalAsXmlTargetKind.Return:
                        return AssemblyName + "|" + TypeFullName + "|" + MethodName + "|" + Signature + "|Return";
                    default:
                        throw new InvalidOperationException("Unknown MarshalAs XML target kind: " + Kind);
                }
            }
        }

        public LuaMarshalAsAttribute ToAttribute()
        {
            return new LuaMarshalAsAttribute(MarshalType)
            {
                Members = Members,
            };
        }
    }
}
