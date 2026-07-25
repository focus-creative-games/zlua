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

            if (LuaMarshalAsXmlRegistry.TryGetParameterRule(parameter, method, out LuaMarshalAsXmlRule paramRule)
                && TryGetDeclaredBinding(
                    paramRule.ToAttribute(),
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
                    GetTypeLevelAttribute(clrType),
                    parameter,
                    method,
                    clrType,
                    direction,
                    isReturnValue: false,
                    out binding))
            {
                return binding;
            }

            if (LuaMarshalAsXmlRegistry.TryGetTypeRule(clrType, out LuaMarshalAsXmlRule typeRule)
                && TryGetDeclaredBinding(
                    typeRule.ToAttribute(),
                    parameter,
                    method,
                    clrType,
                    direction,
                    isReturnValue: false,
                    out binding))
            {
                return binding;
            }

            // MARSHAL_SPEC §4.3: ref/in/out default to OpaqueValue on C#→Lua.
            if (direction == LuaMarshalDirection.CSharpToLua && clrType.IsByRef)
            {
                return new LuaMarshalBinding(LuaMarshalType.OpaqueValue);
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

            if (LuaMarshalAsXmlRegistry.TryGetReturnRule(method, out LuaMarshalAsXmlRule returnRule)
                && TryGetDeclaredBinding(
                    returnRule.ToAttribute(),
                    method.ReturnParameter,
                    method,
                    method.ReturnType,
                    direction,
                    isReturnValue: true,
                    out LuaMarshalBinding bindingFromXml))
            {
                return bindingFromXml;
            }

            LuaMarshalAsAttribute typeAttribute = GetTypeLevelAttribute(method.ReturnType);
            if (typeAttribute != null
                && TryGetDeclaredBinding(
                    typeAttribute,
                    method.ReturnParameter,
                    method,
                    method.ReturnType,
                    direction,
                    isReturnValue: true,
                    out LuaMarshalBinding binding))
            {
                return binding;
            }

            if (LuaMarshalAsXmlRegistry.TryGetTypeRule(method.ReturnType, out LuaMarshalAsXmlRule typeRule)
                && TryGetDeclaredBinding(
                    typeRule.ToAttribute(),
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

            // Method-level [LuaMarshalAs] is not supported (MARSHAL_SPEC §6).
            if (method.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false) != null)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] LuaMarshalAs configuration error: "
                    + method.DeclaringType?.FullName + "." + method.Name
                    + "\n  LuaMarshalAsAttribute must not be applied to methods.");
            }

            ValidateTypeLevelConfiguration(method.DeclaringType);

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
                attribute.Members,
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
            if (!LuaMarshalAsXmlRegistry.IsDeterminedMarshalTargetType(targetType))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                    + "LuaMarshalAs cannot be applied to undetermined generic types "
                    + "(e.g. type parameter T, List<T>, or open generic definitions).");
            }

            switch (marshalType)
            {
                case LuaMarshalType.Table:
                case LuaMarshalType.UnpackedValues:
                    if (attribute.Members == null || attribute.Members.Length == 0)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                            + "LuaMarshalType." + marshalType + " requires non-empty Members.");
                    }

                    if (marshalType == LuaMarshalType.UnpackedValues)
                    {
                        for (int i = 0; i < attribute.Members.Length; i++)
                        {
                            if (HasOptionalSuffix(attribute.Members[i]))
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
                && attribute.Members != null
                && attribute.Members.Length > 0)
            {
                for (int i = 0; i < attribute.Members.Length; i++)
                {
                    if (HasOptionalSuffix(attribute.Members[i]))
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
                        + "Members contains an empty entry.");
                }

                bool optional = marshalType == LuaMarshalType.Table && HasOptionalSuffix(rawName);
                string clrName = optional ? rawName.Substring(0, rawName.Length - 1) : rawName;
                if (string.IsNullOrWhiteSpace(clrName))
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "Members entry '" + rawName + "' is invalid.");
                }

                MemberInfo member = ResolveMember(targetType, clrName);
                if (member == null)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + memberSignature + "\n  "
                        + "Members entry '" + clrName + "' is not a public field or property on "
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
                        + "Members entry '" + clrName + "' must be an instance member.");
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

            if (marshalType == LuaMarshalType.OpaqueValue)
            {
                if (direction != LuaMarshalDirection.CSharpToLua)
                {
                    return "LuaMarshalType.OpaqueValue is CSharpToLua-only; falling back to Default.";
                }

                // ref/in/out (any T): always allowed (default OpaqueValue).
                if (parameter != null && parameter.ParameterType.IsByRef)
                {
                    return null;
                }

                if (clrType != null && clrType.IsByRef)
                {
                    return null;
                }

                // by-val: only managed reference types or ordinary structs.
                if (IsOpaqueValueByValAllowed(targetType))
                {
                    return null;
                }

                return $"LuaMarshalType.{marshalType} is not allowed for {targetType.FullName}; falling back to Default.";
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

            // MARSHAL_SPEC §6.1–§6.2: UserData only on reference types / struct.
            // Primitives, enum, IntPtr/UIntPtr are illegal (fall back to Default).
            if (targetType.IsPrimitive
                || targetType.IsEnum
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr))
            {
                return false;
            }

            return targetType == typeof(string)
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

        /// <summary>
        /// MARSHAL_SPEC §4.2 / §6.2: by-val OpaqueValue only for reference types or ordinary struct.
        /// </summary>
        private static bool IsOpaqueValueByValAllowed(Type targetType)
        {
            if (targetType == null || IsUnsupportedMarshalType(targetType))
            {
                return false;
            }

            if (IsStructType(targetType))
            {
                return true;
            }

            // Managed reference types (class / interface / array / string / delegate / object).
            if (targetType.IsPointer || targetType.IsByRef)
            {
                return false;
            }

            if (targetType.IsPrimitive || targetType.IsEnum
                || targetType == typeof(IntPtr) || targetType == typeof(UIntPtr))
            {
                return false;
            }

            return targetType.IsClass
                || targetType.IsInterface
                || targetType.IsArray
                || typeof(Delegate).IsAssignableFrom(targetType);
        }

        private static bool IsSzArray(Type type)
        {
            return type != null && type.IsArray && type.GetArrayRank() == 1;
        }

        /// <summary>
        /// Type-level [LuaMarshalAs] is only valid on non-generic class/struct definitions (spec §1.1).
        /// </summary>
        public static void ValidateTypeLevelConfiguration(Type type)
        {
            if (type == null)
            {
                return;
            }

            LuaMarshalAsAttribute attr = type.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (attr == null || attr.LuaMarshalType == LuaMarshalType.Default)
            {
                return;
            }

            if (type.IsGenericTypeDefinition || type.IsGenericType)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] LuaMarshalAs configuration error: " + (type.FullName ?? type.Name) + "\n  "
                    + "LuaMarshalAsAttribute must not be applied to generic type definitions "
                    + "(only non-generic types; closed generic positions use member-level attributes).");
            }
        }

        private static LuaMarshalAsAttribute GetTypeLevelAttribute(Type clrType)
        {
            Type owner = UnwrapType(clrType);
            // Type-level rules never apply to open or closed generic types (spec §1.1).
            if (owner == null || owner.IsGenericType)
            {
                return null;
            }

            return owner.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
        }

        private static Type UnwrapType(Type clrType)
        {
            Type targetType = Nullable.GetUnderlyingType(clrType) ?? clrType;
            if (targetType != null && targetType.IsByRef)
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

        /// Optional deferred reporter (Mono wires this at init; must not log with stack traces from Lua callbacks).
        public static Action<string> ReportInvalidConfiguration;

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

            string message = "[ZLua] Invalid LuaMarshalAs: " + memberSignature + "\n  " + clrType.FullName + ": " + reason;
            Action<string> reporter = ReportInvalidConfiguration;
            if (reporter != null)
            {
                reporter(message);
                return;
            }

#if UNITY_EDITOR
            UnityEngine.Debug.LogFormat(
                UnityEngine.LogType.Error,
                UnityEngine.LogOption.NoStacktrace,
                null,
                message);
#endif
        }
    }
}
