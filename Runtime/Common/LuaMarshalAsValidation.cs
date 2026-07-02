using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Shared MARSHAL_SPEC §6.2–§6.3 rules for Mono binding and Editor codegen.
    /// </summary>
    public static class LuaMarshalAsValidation
    {
        public static LuaMarshalBinding ResolveParameterBinding(
            ParameterInfo parameter,
            MethodBase method,
            LuaMarshalDirection direction,
            Type effectiveClrType = null)
        {
            Type clrType = effectiveClrType ?? parameter?.ParameterType;
            if (clrType == null)
            {
                return LuaMarshalBinding.Default;
            }

            if (TryGetDeclaredBinding(
                    parameter?.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false),
                    parameter,
                    method,
                    clrType,
                    direction,
                    isReturnValue: false,
                    out LuaMarshalBinding binding))
            {
                return binding;
            }

            if (method is MethodInfo methodInfo
                && TryGetDeclaredBinding(
                    methodInfo.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false),
                    parameter,
                    method,
                    clrType,
                    direction,
                    isReturnValue: false,
                    out binding))
            {
                return binding;
            }

            if (TryGetDeclaredBinding(
                    clrType.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false),
                    parameter,
                    method,
                    clrType,
                    direction,
                    isReturnValue: false,
                    out binding))
            {
                return binding;
            }

            return LuaMarshalBinding.Default;
        }

        public static LuaMarshalBinding ResolveReturnBinding(MethodInfo method, LuaMarshalDirection direction)
        {
            if (method == null || method.ReturnType == typeof(void))
            {
                return LuaMarshalBinding.Default;
            }

            foreach (LuaMarshalAsAttribute attribute in method.ReturnParameter.GetCustomAttributes(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                if (TryGetDeclaredBinding(
                        attribute,
                        method.ReturnParameter,
                        method,
                        method.ReturnType,
                        direction,
                        isReturnValue: true,
                        out LuaMarshalBinding binding1))
                {
                    return binding1;
                }
            }

            LuaMarshalAsAttribute methodAttribute = method.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (methodAttribute != null
                && TryGetDeclaredBinding(
                    methodAttribute,
                    method.ReturnParameter,
                    method,
                    method.ReturnType,
                    direction,
                    isReturnValue: true,
                    out LuaMarshalBinding binding))
            {
                return binding;
            }

            LuaMarshalAsAttribute typeAttribute = method.ReturnType.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (typeAttribute != null
                && TryGetDeclaredBinding(
                    typeAttribute,
                    method.ReturnParameter,
                    method,
                    method.ReturnType,
                    direction,
                    isReturnValue: true,
                    out binding))
            {
                return binding;
            }

            return LuaMarshalBinding.Default;
        }

        public static void ValidateMethodConfiguration(MethodBase method)
        {
            if (method == null)
            {
                return;
            }

            if (method is MethodInfo methodInfo && methodInfo.ReturnType != typeof(void))
            {
                ResolveReturnBinding(methodInfo, LuaMarshalDirection.LuaToCSharp);
                ResolveReturnBinding(methodInfo, LuaMarshalDirection.CSharpToLua);
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                ResolveParameterBinding(parameters[i], method, LuaMarshalDirection.LuaToCSharp);
                ResolveParameterBinding(parameters[i], method, LuaMarshalDirection.CSharpToLua);
            }
        }

        private static bool TryGetDeclaredBinding(
            LuaMarshalAsAttribute attribute,
            ParameterInfo parameter,
            MethodBase method,
            Type clrType,
            LuaMarshalDirection direction,
            bool isReturnValue,
            out LuaMarshalBinding binding)
        {
            binding = LuaMarshalBinding.Default;
            if (attribute == null || attribute.LuaMarshalType == LuaMarshalType.Default)
            {
                return false;
            }

            string memberSignature = BuildMemberSignature(method, parameter, isReturnValue);
            LuaMarshalType marshalType = attribute.LuaMarshalType;
            Type targetType = UnwrapType(clrType);

            ValidateConfigurationOrThrow(
                marshalType,
                attribute,
                targetType,
                parameter,
                method,
                direction,
                memberSignature);

            string invalidReason = GetTypeOrDirectionInvalidReason(marshalType, targetType, parameter, direction);
            if (invalidReason != null)
            {
                LogInvalidOnce(memberSignature, targetType, marshalType, invalidReason);
                return false;
            }

            LuaMarshalMemberBinding[] members = ExpandMembers(
                marshalType,
                targetType,
                attribute.FieldOrPropertyNames,
                direction,
                memberSignature);

            binding = new LuaMarshalBinding(marshalType, members);
            return true;
        }

        private static void ValidateConfigurationOrThrow(
            LuaMarshalType marshalType,
            LuaMarshalAsAttribute attribute,
            Type targetType,
            ParameterInfo parameter,
            MethodBase method,
            LuaMarshalDirection direction,
            string memberSignature)
        {
            switch (marshalType)
            {
                case LuaMarshalType.Table:
                case LuaMarshalType.UnpackedValues:
                    if (attribute.FieldOrPropertyNames == null || attribute.FieldOrPropertyNames.Length == 0)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType." + marshalType + " requires non-empty FieldOrPropertyNames.");
                    }

                    if (marshalType == LuaMarshalType.UnpackedValues)
                    {
                        for (int i = 0; i < attribute.FieldOrPropertyNames.Length; i++)
                        {
                            if (HasOptionalSuffix(attribute.FieldOrPropertyNames[i]))
                            {
                                throw new LuaMarshalAsConfigurationException(
                                    "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                                    + "UnpackedValues does not support optional member suffix '?'.");
                            }
                        }
                    }

                    if (targetType.IsInterface)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType." + marshalType + " is not allowed on interface types.");
                    }

                    if (!IsStructType(targetType) && !targetType.IsClass)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType." + marshalType + " requires struct or class type.");
                    }
                    break;

                case LuaMarshalType.ParamsTable:
                    if (parameter == null || !parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false))
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType.ParamsTable requires a params T[] parameter.");
                    }

                    if (!IsSzArray(targetType))
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType.ParamsTable requires a one-dimensional array parameter.");
                    }
                    break;
            }

            if (marshalType != LuaMarshalType.Table
                && attribute.FieldOrPropertyNames != null
                && attribute.FieldOrPropertyNames.Length > 0)
            {
                for (int i = 0; i < attribute.FieldOrPropertyNames.Length; i++)
                {
                    if (HasOptionalSuffix(attribute.FieldOrPropertyNames[i]))
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "Optional member suffix '?' is only allowed with LuaMarshalType.Table.");
                    }
                }
            }
        }

        private static LuaMarshalMemberBinding[] ExpandMembers(
            LuaMarshalType marshalType,
            Type targetType,
            string[] fieldOrPropertyNames,
            LuaMarshalDirection direction,
            string memberSignature)
        {
            if (marshalType != LuaMarshalType.Table && marshalType != LuaMarshalType.UnpackedValues)
            {
                return Array.Empty<LuaMarshalMemberBinding>();
            }

            var members = new LuaMarshalMemberBinding[fieldOrPropertyNames.Length];
            for (int i = 0; i < fieldOrPropertyNames.Length; i++)
            {
                string rawName = fieldOrPropertyNames[i];
                if (string.IsNullOrWhiteSpace(rawName))
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "FieldOrPropertyNames contains an empty entry.");
                }

                bool optional = marshalType == LuaMarshalType.Table && HasOptionalSuffix(rawName);
                string clrName = optional ? rawName.Substring(0, rawName.Length - 1) : rawName;
                if (string.IsNullOrWhiteSpace(clrName))
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "FieldOrPropertyNames entry '" + rawName + "' is invalid.");
                }

                MemberInfo member = ResolveMember(targetType, clrName);
                if (member == null)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "FieldOrPropertyNames entry '" + clrName + "' is not a public field or property on "
                        + targetType.FullName + ".");
                }

                ValidateMemberAccess(member, direction, memberSignature, clrName);
                members[i] = new LuaMarshalMemberBinding(clrName, optional, member);
            }

            return members;
        }

        private static void ValidateMemberAccess(
            MemberInfo member,
            LuaMarshalDirection direction,
            string memberSignature,
            string clrName)
        {
            if (member is FieldInfo field)
            {
                if (field.IsStatic)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "FieldOrPropertyNames entry '" + clrName + "' must be an instance member.");
                }

                return;
            }

            if (member is PropertyInfo property)
            {
                if (direction == LuaMarshalDirection.LuaToCSharp && !property.CanWrite)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "Property '" + clrName + "' is not writable for Lua→C#.");
                }

                if (direction == LuaMarshalDirection.CSharpToLua && !property.CanRead)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "Property '" + clrName + "' is not readable for C#→Lua.");
                }
            }
        }

        private static MemberInfo ResolveMember(Type targetType, string name)
        {
            const BindingFlags flags = BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase;
            FieldInfo field = targetType.GetField(name, flags);
            if (field != null)
            {
                return field;
            }

            PropertyInfo property = targetType.GetProperty(name, flags);
            return property;
        }

        public static string GetTypeOrDirectionInvalidReason(
            LuaMarshalType marshalType,
            Type clrType,
            ParameterInfo parameter,
            LuaMarshalDirection direction)
        {
            Type targetType = UnwrapType(clrType);
            if (marshalType == LuaMarshalType.Default)
            {
                return null;
            }

            if (marshalType == LuaMarshalType.OpaqueLightUserData)
            {
                if (direction != LuaMarshalDirection.CSharpToLua)
                {
                    return "LuaMarshalType.OpaqueLightUserData is CSharpToLua-only; falling back to Default.";
                }

                return IsStructType(targetType)
                    ? null
                    : $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                if (targetType == typeof(byte[]) || targetType == typeof(string))
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                return IsUserDataAllowed(targetType)
                    ? null
                    : $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
            }

            if (marshalType == LuaMarshalType.Table || marshalType == LuaMarshalType.UnpackedValues)
            {
                if (targetType.IsInterface)
                {
                    return $"LuaMarshalType.{marshalType} is not allowed for interface {targetType.FullName}; falling back to Default.";
                }

                if (IsStructType(targetType) || targetType.IsClass)
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
            }

            if (marshalType == LuaMarshalType.ParamsTable)
            {
                if (parameter != null
                    && parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false)
                    && IsSzArray(targetType))
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
            }

            return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
        }

        private static bool IsUserDataAllowed(Type targetType)
        {
            if (targetType == null || IsUnsupportedMarshalType(targetType))
            {
                return false;
            }

            if (targetType.IsPointer || targetType.IsByRef)
            {
                return false;
            }

            return targetType.IsPrimitive
                || targetType == typeof(string)
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr)
                || targetType.IsEnum
                || IsStructType(targetType)
                || targetType.IsArray
                || targetType == typeof(object)
                || typeof(Delegate).IsAssignableFrom(targetType)
                || targetType.IsClass
                || targetType.IsInterface;
        }

        private static bool IsUnsupportedMarshalType(Type type)
        {
            if (type == typeof(TypedReference) || type == typeof(decimal))
            {
                return true;
            }

            return type.IsByRef && IsRefStruct(type.GetElementType());
        }

        private static bool IsRefStruct(Type type)
        {
            if (type == null)
            {
                return false;
            }

            return type.IsValueType && type.IsByRefLike;
        }

        private static bool IsStructType(Type type)
        {
            return type != null
                && type.IsValueType
                && !type.IsEnum
                && !type.IsPrimitive
                && type != typeof(decimal)
                && !IsRefStruct(type);
        }

        private static bool IsSzArray(Type type)
        {
            return type != null && type.IsArray && type.GetArrayRank() == 1;
        }

        private static Type UnwrapType(Type clrType)
        {
            Type targetType = Nullable.GetUnderlyingType(clrType) ?? clrType;
            if (targetType.IsByRef)
            {
                targetType = targetType.GetElementType();
            }

            return targetType;
        }

        private static bool HasOptionalSuffix(string name)
        {
            return name != null && name.EndsWith("?", StringComparison.Ordinal);
        }

        private static string BuildMemberSignature(MethodBase method, ParameterInfo parameter, bool isReturnValue)
        {
            if (method == null)
            {
                return "unknown";
            }

            string name = method.DeclaringType?.FullName + "." + method.Name;
            if (isReturnValue)
            {
                return name + " (return value)";
            }

            if (parameter != null)
            {
                return name + " parameter '" + parameter.Name + "'";
            }

            return name;
        }

        private static readonly HashSet<string> LoggedKeys = new HashSet<string>();

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
            UnityEngine.Debug.LogError("[ZLua] Invalid LuaMarshalAs: " + memberSignature + "\n  " + clrType.FullName + ": " + reason);
#endif
        }
    }
}
