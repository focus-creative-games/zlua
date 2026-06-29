using System;
using dnlib.DotNet;

namespace ZLua
{
    internal static class LuaInvokeBridgeMarshalResolver
    {
        internal static LuaMarshalType ResolveParameter(Parameter parameter, MethodDef method)
        {
            TypeSig clrType = parameter?.Type;
            if (clrType == null)
            {
                return LuaMarshalType.Default;
            }

            ParamDef paramDef = parameter.ParamDef;
            if (paramDef != null && TryReadDeclaredMarshal(paramDef.CustomAttributes, out LuaMarshalType declared))
            {
                return Validate(declared, clrType, LuaMarshalDirection.CSharpToLua);
            }

            if (TryReadDeclaredMarshal(method.CustomAttributes, out LuaMarshalType methodDeclared))
            {
                return Validate(methodDeclared, clrType, LuaMarshalDirection.CSharpToLua);
            }

            return LuaMarshalType.Default;
        }

        internal static LuaMarshalType ResolveReturn(MethodDef method)
        {
            TypeSig retType = method.MethodSig?.RetType;
            if (retType == null || retType.ElementType == ElementType.Void)
            {
                return LuaMarshalType.Default;
            }

            ParamDef returnParamDef = method.Parameters.ReturnParameter.ParamDef;
            if (returnParamDef != null
                && TryReadDeclaredMarshal(returnParamDef.CustomAttributes, out LuaMarshalType declared))
            {
                return Validate(declared, retType, LuaMarshalDirection.LuaToCSharp);
            }

            if (TryReadDeclaredMarshal(method.CustomAttributes, out LuaMarshalType methodDeclared))
            {
                return Validate(methodDeclared, retType, LuaMarshalDirection.LuaToCSharp);
            }

            return LuaMarshalType.Default;
        }

        private static LuaMarshalType Validate(
            LuaMarshalType marshalType,
            TypeSig clrType,
            LuaMarshalDirection direction)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return LuaMarshalType.Default;
            }

            return GetInvalidReason(marshalType, clrType, direction) == null
                ? marshalType
                : LuaMarshalType.Default;
        }

        private static string GetInvalidReason(LuaMarshalType marshalType, TypeSig clrType, LuaMarshalDirection direction)
        {
            if (marshalType == LuaMarshalType.OpaqueLightUserData)
            {
                if (direction != LuaMarshalDirection.CSharpToLua)
                {
                    return "OpaqueLightUserData is CSharpToLua-only.";
                }

                if (!IsStructType(clrType))
                {
                    return "OpaqueLightUserData requires struct type.";
                }

                return null;
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                if (IsByteArray(clrType) || clrType.ElementType == ElementType.String)
                {
                    return null;
                }

                return "Bytes requires byte[] or string.";
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                return IsUserDataAllowed(clrType) ? null : "UserData not allowed for type.";
            }

            return "Unsupported marshal type.";
        }

        private static bool IsUserDataAllowed(TypeSig typeSig)
        {
            if (typeSig == null)
            {
                return false;
            }

            if (typeSig.IsByRef)
            {
                return false;
            }

            switch (typeSig.ElementType)
            {
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
                case ElementType.String:
                case ElementType.Object:
                case ElementType.SZArray:
                    return true;
                case ElementType.ValueType:
                    return true;
                default:
                    if (IsCorlibType(typeSig, "System", "IntPtr")
                        || IsCorlibType(typeSig, "System", "UIntPtr"))
                    {
                        return true;
                    }

                    return false;
            }
        }

        private static bool IsStructType(TypeSig typeSig)
        {
            if (typeSig?.ElementType != ElementType.ValueType)
            {
                return false;
            }

            TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
            return typeDef != null && !typeDef.IsEnum;
        }

        private static bool IsEnumType(TypeSig typeSig)
        {
            if (typeSig?.ElementType != ElementType.ValueType)
            {
                return false;
            }

            TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
            return typeDef != null && typeDef.IsEnum;
        }

        internal static bool IsByteArray(TypeSig typeSig)
        {
            return typeSig != null
                && typeSig.ElementType == ElementType.SZArray
                && typeSig.Next?.ElementType == ElementType.U1;
        }

        internal static bool IsEnumTypePublic(TypeSig typeSig) => IsEnumType(typeSig);

        internal static bool IsStructTypePublic(TypeSig typeSig) => IsStructType(typeSig);

        private static bool TryReadDeclaredMarshal(CustomAttributeCollection attributes, out LuaMarshalType marshalType)
        {
            marshalType = LuaMarshalType.Default;
            for (int i = 0; i < attributes.Count; i++)
            {
                CustomAttribute attribute = attributes[i];
                ITypeDefOrRef typeRef = attribute.AttributeType;
                if (typeRef == null
                    || !string.Equals(typeRef.Namespace, "ZLua", StringComparison.Ordinal)
                    || !string.Equals(typeRef.Name, "LuaMarshalAsAttribute", StringComparison.Ordinal))
                {
                    continue;
                }

                if (attribute.ConstructorArguments.Count == 0)
                {
                    continue;
                }

                object value = attribute.ConstructorArguments[0].Value;
                if (value is int intValue && intValue != (int)LuaMarshalType.Default)
                {
                    marshalType = (LuaMarshalType)intValue;
                    return true;
                }
            }

            return false;
        }

        private static bool IsCorlibType(TypeSig typeSig, string ns, string name)
        {
            ITypeDefOrRef typeRef = typeSig?.ToTypeDefOrRef();
            return typeRef != null
                && string.Equals(typeRef.Namespace, ns, StringComparison.Ordinal)
                && string.Equals(typeRef.Name, name, StringComparison.Ordinal);
        }
    }
}
