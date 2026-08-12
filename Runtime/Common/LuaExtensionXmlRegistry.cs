using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// LuaExtension XML registry: load rules; resolve extension classes on Bind (spec 13 §2).
    /// </summary>
    public static class LuaExtensionXmlRegistry
    {
        private static readonly object s_gate = new object();
        private static List<LuaExtensionXmlRule> s_rules = new List<LuaExtensionXmlRule>();
        private static Dictionary<string, List<LuaExtensionXmlRule>> s_rulesByTargetKey =
            new Dictionary<string, List<LuaExtensionXmlRule>>(StringComparer.Ordinal);
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

        public static IReadOnlyList<LuaExtensionXmlRule> Rules
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
                s_rules = new List<LuaExtensionXmlRule>();
                s_rulesByTargetKey = new Dictionary<string, List<LuaExtensionXmlRule>>(StringComparer.Ordinal);
                s_loaded = false;
            }
        }

        public static void Load(IEnumerable<string> configuredPaths, string projectRoot)
        {
            List<LuaExtensionXmlRule> rules = LuaExtensionXmlLoader.LoadFromConfiguredPaths(configuredPaths, projectRoot);
            var byKey = new Dictionary<string, List<LuaExtensionXmlRule>>(StringComparer.Ordinal);
            for (int i = 0; i < rules.Count; i++)
            {
                LuaExtensionXmlRule rule = rules[i];
                string key = TargetKey(rule.TargetAssemblyName, rule.TargetTypeFullName);
                if (!byKey.TryGetValue(key, out List<LuaExtensionXmlRule> list))
                {
                    list = new List<LuaExtensionXmlRule>();
                    byKey[key] = list;
                }

                list.Add(rule);
            }

            lock (s_gate)
            {
                s_rules = rules;
                s_rulesByTargetKey = byKey;
                s_loaded = true;
            }
        }

        /// <summary>
        /// Generate-time hard validation: target + extension types must resolve (spec 13).
        /// </summary>
        public static void ValidateAllLoadedAssemblies()
        {
            List<LuaExtensionXmlRule> rules;
            lock (s_gate)
            {
                if (!s_loaded)
                {
                    return;
                }

                rules = new List<LuaExtensionXmlRule>(s_rules);
            }

            for (int i = 0; i < rules.Count; i++)
            {
                LuaExtensionXmlRule rule = rules[i];
                ResolveTypeOnAssembly(rule.TargetAssemblyName, rule.TargetTypeFullName, "target", rule.SourcePath);
                ResolveExtensionType(rule);
            }
        }

        /// <summary>
        /// Resolves extension classes configured for <paramref name="targetType"/> (exact type only).
        /// Unresolvable extension class → hard failure.
        /// </summary>
        public static bool TryGetExtensionTypes(Type targetType, out Type[] extensionTypes)
        {
            extensionTypes = Array.Empty<Type>();
            if (targetType == null)
            {
                return false;
            }

            string assemblyName = targetType.Assembly.GetName()?.Name;
            string typeFullName = targetType.FullName;
            if (string.IsNullOrEmpty(assemblyName) || string.IsNullOrEmpty(typeFullName))
            {
                return false;
            }

            List<LuaExtensionXmlRule> rules;
            lock (s_gate)
            {
                if (!s_loaded
                    || !s_rulesByTargetKey.TryGetValue(TargetKey(assemblyName, typeFullName), out rules)
                    || rules == null
                    || rules.Count == 0)
                {
                    return false;
                }

                // Copy under lock so resolution can run outside.
                rules = new List<LuaExtensionXmlRule>(rules);
            }

            var resolved = new List<Type>(rules.Count);
            var seen = new HashSet<Type>();
            for (int i = 0; i < rules.Count; i++)
            {
                Type extType = ResolveExtensionType(rules[i]);
                if (seen.Add(extType))
                {
                    resolved.Add(extType);
                }
            }

            extensionTypes = resolved.ToArray();
            return extensionTypes.Length > 0;
        }

        private static Type ResolveTypeOnAssembly(string assemblyName, string typeFullName, string role, string sourcePath)
        {
            Assembly assembly = FindAssembly(assemblyName);
            if (assembly == null)
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] LuaExtension XML " + role + " Assembly '" + assemblyName
                    + "' not loaded (" + sourcePath + ")");
            }

            Type type = assembly.GetType(typeFullName, throwOnError: false, ignoreCase: false);
            if (type == null)
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] LuaExtension XML " + role + " type '" + typeFullName
                    + "' not found in assembly '" + assemblyName
                    + "' (" + sourcePath + ")");
            }

            return type;
        }

        private static Type ResolveExtensionType(LuaExtensionXmlRule rule)
        {
            return ResolveTypeOnAssembly(
                rule.ExtensionAssemblyName, rule.ExtensionTypeFullName, "extension", rule.SourcePath);
        }

        private static string TargetKey(string assemblyName, string typeFullName)
        {
            return assemblyName + "|" + typeFullName;
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
