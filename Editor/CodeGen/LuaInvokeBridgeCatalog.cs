using System;
using System.Collections.Generic;
using System.Text;
using dnlib.DotNet;

namespace ZLua
{
    internal static class LuaInvokeBridgeCatalog
    {
        internal static bool TryResolveBridgeMethod(
            ModuleDefMD module,
            MethodDef method,
            LuaInvokeILPostProcessorAssemblyResolver resolver,
            out IMethod bridgeMethod)
        {
            bridgeMethod = null;
            if (method?.MethodSig == null)
            {
                return false;
            }

            MethodSig sig = method.MethodSig;
            LuaMarshalType retMarshal = LuaInvokeBridgeMarshalResolver.ResolveReturn(method);
            var paramMarshals = new LuaMarshalType[sig.Params.Count];
            bool allDefault = retMarshal == LuaMarshalType.Default;
            int paramIndexBase = method.Parameters.MethodSigIndexBase;
            for (int i = 0; i < sig.Params.Count; i++)
            {
                paramMarshals[i] = LuaInvokeBridgeMarshalResolver.ResolveParameter(
                    method.Parameters[paramIndexBase + i],
                    method);
                if (paramMarshals[i] != LuaMarshalType.Default)
                {
                    allDefault = false;
                }
            }

            string bridgeName;
            TypeSig[] genericArgs = null;
            if (allDefault)
            {
                bridgeName = BuildDefaultBridgeMethodName(method);
            }
            else if (!TryBuildMarshaledBridgeName(sig, retMarshal, paramMarshals, out bridgeName))
            {
                return false;
            }
            else
            {
                genericArgs = ResolveGenericTypeSigs(sig, paramMarshals, bridgeName);
            }

            if (string.IsNullOrEmpty(bridgeName))
            {
                return false;
            }

            bridgeMethod = LuaInvokeMonoReferenceImporter.ImportBridgeMethod(module, resolver, bridgeName, genericArgs);
            return bridgeMethod != null;
        }

        private static TypeSig[] ResolveGenericTypeSigs(
            MethodSig sig,
            LuaMarshalType[] paramMarshals,
            string bridgeName)
        {
            if (string.Equals(bridgeName, "InvokeM_UEnum", StringComparison.Ordinal))
            {
                return new[] { sig.RetType };
            }

            if (string.Equals(bridgeName, "InvokeM_UEnum_UEnum", StringComparison.Ordinal))
            {
                for (int i = 0; i < sig.Params.Count; i++)
                {
                    if (LuaInvokeBridgeMarshalResolver.IsEnumTypePublic(sig.Params[i]))
                    {
                        return new[] { sig.Params[i] };
                    }
                }

                return new[] { sig.RetType };
            }

            if (string.Equals(bridgeName, "InvokeM_Int32_OOpaque", StringComparison.Ordinal))
            {
                for (int i = 0; i < sig.Params.Count; i++)
                {
                    if (paramMarshals[i] == LuaMarshalType.OpaqueLightUserData)
                    {
                        return new[] { sig.Params[i] };
                    }
                }
            }

            return null;
        }

        private static bool TryBuildMarshaledBridgeName(
            MethodSig sig,
            LuaMarshalType retMarshal,
            LuaMarshalType[] paramMarshals,
            out string bridgeName)
        {
            var sb = new StringBuilder("InvokeM_");

            if (!TryAppendMarshaledTypeName(sb, sig.RetType, retMarshal))
            {
                bridgeName = null;
                return false;
            }

            for (int i = 0; i < sig.Params.Count; i++)
            {
                sb.Append('_');
                if (!TryAppendMarshaledTypeName(sb, sig.Params[i], paramMarshals[i]))
                {
                    bridgeName = null;
                    return false;
                }
            }

            bridgeName = sb.ToString();
            return true;
        }

        private static bool TryAppendMarshaledTypeName(
            StringBuilder sb,
            TypeSig typeSig,
            LuaMarshalType marshalType)
        {
            if (typeSig == null)
            {
                sb.Append("Object");
                return true;
            }

            if (typeSig.IsByRef)
            {
                return false;
            }

            if (marshalType == LuaMarshalType.Default)
            {
                AppendDefaultTypeName(sb, typeSig);
                return true;
            }

            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (!LuaInvokeBridgeMarshalResolver.IsByteArray(typeSig))
                    {
                        return false;
                    }

                    sb.Append("BByteArray");
                    return true;
                case LuaMarshalType.UserData:
                    if (LuaInvokeBridgeMarshalResolver.IsEnumTypePublic(typeSig))
                    {
                        sb.Append("UEnum");
                        return true;
                    }

                    sb.Append('U');
                    AppendDefaultTypeName(sb, typeSig);
                    return true;
                case LuaMarshalType.OpaqueLightUserData:
                    if (!LuaInvokeBridgeMarshalResolver.IsStructTypePublic(typeSig))
                    {
                        return false;
                    }

                    sb.Append("OOpaque");
                    return true;
                default:
                    return false;
            }
        }

        internal static string BuildDefaultBridgeMethodName(MethodDef method)
        {
            MethodSig sig = method.MethodSig;
            if (sig == null)
            {
                return null;
            }

            if (sig.RetType.ElementType == ElementType.Void && sig.Params.Count == 0)
            {
                return "InvokeVoid";
            }

            var sb = new StringBuilder("Invoke_");
            AppendDefaultTypeName(sb, sig.RetType);
            for (int i = 0; i < sig.Params.Count; i++)
            {
                sb.Append('_');
                AppendDefaultTypeName(sb, sig.Params[i]);
            }

            return sb.ToString();
        }

        private static void AppendDefaultTypeName(StringBuilder sb, TypeSig typeSig)
        {
            if (typeSig == null)
            {
                sb.Append("Object");
                return;
            }

            if (typeSig.IsByRef)
            {
                throw new NotSupportedException("ref/out is not supported for fast LuaInvoke.");
            }

            switch (typeSig.ElementType)
            {
                case ElementType.Void:
                    sb.Append("Void");
                    return;
                case ElementType.Boolean:
                    sb.Append("Boolean");
                    return;
                case ElementType.Char:
                    sb.Append("Char");
                    return;
                case ElementType.I1:
                    sb.Append("SByte");
                    return;
                case ElementType.U1:
                    sb.Append("Byte");
                    return;
                case ElementType.I2:
                    sb.Append("Int16");
                    return;
                case ElementType.U2:
                    sb.Append("UInt16");
                    return;
                case ElementType.I4:
                    sb.Append("Int32");
                    return;
                case ElementType.U4:
                    sb.Append("UInt32");
                    return;
                case ElementType.I8:
                    sb.Append("Int64");
                    return;
                case ElementType.U8:
                    sb.Append("UInt64");
                    return;
                case ElementType.R4:
                    sb.Append("Single");
                    return;
                case ElementType.R8:
                    sb.Append("Double");
                    return;
                case ElementType.String:
                    sb.Append("String");
                    return;
                case ElementType.ValueType:
                {
                    TypeDef typeDef = typeSig.ToTypeDefOrRef()?.ResolveTypeDef();
                    if (typeDef != null && typeDef.IsEnum)
                    {
                        sb.Append("Int32");
                        return;
                    }

                    throw new NotSupportedException($"unsupported value type for fast LuaInvoke: {typeSig}");
                }
                case ElementType.Object:
                    sb.Append("Object");
                    return;
                default:
                    if (IsCorlibType(typeSig, "System", "IntPtr"))
                    {
                        sb.Append("IntPtr");
                        return;
                    }

                    if (IsCorlibType(typeSig, "System", "UIntPtr"))
                    {
                        sb.Append("UIntPtr");
                        return;
                    }

                    throw new NotSupportedException($"unsupported type for fast LuaInvoke: {typeSig}");
            }
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
