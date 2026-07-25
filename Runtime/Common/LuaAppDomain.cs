using System;

namespace ZLua
{
    /// <summary>
    /// Host-facing facade. Concrete work is performed by an <see cref="ILuaRuntime"/>
    /// registered from <c>ZLua.Mono</c> (Editor) or <c>ZLua.Il2Cpp</c> (Player).
    /// </summary>
    public static class LuaAppDomain
    {
        private static ILuaRuntime s_runtime;

        /// <summary>
        /// Called by backend assemblies at <c>SubsystemRegistration</c>.
        /// </summary>
        public static void SetRuntime(ILuaRuntime runtime)
        {
            s_runtime = runtime ?? throw new ArgumentNullException(nameof(runtime));
        }

        public static void Initialize(Func<string, object> moduleLoader)
        {
            EnsureRuntime();
            s_runtime.Initialize(moduleLoader);
            LuaFramePump.EnsureRegistered();
        }

        /// <summary>
        /// Bind a Lua module function to a closed delegate of type <typeparamref name="T"/>.
        /// Must be called after <see cref="Initialize"/>. Does not guarantee instance reuse across calls.
        /// </summary>
        public static T GetFunction<T>(string luaModule, string luaMethodName)
            where T : MulticastDelegate
        {
            EnsureRuntime();
            if (string.IsNullOrEmpty(luaModule))
            {
                throw new ArgumentException("luaModule must be non-empty.", nameof(luaModule));
            }

            if (string.IsNullOrEmpty(luaMethodName))
            {
                throw new ArgumentException("luaMethodName must be non-empty.", nameof(luaMethodName));
            }

            Delegate bound = s_runtime.GetFunction(typeof(T), luaModule, luaMethodName);
            return (T)bound;
        }

        internal static void ProcessPendingRefReleases()
        {
            s_runtime?.ProcessPendingRefReleases();
        }

        private static void EnsureRuntime()
        {
            if (s_runtime == null)
            {
                throw new InvalidOperationException(
                    "ZLua runtime is not registered. Ensure ZLua.Mono (Editor) or ZLua.Il2Cpp (Player) is loaded.");
            }
        }
    }
}
