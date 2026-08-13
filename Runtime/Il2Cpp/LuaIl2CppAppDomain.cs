using System;
using System.Runtime.CompilerServices;

namespace ZLua
{
    /// <summary>
    /// Player Il2Cpp backend entry. Invoked by <see cref="LuaAppDomain"/> via reflective
    /// construction of nested <see cref="Runtime"/> (Editor uses Mono instead).
    /// </summary>
    public static class LuaIl2CppAppDomain
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeInternal(Func<string, object> moduleLoader);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ResetInternal(Func<string, object> moduleLoader);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ProcessPendingRefReleases();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Delegate GetFunctionInternal(Type delegateType, string luaModule, string luaMethodName);

        public static void Initialize(Func<string, object> moduleLoader)
        {
            InitializeInternal(moduleLoader);
        }

        public static void Reset(Func<string, object> moduleLoader)
        {
            ResetInternal(moduleLoader);
        }

        private static Delegate GetFunction(Type delegateType, string luaModule, string luaMethodName)
        {
            if (delegateType == null)
            {
                throw new ArgumentNullException(nameof(delegateType));
            }

            if (!typeof(MulticastDelegate).IsAssignableFrom(delegateType))
            {
                throw new ArgumentException($"Type '{delegateType.FullName}' is not a MulticastDelegate.", nameof(delegateType));
            }

            return GetFunctionInternal(delegateType, luaModule, luaMethodName);
        }

        private sealed class Runtime : ILuaRuntime
        {
            public void Initialize(Func<string, object> moduleLoader)
            {
                LuaIl2CppAppDomain.Initialize(moduleLoader);
            }

            public void Reset(Func<string, object> moduleLoader)
            {
                LuaIl2CppAppDomain.Reset(moduleLoader);
            }

            public void ProcessPendingRefReleases()
            {
                LuaIl2CppAppDomain.ProcessPendingRefReleases();
            }

            public Delegate GetFunction(Type delegateType, string luaModule, string luaMethodName)
            {
                return LuaIl2CppAppDomain.GetFunction(delegateType, luaModule, luaMethodName);
            }
        }
    }
}
