using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Resolves <see cref="MethodInfo"/> for <see cref="LuaInvokeAttribute"/> sites (module + Lua function name).
    /// Used when woven IL does not pass MethodInfo, and for marshal attribute lookup.
    /// </summary>
    internal static class LuaInvokeMethodRegistry
    {
        private static readonly object Sync = new object();
        private static readonly Dictionary<string, MethodInfo> Cache = new Dictionary<string, MethodInfo>(StringComparer.Ordinal);

        internal static MethodInfo Resolve(string moduleName, string methodName)
        {
            if (string.IsNullOrEmpty(moduleName) || string.IsNullOrEmpty(methodName))
            {
                return null;
            }

            string key = BuildKey(moduleName, methodName);
            lock (Sync)
            {
                if (Cache.TryGetValue(key, out MethodInfo cached))
                {
                    return cached;
                }
            }

            MethodInfo resolved = ScanForMethod(moduleName, methodName);
            lock (Sync)
            {
                Cache[key] = resolved;
                return resolved;
            }
        }

        private static MethodInfo ScanForMethod(string moduleName, string methodName)
        {
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            for (int i = 0; i < assemblies.Length; i++)
            {
                MethodInfo method = ScanAssembly(assemblies[i], moduleName, methodName);
                if (method != null)
                {
                    return method;
                }
            }

            return null;
        }

        private static MethodInfo ScanAssembly(Assembly assembly, string moduleName, string methodName)
        {
            if (assembly == null || assembly.IsDynamic)
            {
                return null;
            }

            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (ReflectionTypeLoadException ex)
            {
                types = ex.Types;
            }

            if (types == null)
            {
                return null;
            }

            const BindingFlags flags = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
            for (int i = 0; i < types.Length; i++)
            {
                Type type = types[i];
                if (type == null)
                {
                    continue;
                }

                MethodInfo[] methods;
                try
                {
                    methods = type.GetMethods(flags);
                }
                catch
                {
                    continue;
                }

                for (int j = 0; j < methods.Length; j++)
                {
                    MethodInfo method = methods[j];
                    LuaInvokeAttribute invokeAttribute = method.GetCustomAttribute<LuaInvokeAttribute>(inherit: false);
                    if (invokeAttribute == null)
                    {
                        continue;
                    }

                    if (string.Equals(invokeAttribute.Module, moduleName, StringComparison.Ordinal)
                        && string.Equals(invokeAttribute.Function, methodName, StringComparison.Ordinal))
                    {
                        return method;
                    }
                }
            }

            return null;
        }

        private static string BuildKey(string moduleName, string methodName)
        {
            return moduleName + "::" + methodName;
        }
    }
}
