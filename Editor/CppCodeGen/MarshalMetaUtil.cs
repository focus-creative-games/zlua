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
        private static LuaMarshalAsInfo GetLuaMarshalAsInfoImpl(IHasCustomAttribute provider)
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

        private static readonly Dictionary<TypeDef, LuaMarshalAsInfo> _luaMarshalAsInfoCache = new Dictionary<TypeDef, LuaMarshalAsInfo>();


        private static LuaMarshalAsInfo GetLuaMarshalAsInfo(TypeDef typeDef)
        {
            if (_luaMarshalAsInfoCache.TryGetValue(typeDef, out var info))
            {
                return info;
            }
            info = GetLuaMarshalAsInfoImpl(typeDef);
            _luaMarshalAsInfoCache[typeDef] = info;
            return info;
        }

        public static LuaMarshalAsInfo GetLuaMarshalAsInfo(ParamDef paramDef, bool ignoreMarshalAsFromParam)
        {
            if (paramDef == null)
            {
                return null;
            }
            if (!ignoreMarshalAsFromParam)
            {
                LuaMarshalAsInfo info = GetLuaMarshalAsInfoImpl(paramDef);
                if (info != null)
                {
                    return info;
                }
            }
            return GetLuaMarshalAsInfo(paramDef.DeclaringMethod.DeclaringType);
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

        public static LuaMarshalMetaInfo CreateMarshalMetaInfo(TypeSig typeSig, ParamDef paramDef, bool ignoreMarshalAsFromParam)
        {
            return new LuaMarshalMetaInfo
            {
                typeSig = ToSharedGenericInstTypeSig(typeSig),
                marshalAsInfo = GetLuaMarshalAsInfo(paramDef, ignoreMarshalAsFromParam),
            };
        }

        public static ParamMarshalInfo CreateParamMarshalInfo(Parameter parameter, bool ignoreMarshalAsFromParam)
        {
            return new ParamMarshalInfo
            {
                indexExcludedThis = parameter.Index,
                name = parameter.Index >= 0 ? $"__p{parameter.Index}" : "__ret",
                marshalMetaInfo = CreateMarshalMetaInfo(parameter.Type, parameter.ParamDef, ignoreMarshalAsFromParam),
            };
        }
    }
}
