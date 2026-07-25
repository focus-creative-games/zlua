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

    public sealed class ParamMarshalInfo
    {
        public int indexExcludedThis; // start from 0
        public string name;
        public LuaMarshalMetaInfo marshalMetaInfo;
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
            if (attr.NamedArguments.Count >= 1)
            {
                foreach (var named in attr.NamedArguments)
                {
                    string argName = named.Name;
                    if (argName == "Members" || argName == "FieldOrPropertyNames")
                    {
                        info.fieldOrProperties = ((UTF8String[])named.Value)?.Select(s => s.ToString()).ToList();
                    }
                }
            }
            return info;
        }

        private static readonly Dictionary<TypeDef, LuaMarshalAsInfo> _luaMarshalAsInfoCache = new Dictionary<TypeDef, LuaMarshalAsInfo>();


        private static LuaMarshalAsInfo GetLuaMarshalAsInfo(TypeDef typeDef)
        {
            if (typeDef == null)
            {
                return null;
            }

            if (_luaMarshalAsInfoCache.TryGetValue(typeDef, out var info))
            {
                return info;
            }

            // Type-level MarshalAs is illegal on generic type definitions (spec §1.1).
            if (typeDef.GenericParameters.Count > 0)
            {
                _luaMarshalAsInfoCache[typeDef] = null;
                return null;
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

            TypeDef declaringType = paramDef.DeclaringMethod?.DeclaringType;
            if (declaringType == null || declaringType.GenericParameters.Count > 0)
            {
                return null;
            }

            return GetLuaMarshalAsInfo(declaringType);
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
