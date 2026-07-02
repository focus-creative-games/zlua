// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEditor.Compilation;
using ReflectionAssembly = System.Reflection.Assembly;

namespace ZLua.BuildProcessors
{
    internal static class LuaInvokeUserAssemblyResolver
    {
        internal static IReadOnlyList<ReflectionAssembly> ResolvePlayerAssembliesFromAppDomain()
        {
            return ResolveFromAppDomain(AssembliesType.Player);
        }

        private static IReadOnlyList<ReflectionAssembly> ResolveFromAppDomain(AssembliesType assembliesType)
        {
            var wantedNames = new HashSet<string>(StringComparer.Ordinal);
            UnityEditor.Compilation.Assembly[] definitions = CompilationPipeline.GetAssemblies(assembliesType);
            for (int i = 0; i < definitions.Length; i++)
            {
                string name = definitions[i].name;
                if (ShouldSkipAssemblyName(name))
                {
                    continue;
                }

                wantedNames.Add(name);
            }

            var loadedByName = new Dictionary<string, ReflectionAssembly>(StringComparer.Ordinal);
            foreach (ReflectionAssembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (assembly.IsDynamic)
                {
                    continue;
                }

                string name = assembly.GetName().Name;
                if (!wantedNames.Contains(name) || loadedByName.ContainsKey(name))
                {
                    continue;
                }

                loadedByName[name] = assembly;
            }

            var results = new List<ReflectionAssembly>(wantedNames.Count);
            foreach (string name in wantedNames)
            {
                if (loadedByName.TryGetValue(name, out ReflectionAssembly assembly))
                {
                    results.Add(assembly);
                }
            }

            results.Sort((a, b) => string.CompareOrdinal(a.GetName().Name, b.GetName().Name));
            return results;
        }

        private static bool ShouldSkipAssemblyName(string assemblyName)
        {
            if (string.IsNullOrEmpty(assemblyName))
            {
                return true;
            }

            if (assemblyName.StartsWith("Unity.", StringComparison.Ordinal)
                || assemblyName.StartsWith("UnityEngine.", StringComparison.Ordinal)
                || assemblyName.StartsWith("UnityEditor.", StringComparison.Ordinal)
                || assemblyName.StartsWith("System.", StringComparison.Ordinal)
                || string.Equals(assemblyName, "netstandard", StringComparison.Ordinal)
                || string.Equals(assemblyName, "mscorlib", StringComparison.Ordinal)
                || string.Equals(assemblyName, "dnlib", StringComparison.Ordinal))
            {
                return true;
            }

            return false;
        }
    }
}
