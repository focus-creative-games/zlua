using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using ZLua.Meta;

namespace ZLua.CppCodeGen
{
    public class LuaMarshalMetaInfo
    {
        public TypeSig typeSig;
        public LuaMarshalAsInfo marshalAsInfo;
    }

    public class LuaMarshalAsInfo
    {
        public LuaMarshalType marshalType;
        public List<string> fieldOrProperties;
    }

    public static class MarshalMetaUtil
    {
        public static LuaMarshalAsInfo GetLuaMarshalAsInfo(IHasCustomAttribute provider)
        {
            if (provider == null)
            {
                return null;
            }
            if (!provider.HasCustomAttributes)
            {
                return null;
            }
            var attr = provider.CustomAttributes.FirstOrDefault(a => a.AttributeType.FullName == typeof(LuaMarshalAsAttribute).FullName);
            if (attr == null)
            {
                return null;
            }
            var info = new LuaMarshalAsInfo();
            if (attr.ConstructorArguments.Count == 1)
            {
                info.marshalType = (LuaMarshalType)(int)attr.ConstructorArguments[0].Value;
            }
            if (attr.NamedArguments.Count == 1)
            {
                info.fieldOrProperties = ((UTF8String[])attr.NamedArguments[0].Value)?.Select(s => s.ToString()).ToList();
            }
            return info;
        }

        

        public static TypeSig ToSharedGenericInstTypeSig(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            if (t.ElementType == ElementType.GenericInst)
            {
                return MetaUtil.ToSharedTypeSig(typeSig.Module.CorLibTypes, t);
            }
            return t;
        }

        public static LuaMarshalMetaInfo CreateMarshalMetaInfo(TypeSig typeSig, ParamDef paramDef, bool ignoreMarshalAs)
        {
            return new LuaMarshalMetaInfo
            {
                typeSig = ToSharedGenericInstTypeSig(typeSig),
                marshalAsInfo = ignoreMarshalAs ? null : GetLuaMarshalAsInfo(paramDef),
            };
        }

        public static ParamMarshalInfo CreateParamMarshalInfo(Parameter parameter, bool ignoreMarshalAs)
        {
            return new ParamMarshalInfo
            {
                indexExcludedThis = parameter.Index,
                name = parameter.Index >= 0 ? $"__p{parameter.Index}" : "__ret",
                marshalMetaInfo = CreateMarshalMetaInfo(parameter.Type, parameter.ParamDef, ignoreMarshalAs),
            };
        }
    }
}
