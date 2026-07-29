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

            // Method-level [LuaMarshalAs] is not supported (MARSHAL_SPEC §6) — Mono: log + ignore.
            if (method.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false) != null)
            {
                LogInvalidOnce(
                    method.DeclaringType?.FullName + "." + method.Name,
                    method.DeclaringType,
                    LuaMarshalType.Default,
                    "LuaMarshalAsAttribute must not be applied to methods; ignoring.");
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
            Type declaredType = StripByRef(clrType);
            Type memberOwnerType = UnwrapType(clrType);

            string invalidReason = GetConfigurationInvalidReason(
                marshalType,
                attribute,
                declaredType,
                memberOwnerType,
                memberSignature);
            if (invalidReason == null)
            {
                invalidReason = GetTypeOrDirectionInvalidReason(marshalType, clrType, parameter, direction);
            }

            if (invalidReason != null)
            {
                LogInvalidOnce(memberSignature, declaredType ?? memberOwnerType, marshalType, invalidReason);
                return false;
            }

            if (!TryExpandMembers(
                    marshalType,
                    memberOwnerType,
                    attribute.Members,
                    direction,
                    memberSignature,
                    out LuaMarshalMemberBinding[] members,
                    out string expandReason))
            {
                LogInvalidOnce(memberSignature, memberOwnerType, marshalType, expandReason);
                return false;
            }

            binding = new LuaMarshalBinding(marshalType, members);
            return true;
        }

        /// <summary>
        /// Mono Attribute path: configuration problems become log + Default (spec §4.1).
        /// </summary>
        private static string GetConfigurationInvalidReason(
            LuaMarshalType marshalType,
            LuaMarshalAsAttribute attribute,
            Type declaredType,
            Type memberOwnerType,
            string memberSignature)
        {
            if (!LuaMarshalAsXmlRegistry.IsDeterminedMarshalTargetType(declaredType)
                && !LuaMarshalAsXmlRegistry.IsDeterminedMarshalTargetType(memberOwnerType))
            {
                return "LuaMarshalAs cannot be applied to undetermined generic types "
                    + "(e.g. type parameter T, List<T>, or open generic definitions); falling back to Default.";
            }

            if (marshalType == LuaMarshalType.Table || marshalType == LuaMarshalType.UnpackedValues)
            {
                if (attribute.Members == null || attribute.Members.Length == 0)
                {
                    return "LuaMarshalType." + marshalType + " requires non-empty Members; falling back to Default.";
                }

                if (marshalType == LuaMarshalType.UnpackedValues)
                {
                    for (int i = 0; i < attribute.Members.Length; i++)
                    {
                        if (HasOptionalSuffix(attribute.Members[i]))
                        {
                            return "UnpackedValues does not support optional member suffix '?'; falling back to Default.";
                        }
                    }
                }
            }
            else if (attribute.Members != null && attribute.Members.Length > 0)
            {
                for (int i = 0; i < attribute.Members.Length; i++)
                {
                    if (HasOptionalSuffix(attribute.Members[i]))
                    {
                        return "Optional member suffix '?' is only allowed with LuaMarshalType.Table; falling back to Default.";
                    }
                }
            }

            return null;
        }

        private static bool TryExpandMembers(
            LuaMarshalType marshalType,
            Type memberOwnerType,
            string[] fieldOrPropertyNames,
            LuaMarshalDirection direction,
            string memberSignature,
            out LuaMarshalMemberBinding[] members,
            out string reason)
        {
            members = Array.Empty<LuaMarshalMemberBinding>();
            reason = null;
            if (marshalType != LuaMarshalType.Table && marshalType != LuaMarshalType.UnpackedValues)
            {
                return true;
            }

            members = new LuaMarshalMemberBinding[fieldOrPropertyNames.Length];
            for (int i = 0; i < fieldOrPropertyNames.Length; i++)
            {
                string rawName = fieldOrPropertyNames[i];
                if (string.IsNullOrWhiteSpace(rawName))
                {
                    reason = "Members contains an empty entry; falling back to Default.";
                    return false;
                }

                bool optional = marshalType == LuaMarshalType.Table && HasOptionalSuffix(rawName);
                string clrName = optional ? rawName.Substring(0, rawName.Length - 1) : rawName;
                if (string.IsNullOrWhiteSpace(clrName))
                {
                    reason = "Members entry '" + rawName + "' is invalid; falling back to Default.";
                    return false;
                }

                MemberInfo member = ResolveMember(memberOwnerType, clrName);
                if (member == null)
                {
                    reason = "Members entry '" + clrName + "' is not a public field or property on "
                        + memberOwnerType.FullName + "; falling back to Default.";
                    return false;
                }

                if (!TryValidateMemberAccess(member, direction, clrName, out reason))
                {
                    return false;
                }

                members[i] = new LuaMarshalMemberBinding(clrName, optional, member);
            }

            return true;
        }

        private static bool TryValidateMemberAccess(
            MemberInfo member,
            LuaMarshalDirection direction,
            string clrName,
            out string reason)
        {
            reason = null;
            if (member is FieldInfo field)
            {
                if (field.IsStatic)
                {
                    reason = "Members entry '" + clrName + "' must be an instance member; falling back to Default.";
                    return false;
                }

                return true;
            }

            if (member is PropertyInfo property)
            {
                if (direction == LuaMarshalDirection.LuaToCSharp && !property.CanWrite)
                {
                    reason = "Property '" + clrName + "' is not writable for Lua→C#; falling back to Default.";
                    return false;
                }

                if (direction == LuaMarshalDirection.CSharpToLua && !property.CanRead)
                {
                    reason = "Property '" + clrName + "' is not readable for C#→Lua; falling back to Default.";
                    return false;
                }
            }

            return true;
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
            Type declaredType = StripByRef(clrType);
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
                Type nullableUnderlying = declaredType != null ? Nullable.GetUnderlyingType(declaredType) : null;
                if (nullableUnderlying != null)
                {
                    if (marshalType == LuaMarshalType.UnpackedValues)
                    {
                        return "LuaMarshalType.UnpackedValues is not allowed for Nullable<T> "
                            + "(cannot distinguish nil vs non-nil); falling back to Default.";
                    }

                    // Table + Nullable<struct> only.
                    if (!IsStructType(nullableUnderlying))
                    {
                        return $"LuaMarshalType.Table is not allowed for Nullable<{nullableUnderlying.FullName}>; falling back to Default.";
                    }

                    return null;
                }

                if (!IsStructType(declaredType ?? targetType))
                {
                    return $"LuaMarshalType.{marshalType} is only allowed for struct / closed generic struct"
                        + (marshalType == LuaMarshalType.Table ? " / Nullable<struct>" : "")
                        + $"; falling back to Default (got {declaredType?.FullName ?? targetType?.FullName}).";
                }

                return null;
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
        /// Mono: log + ignore invalid type-level attributes.
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
                LogInvalidOnce(
                    type.FullName ?? type.Name,
                    type,
                    attr.LuaMarshalType,
                    "LuaMarshalAsAttribute must not be applied to generic type definitions; falling back to Default.");
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

            // Generic type definitions already rejected in ValidateTypeLevelConfiguration (soft).
            if (owner.IsGenericTypeDefinition)
            {
                return null;
            }

            return owner.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
        }

        private static Type StripByRef(Type clrType)
        {
            if (clrType != null && clrType.IsByRef)
            {
                return clrType.GetElementType();
            }

            return clrType;
        }

        private static Type UnwrapType(Type clrType)
        {
            Type targetType = StripByRef(clrType);
            if (targetType == null)
            {
                return null;
            }

            return Nullable.GetUnderlyingType(targetType) ?? targetType;
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

        /// <summary>
        /// When true, invalid LuaMarshalAs diagnostics are not written to the Unity console.
        /// Intended for unit-test hosts that intentionally exercise invalid attributes.
        /// </summary>
        public static bool SuppressInvalidLogging { get; set; }

        private static void LogInvalidOnce(string memberSignature, Type clrType, LuaMarshalType marshalType, string reason)
        {
            if (SuppressInvalidLogging)
            {
                return;
            }

            string key = memberSignature + "|" + clrType.AssemblyQualifiedName + "|" + marshalType;
            lock (LoggedKeys)
            {
                if (!LoggedKeys.Add(key))
                {
                    return;
                }
            }

            string message = "[ZLua] Invalid LuaMarshalAs: " + memberSignature + "\n  " + clrType.FullName + ": " + reason;
            UnityEngine.Debug.LogError(message);
        }
    }
}
