using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// MarshalAs XML: load stores rules by assembly name; first query per Assembly
    /// lazily binds (spec §9.6):
    /// - Type / Field / Property: Assembly → memberToken → Rule
    /// - Param / Return: Assembly → (methodDefToken, index) → Rule
    ///   (param index >= 0; return index = -1). ParamDef tokens are not required.
    /// </summary>
    public static class LuaMarshalAsXmlRegistry
    {
        private readonly struct MethodSlotKey : IEquatable<MethodSlotKey>
        {
            public readonly int MethodToken;
            public readonly int Index; // >=0 param; -1 return

            public MethodSlotKey(int methodToken, int index)
            {
                MethodToken = methodToken;
                Index = index;
            }

            public bool Equals(MethodSlotKey other)
            {
                return MethodToken == other.MethodToken && Index == other.Index;
            }

            public override bool Equals(object obj)
            {
                return obj is MethodSlotKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return (MethodToken * 397) ^ Index;
                }
            }
        }

        private sealed class BoundMaps
        {
            public readonly Dictionary<int, LuaMarshalAsXmlRule> ByMemberToken =
                new Dictionary<int, LuaMarshalAsXmlRule>();
            public readonly Dictionary<MethodSlotKey, LuaMarshalAsXmlRule> ByMethodSlot =
                new Dictionary<MethodSlotKey, LuaMarshalAsXmlRule>();
        }

        private static readonly object s_gate = new object();
        private static List<LuaMarshalAsXmlRule> s_rules = new List<LuaMarshalAsXmlRule>();
        private static Dictionary<string, List<LuaMarshalAsXmlRule>> s_rulesByAssemblyName =
            new Dictionary<string, List<LuaMarshalAsXmlRule>>(StringComparer.Ordinal);
        /// <summary>Key present (even empty maps) means this Assembly was already initialized.</summary>
        private static Dictionary<Assembly, BoundMaps> s_boundByAssembly =
            new Dictionary<Assembly, BoundMaps>();
        private static bool s_loaded;

        public static bool IsLoaded
        {
            get
            {
                lock (s_gate)
                {
                    return s_loaded;
                }
            }
        }

        public static IReadOnlyList<LuaMarshalAsXmlRule> Rules
        {
            get
            {
                lock (s_gate)
                {
                    return s_rules;
                }
            }
        }

        public static void Clear()
        {
            lock (s_gate)
            {
                s_rules = new List<LuaMarshalAsXmlRule>();
                s_rulesByAssemblyName = new Dictionary<string, List<LuaMarshalAsXmlRule>>(StringComparer.Ordinal);
                s_boundByAssembly = new Dictionary<Assembly, BoundMaps>();
                s_loaded = false;
            }
        }

        /// <summary>
        /// Parse XML only (duplicate-key checks). Metadata bind + validation are lazy per Assembly.
        /// </summary>
        public static void Load(IEnumerable<string> configuredPaths, string projectRoot)
        {
            List<LuaMarshalAsXmlRule> rules = LuaMarshalAsXmlLoader.LoadFromConfiguredPaths(configuredPaths, projectRoot);
            var byName = new Dictionary<string, List<LuaMarshalAsXmlRule>>(StringComparer.Ordinal);
            for (int i = 0; i < rules.Count; i++)
            {
                LuaMarshalAsXmlRule rule = rules[i];
                if (!byName.TryGetValue(rule.AssemblyName, out List<LuaMarshalAsXmlRule> list))
                {
                    list = new List<LuaMarshalAsXmlRule>();
                    byName[rule.AssemblyName] = list;
                }

                list.Add(rule);
            }

            lock (s_gate)
            {
                s_rules = rules;
                s_rulesByAssemblyName = byName;
                s_boundByAssembly = new Dictionary<Assembly, BoundMaps>();
                s_loaded = true;
            }
        }

        /// <summary>
        /// Eagerly bind and validate every assembly named in loaded rules (Editor Generate / tests).
        /// </summary>
        public static void ValidateAllLoadedAssemblies()
        {
            lock (s_gate)
            {
                if (!s_loaded)
                {
                    return;
                }

                foreach (KeyValuePair<string, List<LuaMarshalAsXmlRule>> pair in s_rulesByAssemblyName)
                {
                    Assembly assembly = FindAssembly(pair.Key);
                    if (assembly == null)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML Assembly '" + pair.Key + "' not loaded (validation).");
                    }

                    EnsureBoundUnlocked(assembly);
                }
            }
        }

        /// <summary>
        /// Force bind+validate for one assembly (tests / diagnostics).
        /// </summary>
        public static void EnsureBound(Assembly assembly)
        {
            if (assembly == null)
            {
                return;
            }

            lock (s_gate)
            {
                EnsureBoundUnlocked(assembly);
            }
        }

        public static bool TryGetTypeRule(Type type, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            Type keyType = UnwrapType(type);
            if (keyType == null || keyType.IsGenericType)
            {
                return false;
            }

            return TryGetMember(keyType.Assembly, keyType.MetadataToken, out rule);
        }

        public static bool TryGetFieldRule(FieldInfo field, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            if (field == null)
            {
                return false;
            }

            return TryGetMember(field.Module.Assembly, field.MetadataToken, out rule);
        }

        public static bool TryGetPropertyRule(PropertyInfo property, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            if (property == null)
            {
                return false;
            }

            return TryGetMember(property.Module.Assembly, property.MetadataToken, out rule);
        }

        public static bool TryGetParameterRule(ParameterInfo parameter, MethodBase method, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            if (parameter == null)
            {
                return false;
            }

            MethodBase defMethod = ToDefinitionMethod(method ?? parameter.Member as MethodBase);
            if (defMethod == null)
            {
                return false;
            }

            return TryGetMethodSlot(defMethod.Module.Assembly, defMethod.MetadataToken, parameter.Position, out rule);
        }

        public static bool TryGetReturnRule(MethodInfo method, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            MethodBase defMethod = ToDefinitionMethod(method);
            if (defMethod == null)
            {
                return false;
            }

            return TryGetMethodSlot(defMethod.Module.Assembly, defMethod.MetadataToken, -1, out rule);
        }

        public static bool IsDeterminedMarshalTargetType(Type type)
        {
            type = UnwrapType(type);
            if (type == null)
            {
                return false;
            }

            return !type.IsGenericParameter && !type.ContainsGenericParameters;
        }

        public static Type UnwrapType(Type type)
        {
            if (type == null)
            {
                return null;
            }

            if (type.IsByRef)
            {
                type = type.GetElementType();
            }

            Type underlying = Nullable.GetUnderlyingType(type);
            return underlying ?? type;
        }

        public static MethodBase ToDefinitionMethod(MethodBase method)
        {
            if (method == null)
            {
                return null;
            }

            if (method is MethodInfo methodInfo
                && methodInfo.IsGenericMethod
                && !methodInfo.IsGenericMethodDefinition)
            {
                method = methodInfo.GetGenericMethodDefinition();
            }

            Type declaring = method.DeclaringType;
            if (declaring != null && declaring.IsGenericType && !declaring.IsGenericTypeDefinition)
            {
                try
                {
                    return declaring.Module.ResolveMethod(method.MetadataToken);
                }
                catch (Exception)
                {
                    return method;
                }
            }

            return method;
        }

        public static string FormatMethodSignature(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            if (parameters.Length == 0)
            {
                return "()";
            }

            var parts = new string[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                parts[i] = FormatTypeName(parameters[i].ParameterType);
            }

            return "(" + string.Join(",", parts) + ")";
        }

        public static string FormatTypeName(Type type)
        {
            if (type == null)
            {
                return "System.Void";
            }

            if (type.IsByRef)
            {
                return FormatTypeName(type.GetElementType()) + "&";
            }

            if (type.IsArray)
            {
                Type element = type.GetElementType();
                int rank = type.GetArrayRank();
                if (rank == 1 && type.Name.EndsWith("[]", StringComparison.Ordinal))
                {
                    return FormatTypeName(element) + "[]";
                }

                return FormatTypeName(element) + "[" + new string(',', rank - 1) + "]";
            }

            if (type.IsGenericParameter)
            {
                return type.Name;
            }

            return type.FullName ?? type.Name;
        }

        private static bool TryGetMember(Assembly assembly, int token, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            if (assembly == null || token == 0)
            {
                return false;
            }

            lock (s_gate)
            {
                BoundMaps maps = EnsureBoundUnlocked(assembly);
                return maps.ByMemberToken.TryGetValue(token, out rule);
            }
        }

        private static bool TryGetMethodSlot(Assembly assembly, int methodToken, int index, out LuaMarshalAsXmlRule rule)
        {
            rule = null;
            if (assembly == null || methodToken == 0)
            {
                return false;
            }

            lock (s_gate)
            {
                BoundMaps maps = EnsureBoundUnlocked(assembly);
                return maps.ByMethodSlot.TryGetValue(new MethodSlotKey(methodToken, index), out rule);
            }
        }

        private static BoundMaps EnsureBoundUnlocked(Assembly assembly)
        {
            if (s_boundByAssembly.TryGetValue(assembly, out BoundMaps existing))
            {
                return existing;
            }

            string assemblyName = assembly.GetName()?.Name;
            var maps = new BoundMaps();
            if (assemblyName != null
                && s_rulesByAssemblyName.TryGetValue(assemblyName, out List<LuaMarshalAsXmlRule> rules)
                && rules != null)
            {
                for (int i = 0; i < rules.Count; i++)
                {
                    BindRule(rules[i], assembly, maps);
                }
            }

            s_boundByAssembly[assembly] = maps;
            return maps;
        }

        private static void BindRule(LuaMarshalAsXmlRule rule, Assembly expectedAssembly, BoundMaps maps)
        {
            Type type = ResolveTypeOnAssembly(expectedAssembly, rule.TypeFullName, rule.SourcePath);
            if (type.IsGenericType && !type.IsGenericTypeDefinition)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs XML Type must be an open generic definition or non-generic type, not a closed instance: "
                    + rule.TypeFullName + " in " + rule.SourcePath);
            }

            switch (rule.Kind)
            {
                case LuaMarshalAsXmlTargetKind.Type:
                    if (type.IsGenericTypeDefinition || type.IsGenericType)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML type-level rule cannot target a generic type: "
                            + rule.TypeFullName + " in " + rule.SourcePath);
                    }

                    PutMember(maps, type.MetadataToken, rule);
                    break;

                case LuaMarshalAsXmlTargetKind.Field:
                {
                    FieldInfo field = type.GetField(
                        rule.MemberName,
                        BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
                    if (field == null)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML Field '" + rule.MemberName + "' not found on "
                            + type.FullName + " in " + rule.SourcePath);
                    }

                    EnsureDeterminedTarget(field.FieldType, "Field '" + rule.MemberName + "'", rule);
                    PutMember(maps, field.MetadataToken, rule);
                    break;
                }

                case LuaMarshalAsXmlTargetKind.Property:
                {
                    PropertyInfo property = type.GetProperty(
                        rule.MemberName,
                        BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
                    if (property == null)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML Property '" + rule.MemberName + "' not found on "
                            + type.FullName + " in " + rule.SourcePath);
                    }

                    EnsureDeterminedTarget(property.PropertyType, "Property '" + rule.MemberName + "'", rule);
                    PutMember(maps, property.MetadataToken, rule);
                    break;
                }

                case LuaMarshalAsXmlTargetKind.Param:
                {
                    MethodBase method = ResolveMethod(type, rule.MethodName, rule.Signature, rule.SourcePath);
                    ParameterInfo[] parameters = method.GetParameters();
                    int index = rule.ParamIndex;
                    if (index < 0 || index >= parameters.Length)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML Param index " + index + " out of range for "
                            + type.FullName + "." + rule.MethodName + rule.Signature
                            + " in " + rule.SourcePath);
                    }

                    EnsureDeterminedTarget(
                        parameters[index].ParameterType,
                        "Param index " + index + " of " + rule.MethodName + rule.Signature,
                        rule);
                    PutMethodSlot(maps, method.MetadataToken, index, rule);
                    break;
                }

                case LuaMarshalAsXmlTargetKind.Return:
                {
                    MethodInfo methodInfo = ResolveMethod(type, rule.MethodName, rule.Signature, rule.SourcePath) as MethodInfo;
                    if (methodInfo == null)
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] MarshalAs XML Return requires a MethodInfo (not constructor): "
                            + type.FullName + "." + rule.MethodName + " in " + rule.SourcePath);
                    }

                    EnsureDeterminedTarget(
                        methodInfo.ReturnType,
                        "Return of " + rule.MethodName + rule.Signature,
                        rule);
                    PutMethodSlot(maps, methodInfo.MetadataToken, -1, rule);
                    break;
                }

                default:
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] Unknown MarshalAs XML target kind in " + rule.SourcePath);
            }
        }

        private static void EnsureDeterminedTarget(Type clrType, string targetDescription, LuaMarshalAsXmlRule rule)
        {
            if (IsDeterminedMarshalTargetType(clrType))
            {
                return;
            }

            throw new LuaMarshalAsConfigurationException(
                "[ZLua] MarshalAs XML cannot target undetermined generic type (" + targetDescription + "): "
                + (clrType != null ? (clrType.FullName ?? clrType.Name) : "<null>")
                + " in " + rule.SourcePath);
        }

        private static void PutMember(BoundMaps maps, int token, LuaMarshalAsXmlRule rule)
        {
            if (maps.ByMemberToken.ContainsKey(token))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs XML duplicate metadata token 0x" + token.ToString("X8")
                    + " while binding " + rule.SourcePath);
            }

            maps.ByMemberToken[token] = rule;
        }

        private static void PutMethodSlot(BoundMaps maps, int methodToken, int index, LuaMarshalAsXmlRule rule)
        {
            var key = new MethodSlotKey(methodToken, index);
            if (maps.ByMethodSlot.ContainsKey(key))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs XML duplicate method slot (token=0x" + methodToken.ToString("X8")
                    + ", index=" + index + ") while binding " + rule.SourcePath);
            }

            maps.ByMethodSlot[key] = rule;
        }

        private static Type ResolveTypeOnAssembly(Assembly assembly, string typeFullName, string sourcePath)
        {
            Type type = assembly.GetType(typeFullName, throwOnError: false, ignoreCase: false);
            if (type == null)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs XML Type '" + typeFullName + "' not found in assembly '"
                    + assembly.GetName().Name + "' (" + sourcePath + ")");
            }

            return type;
        }

        private static Assembly FindAssembly(string assemblyName)
        {
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            for (int i = 0; i < assemblies.Length; i++)
            {
                AssemblyName name = assemblies[i].GetName();
                if (name != null && string.Equals(name.Name, assemblyName, StringComparison.Ordinal))
                {
                    return assemblies[i];
                }
            }

            return null;
        }

        private static MethodBase ResolveMethod(Type type, string methodName, string signature, string sourcePath)
        {
            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic
                                       | BindingFlags.Instance | BindingFlags.Static
                                       | BindingFlags.DeclaredOnly;

            MethodInfo[] methods = type.GetMethods(flags);
            for (int i = 0; i < methods.Length; i++)
            {
                MethodInfo method = methods[i];
                if (!string.Equals(method.Name, methodName, StringComparison.Ordinal))
                {
                    continue;
                }

                if (string.Equals(FormatMethodSignature(method), signature, StringComparison.Ordinal))
                {
                    return method;
                }
            }

            if (string.Equals(methodName, ".ctor", StringComparison.Ordinal))
            {
                ConstructorInfo[] ctors = type.GetConstructors(flags);
                for (int i = 0; i < ctors.Length; i++)
                {
                    if (string.Equals(FormatMethodSignature(ctors[i]), signature, StringComparison.Ordinal))
                    {
                        return ctors[i];
                    }
                }
            }

            throw new LuaMarshalAsConfigurationException(
                "[ZLua] MarshalAs XML Method '" + methodName + signature + "' not found on "
                + type.FullName + " in " + sourcePath);
        }
    }
}
