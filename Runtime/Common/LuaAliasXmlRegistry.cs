using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// LuaAlias XML registry: load by assembly name; bind lazily to MethodInfo metadata token.
    /// </summary>
    public static class LuaAliasXmlRegistry
    {
        private static readonly object s_gate = new object();
        private static List<LuaAliasXmlRule> s_rules = new List<LuaAliasXmlRule>();
        private static Dictionary<string, List<LuaAliasXmlRule>> s_rulesByAssemblyName =
            new Dictionary<string, List<LuaAliasXmlRule>>(StringComparer.Ordinal);
        private static Dictionary<Assembly, Dictionary<int, string>> s_aliasByMethodToken =
            new Dictionary<Assembly, Dictionary<int, string>>();
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

        public static IReadOnlyList<LuaAliasXmlRule> Rules
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
                s_rules = new List<LuaAliasXmlRule>();
                s_rulesByAssemblyName = new Dictionary<string, List<LuaAliasXmlRule>>(StringComparer.Ordinal);
                s_aliasByMethodToken = new Dictionary<Assembly, Dictionary<int, string>>();
                s_loaded = false;
            }
        }

        public static void Load(IEnumerable<string> configuredPaths, string projectRoot)
        {
            List<LuaAliasXmlRule> rules = LuaAliasXmlLoader.LoadFromConfiguredPaths(configuredPaths, projectRoot);
            var byName = new Dictionary<string, List<LuaAliasXmlRule>>(StringComparer.Ordinal);
            for (int i = 0; i < rules.Count; i++)
            {
                LuaAliasXmlRule rule = rules[i];
                if (!byName.TryGetValue(rule.AssemblyName, out List<LuaAliasXmlRule> list))
                {
                    list = new List<LuaAliasXmlRule>();
                    byName[rule.AssemblyName] = list;
                }

                list.Add(rule);
            }

            lock (s_gate)
            {
                s_rules = rules;
                s_rulesByAssemblyName = byName;
                s_aliasByMethodToken = new Dictionary<Assembly, Dictionary<int, string>>();
                s_loaded = true;
            }
        }

        public static void ValidateAllLoadedAssemblies()
        {
            lock (s_gate)
            {
                if (!s_loaded)
                {
                    return;
                }

                foreach (KeyValuePair<string, List<LuaAliasXmlRule>> pair in s_rulesByAssemblyName)
                {
                    Assembly assembly = FindAssembly(pair.Key);
                    if (assembly == null)
                    {
                        throw new LuaAliasConfigurationException(
                            "[ZLua] LuaAlias XML Assembly '" + pair.Key + "' not loaded (validation).");
                    }

                    EnsureBoundUnlocked(assembly);
                }
            }
        }

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

        public static bool TryGetAlias(MethodInfo method, out string alias)
        {
            alias = null;
            MethodBase defMethod = LuaMarshalAsXmlRegistry.ToDefinitionMethod(method);
            if (defMethod == null)
            {
                return false;
            }

            Assembly assembly = defMethod.Module.Assembly;
            int token = defMethod.MetadataToken;
            if (assembly == null || token == 0)
            {
                return false;
            }

            lock (s_gate)
            {
                Dictionary<int, string> map = EnsureBoundUnlocked(assembly);
                return map.TryGetValue(token, out alias);
            }
        }

        private static Dictionary<int, string> EnsureBoundUnlocked(Assembly assembly)
        {
            if (s_aliasByMethodToken.TryGetValue(assembly, out Dictionary<int, string> existing))
            {
                return existing;
            }

            string assemblyName = assembly.GetName()?.Name;
            var map = new Dictionary<int, string>();
            if (assemblyName != null
                && s_rulesByAssemblyName.TryGetValue(assemblyName, out List<LuaAliasXmlRule> rules)
                && rules != null)
            {
                for (int i = 0; i < rules.Count; i++)
                {
                    BindRule(rules[i], assembly, map);
                }
            }

            s_aliasByMethodToken[assembly] = map;
            return map;
        }

        private static void BindRule(LuaAliasXmlRule rule, Assembly expectedAssembly, Dictionary<int, string> map)
        {
            Type type = expectedAssembly.GetType(rule.TypeFullName, throwOnError: false, ignoreCase: false);
            if (type == null)
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] LuaAlias XML Type '" + rule.TypeFullName + "' not found in assembly '"
                    + expectedAssembly.GetName().Name + "' (" + rule.SourcePath + ")");
            }

            if (type.IsGenericType && !type.IsGenericTypeDefinition)
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] LuaAlias XML Type must be an open generic definition or non-generic type: "
                    + rule.TypeFullName + " in " + rule.SourcePath);
            }

            MethodBase method = ResolveMethod(type, rule.MethodName, rule.Signature, rule.SourcePath);
            int token = method.MetadataToken;
            if (map.ContainsKey(token))
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] LuaAlias XML duplicate method token 0x" + token.ToString("X8")
                    + " while binding " + rule.SourcePath);
            }

            map[token] = rule.Alias;
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

                if (string.Equals(LuaMarshalAsXmlRegistry.FormatMethodSignature(method), signature, StringComparison.Ordinal))
                {
                    return method;
                }
            }

            if (string.Equals(methodName, ".ctor", StringComparison.Ordinal))
            {
                ConstructorInfo[] ctors = type.GetConstructors(flags);
                for (int i = 0; i < ctors.Length; i++)
                {
                    if (string.Equals(LuaMarshalAsXmlRegistry.FormatMethodSignature(ctors[i]), signature, StringComparison.Ordinal))
                    {
                        return ctors[i];
                    }
                }
            }

            throw new LuaAliasConfigurationException(
                "[ZLua] LuaAlias XML Method '" + methodName + signature + "' not found on "
                + type.FullName + " in " + sourcePath);
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
    }
}
