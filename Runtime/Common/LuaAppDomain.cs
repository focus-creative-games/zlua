using System;
using System.Linq;
using System.Reflection;
using UnityEngine;

namespace ZLua
{
    public class LuaAppDomain
    {
        private static Action _processPendingRefReleases;

        public static void Initialize(Func<string, object> moduleLoader)
        {
            MethodInfo initializeMethod = ResolveBackendMethod("Initialize", BindingFlags.Public | BindingFlags.Static);
            initializeMethod?.Invoke(null, new object[] { moduleLoader });

            MethodInfo processPendingMethod = ResolveBackendMethod(
                nameof(ProcessPendingRefReleases),
                BindingFlags.NonPublic | BindingFlags.Static);
            _processPendingRefReleases = (Action)Delegate.CreateDelegate(typeof(Action), processPendingMethod);

            LuaFramePump.EnsureRegistered();
        }

        internal static void ProcessPendingRefReleases()
        {
            _processPendingRefReleases();
        }

        private static MethodInfo ResolveBackendMethod(string methodName, BindingFlags bindingFlags)
        {
            string assemblyName = Application.isEditor ? "ZLua.Mono" : "ZLua.Il2Cpp";
            Assembly assembly = AppDomain.CurrentDomain.GetAssemblies()
                .FirstOrDefault(a => string.Equals(a.GetName().Name, assemblyName, StringComparison.Ordinal));
            string typeName = Application.isEditor ? "ZLua.LuaMonoAppDomain" : "ZLua.LuaIl2CppAppDomain";
            return assembly.GetType(typeName)?.GetMethod(methodName, bindingFlags);
        }
    }
}
