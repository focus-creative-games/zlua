// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
        public List<string> members;
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
                    if (argName == "Members")
                    {
                        // dnlib: string[] named-args are IList<CAArgument>, not UTF8String[].
                        info.members = ParseStringArrayArgument(named.Value);
                    }
                }
            }
            return info;
        }

        private static List<string> ParseStringArrayArgument(object value)
        {
            if (value == null)
            {
                return null;
            }

            if (value is UTF8String[] utf8Array)
            {
                return utf8Array.Select(s => s?.ToString() ?? string.Empty).ToList();
            }

            if (value is string[] stringArray)
            {
                return stringArray.ToList();
            }

            if (value is IList<CAArgument> caArgs)
            {
                var list = new List<string>(caArgs.Count);
                for (int i = 0; i < caArgs.Count; i++)
                {
                    list.Add(CaArgumentToString(caArgs[i]));
                }
                return list;
            }

            if (value is System.Collections.IEnumerable enumerable && !(value is string))
            {
                var list = new List<string>();
                foreach (object item in enumerable)
                {
                    if (item is CAArgument ca)
                    {
                        list.Add(CaArgumentToString(ca));
                    }
                    else if (item is UTF8String utf8)
                    {
                        list.Add(utf8.ToString());
                    }
                    else if (item != null)
                    {
                        list.Add(item.ToString());
                    }
                    else
                    {
                        list.Add(string.Empty);
                    }
                }
                return list;
            }

            return null;
        }

        private static string CaArgumentToString(CAArgument arg)
        {
            object v = arg.Value;
            if (v == null)
            {
                return string.Empty;
            }
            if (v is UTF8String utf8)
            {
                return utf8.ToString();
            }
            if (v is CAArgument nested)
            {
                return CaArgumentToString(nested);
            }
            return v.ToString();
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

        public static LuaMarshalAsInfo GetLuaMarshalAsInfoForType(TypeDef typeDef)
        {
            return GetLuaMarshalAsInfo(typeDef);
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
