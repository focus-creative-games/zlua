using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    internal enum LuaMarshalDirection
    {
        LuaToCSharp,
        CSharpToLua,
    }

    /// <summary>
    /// Resolves effective <see cref="LuaMarshalType"/> per MARSHAL_SPEC §6.2–§6.4.
    /// Invalid annotations fall back to <see cref="LuaMarshalType.Default"/> (§6.3).
    /// </summary>
    internal static class LuaMarshalAsResolver
    {
        private static readonly HashSet<string> LoggedKeys = new HashSet<string>();

        internal static LuaMarshalType ResolveParameter(
            ParameterInfo parameter,
            MethodBase method,
            LuaMarshalDirection direction,
            Type effectiveClrType = null)
        {
            Type clrType = effectiveClrType ?? parameter?.ParameterType;
            if (clrType == null)
            {
                return LuaMarshalType.Default;
            }

            if (parameter != null)
            {
                LuaMarshalAsAttribute parameterAttribute = parameter.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
                if (parameterAttribute != null && parameterAttribute.LuaMarshalType != LuaMarshalType.Default)
                {
                    return Validate(
                        parameterAttribute.LuaMarshalType,
                        clrType,
                        direction,
                        BuildMemberSignature(method, parameter));
                }
            }

            if (method is MethodInfo methodInfo)
            {
                LuaMarshalAsAttribute methodAttribute = methodInfo.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
                if (methodAttribute != null && methodAttribute.LuaMarshalType != LuaMarshalType.Default)
                {
                    return Validate(
                        methodAttribute.LuaMarshalType,
                        clrType,
                        direction,
                        BuildMemberSignature(method, parameter));
                }
            }

            return LuaMarshalType.Default;
        }

        internal static LuaMarshalType ResolveReturn(MethodInfo method, LuaMarshalDirection direction)
        {
            if (method == null || method.ReturnType == typeof(void))
            {
                return LuaMarshalType.Default;
            }

            foreach (LuaMarshalAsAttribute attribute in method.ReturnParameter.GetCustomAttributes(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                if (attribute.LuaMarshalType != LuaMarshalType.Default)
                {
                    return Validate(
                        attribute.LuaMarshalType,
                        method.ReturnType,
                        direction,
                        BuildMemberSignature(method, returnValue: true));
                }
            }

            LuaMarshalAsAttribute methodAttribute = method.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (methodAttribute != null && methodAttribute.LuaMarshalType != LuaMarshalType.Default)
            {
                return Validate(
                    methodAttribute.LuaMarshalType,
                    method.ReturnType,
                    direction,
                    BuildMemberSignature(method, returnValue: true));
            }

            return LuaMarshalType.Default;
        }

        private static LuaMarshalType Validate(
            LuaMarshalType marshalType,
            Type clrType,
            LuaMarshalDirection direction,
            string memberSignature)
        {
            Type targetType = Nullable.GetUnderlyingType(clrType) ?? clrType;
            if (targetType.IsByRef)
            {
                targetType = targetType.GetElementType();
            }

            string reason = GetInvalidReason(marshalType, targetType, direction);
            if (reason == null)
            {
                return marshalType;
            }

            LogInvalidOnce(memberSignature, targetType, marshalType, reason);
            return LuaMarshalType.Default;
        }

        private static string GetInvalidReason(LuaMarshalType marshalType, Type targetType, LuaMarshalDirection direction)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return null;
            }

            if (marshalType == LuaMarshalType.OpaqueLightUserData)
            {
                if (direction != LuaMarshalDirection.CSharpToLua)
                {
                    return "LuaMarshalType.OpaqueLightUserData is CSharpToLua-only; falling back to Default for LuaToCSharp.";
                }

                if (!ValueTypeMarshaling.IsStructType(targetType))
                {
                    return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}.";
                }

                return null;
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                if (targetType == typeof(byte[]) || targetType == typeof(string))
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}.";
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                if (IsUserDataAllowed(targetType))
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}.";
            }

            return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}.";
        }

        private static bool IsUserDataAllowed(Type targetType)
        {
            if (targetType == null)
            {
                return false;
            }

            if (PointerMarshaling.IsPointerLikeType(targetType)
                || PointerMarshaling.IsTypedReference(targetType)
                || PointerMarshaling.IsUnsupportedMarshalType(targetType))
            {
                return false;
            }

            if (targetType.IsPrimitive
                || targetType == typeof(string)
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr)
                || targetType.IsEnum
                || ValueTypeMarshaling.IsStructType(targetType)
                || targetType.IsArray
                || targetType == typeof(object)
                || typeof(Delegate).IsAssignableFrom(targetType)
                || targetType.IsClass
                || targetType.IsInterface)
            {
                return true;
            }

            return false;
        }

        private static string BuildMemberSignature(MethodBase method, ParameterInfo parameter = null, bool returnValue = false)
        {
            if (method == null)
            {
                return "unknown";
            }

            string name = method.DeclaringType?.FullName + "." + method.Name;
            if (returnValue)
            {
                return name + " (return value)";
            }

            if (parameter != null)
            {
                return name + " parameter '" + parameter.Name + "'";
            }

            return name;
        }

        private static void LogInvalidOnce(string memberSignature, Type clrType, LuaMarshalType marshalType, string reason)
        {
            string key = memberSignature + "|" + clrType.AssemblyQualifiedName + "|" + marshalType;
            lock (LoggedKeys)
            {
                if (!LoggedKeys.Add(key))
                {
                    return;
                }
            }

#if UNITY_EDITOR
            LuaPrintBuffer.EnqueueEditorError(
                "[ZLua] Invalid LuaMarshalAs: " + memberSignature + "\n  "
                + clrType.FullName + ": " + reason);
#endif
        }
    }
}
