using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using ZLua.CppCodeGen;
using ZLua.Meta;

namespace ZLua.Utils
{
    public static class NameUtil
    {

        public static string EnscapeAsValidCppSymbol(string s)
        {
            StringBuilder r = new StringBuilder();
            foreach (char c in s)
            {
                if (char.IsLetter(c) || char.IsDigit(c) || c == '_')
                {
                    r.Append(c);
                }
                else if (c == '.' || c == ':' || c == '-' || c == '<' || c == '>' || c == '+' || c == '/')
                {
                    r.Append('_');
                }
                else
                {
                    r.Append($"_{(int)c:X2}");
                }
            }

            return r.ToString();
        }

        public static string CreateUniqueName(MethodDef method, List<TypeSig> klassInst, List<TypeSig> methodInst)
        {
            string baseName = $"{method.DeclaringType.FullName}_{method.Name}";

            StringBuilder sb = new StringBuilder();
            sb.Append(method.FullName);
            if (klassInst != null)
            {
                sb.Append("<<");
                foreach (var type in klassInst)
                {
                    sb.Append(type.FullName);
                    sb.Append(',');
                }
                sb.Append(">>");
            }
            if (methodInst != null)
            {
                sb.Append('<');
                foreach (var type in methodInst)
                {
                    sb.Append(type.FullName);
                    sb.Append(',');
                }
                sb.Append('>');
            }
            return EnscapeAsValidCppSymbol(
                $"{baseName}_{ToMd5Upper(sb.ToString())}");
        }

        public static string CreateUniqueName(PropertyDef property)
        {
            string baseName = $"{property.DeclaringType.FullName}_{property.Name}";
            return EnscapeAsValidCppSymbol($"{baseName}_{ToMd5Upper(property.FullName)}");
        }

        public static string CreateUniqueName(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            StringBuilder sb = new StringBuilder();
            string baseName;

            switch (t.ElementType)
            {
            case ElementType.Class:
            case ElementType.ValueType:
            {
                var typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
                baseName = typeDef.Name;
                sb.Append('[').Append(typeDef.Module.Assembly.Name).Append(']').Append(typeDef.FullName);
                break;
            }
            case ElementType.GenericInst:
            {
                var gis = t.ToGenericInstSig();
                TypeDef genericType = gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                baseName = genericType.Name;
                sb.Append('[').Append(genericType.Module.Assembly.Name).Append(']').Append(genericType.FullName);
                foreach (var type in gis.GenericArguments)
                {
                    sb.Append(GetTypeName(type));
                }
                break;
            }
            default: throw new CodeGenException($"Unsupported type: {t.ElementType}");
            }
            return EnscapeAsValidCppSymbol($"{baseName}_{ToMd5Upper(sb.ToString())}");
        }

        public static string CreateInternalCallSignature(MethodDef method)
        {
            var sb = new StringBuilder();
            sb.Append(GetFullNameWithPlusAsSeparator(method.DeclaringType.ToTypeSig()));
            sb.Append("::");
            sb.Append(method.Name);
            sb.Append('(');
            bool first = true;
            foreach (var param in method.Parameters)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    sb.Append(',');
                }
                sb.Append(GetFullNameWithPlusAsSeparator(param.Type));
            }
            sb.Append(')');
            return sb.ToString();
        }

        public static string CreateDelegateInvokeSignature(MethodDef method, List<TypeSig> klassInst, List<TypeSig> methodInst)
        {

            MethodDesc methodDesc = MethodDesc.CreateMethodDesc(method, klassInst, methodInst, false);
            var sb = new StringBuilder();
            sb.Append(GetFullNameWithPlusAsSeparator(methodDesc.ReturnInfo.type));
            sb.Append('(');
            int index = 0;
            foreach (var param in methodDesc.ParamInfos)
            {
                if (index > 0)
                {
                    sb.Append(',');
                }
                sb.Append(GetFullNameWithPlusAsSeparator(param.type));
                ++index;
            }
            sb.Append(')');
            return sb.ToString();
        }

        static string ToMd5Upper(string input)
        {
            byte[] bytes = Encoding.UTF8.GetBytes(input);
            byte[] hash = MD5.Create().ComputeHash(bytes);
            var sb = new StringBuilder(hash.Length * 2);
            foreach (byte b in hash)
            {
                sb.Append(b.ToString("X2"));
            }
            return sb.ToString();
        }

        private static Dictionary<TypeSig, string> _cachedCppStructTypeNames = new Dictionary<TypeSig, string>();

        private static string GetCachedStructTypeName(TypeSig typeSig)
        {
            if (_cachedCppStructTypeNames.TryGetValue(typeSig, out string name))
            {
                return name;
            }

            name = CreateUniqueName(typeSig);
            _cachedCppStructTypeNames.Add(typeSig, name);
            return name;
        }

        public static string GetTypeName(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            if (t.IsByRef)
            {
                return "void*";
            }
            switch (t.ElementType)
            {
            case ElementType.Void:
                return "void";
            case ElementType.Boolean:
                return "bool";
            case ElementType.Char:
                return "uint16_t";
            case ElementType.I1:
                return "int8_t";
            case ElementType.U1:
                return "uint8_t";
            case ElementType.I2:
                return "int16_t";
            case ElementType.U2:
                return "uint16_t";
            case ElementType.I4:
                return "int32_t";
            case ElementType.U4:
                return "uint32_t";
            case ElementType.I8:
                return "int64_t";
            case ElementType.U8:
                return "uint64_t";
            case ElementType.R4:
                return "float";
            case ElementType.R8:
                return "double";
            case ElementType.I:
                return "intptr_t";
            case ElementType.U:
                return "uintptr_t";
            case ElementType.TypedByRef:
                return "Il2CppTypedRef";
            case ElementType.Object:
                return "Il2CppObject*";
            case ElementType.String:
                return "Il2CppString*";
            case ElementType.SZArray:
            case ElementType.Array:
                return $"Il2CppArray*";
            case ElementType.Ptr:
                return "void*";
            case ElementType.FnPtr:
                return "void*";
            case ElementType.Class:
                return "Il2CppObject*";
            case ElementType.ValueType:
            {
                TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetTypeName(typeDef.GetEnumUnderlyingType());
                }
                return GetCachedStructTypeName(t);
            }
            case ElementType.GenericInst:
            {
                GenericInstSig gis = t.ToGenericInstSig();
                TypeDef genericType = gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (genericType.IsValueType)
                {
                    if (genericType.IsEnum)
                    {
                        return GetTypeName(genericType.GetEnumUnderlyingType());
                    }
                    return GetCachedStructTypeName(t);
                }
                else
                {
                    return $"Il2CppObject*";
                }
            }
            default:
                throw new NotImplementedException(t.ElementType.ToString());
            }
        }


        private static string GetCppReducedClassMarshalTypeName(TypeSig typeSig)
        {
            if (MetaUtil.IsSubOfMulticastDelegate(typeSig))
            {
                return "Il2CppDelegate*";
            }
            return "Il2CppObject*";
        }

        public static string CreateCppReducedMarshalTypeName(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            if (t.IsByRef)
            {
                return "void*";
            }
            switch (t.ElementType)
            {
            case ElementType.Void:
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.I:
            case ElementType.U:
            case ElementType.TypedByRef:
            case ElementType.ValueType:
            case ElementType.Ptr:
            case ElementType.FnPtr:
            case ElementType.Object:
            case ElementType.String:
            case ElementType.SZArray:
            case ElementType.Array:
                return GetTypeName(t);
            case ElementType.Class:
                return GetCppReducedClassMarshalTypeName(t);
            case ElementType.GenericInst:
            {
                GenericInstSig gis = t.ToGenericInstSig();
                TypeDef genericType = gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (genericType.IsValueType)
                {
                    return GetTypeName(t);
                }
                else
                {
                    return GetCppReducedClassMarshalTypeName(t);
                }
            }
            default:
                throw new NotImplementedException(t.ElementType.ToString());
            }
        }

        private static string GetShortReducedClassMarshalTypeName(TypeSig typeSig)
        {
            if (MetaUtil.IsSubOfMulticastDelegate(typeSig))
            {
                return "D";
            }
            return "O";
        }


        private static string GetFullNameWithPlusAsSeparator(TypeSig typeSig)
        {
            return typeSig.FullName.Replace('/', '+');
        }

        private static string CreateShortReducedStructMarshalTypeName(TypeDef typeDef)
        {
            return $"${typeDef.FullName.Replace('/', '+')}";
        }

        public static string CreateShortReducedMarshalTypeName(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            if (t.IsByRef)
            {
                return "&R";
            }
            switch (t.ElementType)
            {
            case ElementType.Void: return "V";
            case ElementType.Boolean: return "B";
            case ElementType.Char: return "U2";
            case ElementType.I1: return "I1";
            case ElementType.U1: return "U1";
            case ElementType.I2: return "I2";
            case ElementType.U2: return "U2";
            case ElementType.I4: return "I4";
            case ElementType.U4: return "U4";
            case ElementType.I8: return "I8";
            case ElementType.U8: return "U8";
            case ElementType.R4: return "R4";
            case ElementType.R8: return "R8";
            case ElementType.I: return "I";
            case ElementType.U: return "U";
            case ElementType.TypedByRef: return "TypedByRef";
            case ElementType.Ptr:
            case ElementType.FnPtr: return "P";
            case ElementType.Object: return "O";
            case ElementType.String: return "S";
            case ElementType.SZArray:
            case ElementType.Array: return "A";
            case ElementType.Class:
                return GetShortReducedClassMarshalTypeName(t);
            case ElementType.ValueType:
            {
                TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return CreateShortReducedMarshalTypeName(typeDef.GetEnumUnderlyingType());
                }
                return CreateShortReducedStructMarshalTypeName(typeDef);
            }
            case ElementType.GenericInst:
            {
                GenericInstSig gis = t.ToGenericInstSig();
                TypeDef genericType = gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (genericType.IsValueType)
                {
                    if (genericType.IsEnum)
                    {
                        return CreateShortReducedMarshalTypeName(genericType.GetEnumUnderlyingType());
                    }
                    var sb = new StringBuilder();
                    sb.Append(CreateShortReducedStructMarshalTypeName(genericType));
                    sb.Append('<');
                    int index = 0;
                    foreach (var type in gis.GenericArguments)
                    {
                        if (index > 0)
                        {
                            sb.Append(',');
                        }
                        sb.Append(CreateShortReducedMarshalTypeName(type));
                        index++;
                    }
                    sb.Append('>');
                    return sb.ToString();
                }
                else
                {
                    return GetShortReducedClassMarshalTypeName(t);
                }
            }
            default:
                throw new NotImplementedException(t.ElementType.ToString());
            }
        }

        public static string CreateStubName(PropertyDef propertyDef)
        {
            return CreateShortReducedMarshalTypeName(propertyDef.PropertySig.RetType);
        }

        public static string CreateStubName(MethodDef methodDef, List<TypeSig> klassInst, List<TypeSig> methodInst)
        {
            MethodDesc methodDesc = MethodDesc.CreateMethodDesc(methodDef, klassInst, methodInst, false);
            var sb = new StringBuilder();
            sb.Append(CreateShortReducedMarshalTypeName(methodDesc.ReturnInfo.type));
            sb.Append('(');
            int index = 0;
            foreach (var param in methodDesc.ParamInfos)
            {
                if (index > 0)
                {
                    sb.Append(',');
                }
                sb.Append(CreateShortReducedMarshalTypeName(param.type));
                index++;
            }
            sb.Append(')');
            return sb.ToString();
        }

    }
}