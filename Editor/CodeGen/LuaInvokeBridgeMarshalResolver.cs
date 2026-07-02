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
                return Validate(declared, clrType, parameter, LuaMarshalDirection.CSharpToLua);
            }

            // MARSHAL_SPEC §4.3: ref/in/out default to OpaqueValue on C#→Lua.
            if (clrType.IsByRef)
            {
                return LuaMarshalType.OpaqueLightUserData;
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
                return Validate(declared, retType, null, LuaMarshalDirection.LuaToCSharp);
            }

            return LuaMarshalType.Default;
        }

        private static LuaMarshalType Validate(
            LuaMarshalType marshalType,
            TypeSig clrType,
            Parameter parameter,
            LuaMarshalDirection direction)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return LuaMarshalType.Default;
            }

            if (marshalType == LuaMarshalType.Table
                || marshalType == LuaMarshalType.UnpackedValues
                || marshalType == LuaMarshalType.ParamsTable)
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

                // ref/in/out: always allowed (default OpaqueValue).
                if (clrType != null && clrType.IsByRef)
                {
                    return null;
                }

                TypeSig byValType = clrType;
                if (byValType != null && byValType.IsByRef)
                {
                    byValType = byValType.Next;
                }

                if (IsOpaqueValueByValAllowed(byValType))
                {
                    return null;
                }

                return "OpaqueLightUserData requires ref/in/out, struct, or managed reference type.";
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

        internal static bool IsUserDataAllowedPublic(TypeSig typeSig) => IsUserDataAllowed(typeSig);

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

            // MARSHAL_SPEC §6.1–§6.2: reference types + struct only (not primitives / enum / IntPtr).
            switch (typeSig.ElementType)
            {
                case ElementType.String:
                case ElementType.Object:
                case ElementType.SZArray:
                case ElementType.Array:
                case ElementType.Class:
                    return true;
                case ElementType.ValueType:
                    return IsStructType(typeSig);
                default:
                    if (IsCorlibType(typeSig, "System", "IntPtr")
                        || IsCorlibType(typeSig, "System", "UIntPtr"))
                    {
                        return false;
                    }

                    // Generic instantiations / other reference-like forms.
                    if (typeSig.IsGenericInstanceType)
                    {
                        TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
                        if (typeDef != null && typeDef.IsClass && !typeDef.IsValueType)
                        {
                            return true;
                        }
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

        private static bool IsOpaqueValueByValAllowed(TypeSig typeSig)
        {
            if (typeSig == null || typeSig.IsByRef)
            {
                return false;
            }

            switch (typeSig.ElementType)
            {
                case ElementType.String:
                case ElementType.Object:
                case ElementType.SZArray:
                case ElementType.Array:
                case ElementType.Class:
                    return true;
                case ElementType.ValueType:
                    return IsStructType(typeSig);
                default:
                    if (IsCorlibType(typeSig, "System", "IntPtr")
                        || IsCorlibType(typeSig, "System", "UIntPtr"))
                    {
                        return false;
                    }

                    if (typeSig.IsGenericInstanceType)
                    {
                        TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
                        if (typeDef != null && typeDef.IsClass && !typeDef.IsValueType)
                        {
                            return true;
                        }
                    }

                    return false;
            }
        }

        internal static bool IsByteArray(TypeSig typeSig)
        {
            return typeSig != null
                && typeSig.ElementType == ElementType.SZArray
                && typeSig.Next?.ElementType == ElementType.U1;
        }

        internal static bool IsEnumTypePublic(TypeSig typeSig)
        {
            if (typeSig?.ElementType != ElementType.ValueType)
            {
                return false;
            }

            TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
            return typeDef != null && typeDef.IsEnum;
        }

        internal static bool IsStructTypePublic(TypeSig typeSig) => IsStructType(typeSig);

        internal static bool IsOpaqueValueByValAllowedPublic(TypeSig typeSig) => IsOpaqueValueByValAllowed(typeSig);

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
