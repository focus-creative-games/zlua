using System;
using System.IO;
using UnityEngine;
using ZLua.DelegateImpl;
using ZLua.Lvm;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua
{
    /// <summary>
    /// Editor Mono backend entry. Registers into <see cref="LuaAppDomain"/> at subsystem startup.
    /// </summary>
    public static class LuaMonoAppDomain
    {
        private static LuaEnv _luaEnv;

        public static LuaEnv LuaEnv
        {
            get
            {
                if (_luaEnv == null)
                {
                    throw new InvalidOperationException("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
                }

                return _luaEnv;
            }
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
        private static void RegisterRuntimeEarly() => EnsureRuntimeRegistered();

        // Batchmode -executeMethod may compile packages after SubsystemRegistration already ran.
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterAssembliesLoaded)]
        private static void RegisterRuntimeAfterAssemblies() => EnsureRuntimeRegistered();

        /// <summary>Idempotent Editor backend registration (safe to call from batch smoke).</summary>
        public static void EnsureRuntimeRegistered()
        {
            LuaAppDomain.SetRuntime(new Runtime());
        }

        public static void Initialize(Func<string, object> moduleLoader)
        {
            if (_luaEnv != null)
            {
                _luaEnv.SetModuleLoader(moduleLoader);
                _luaEnv.EnsureBuiltinZLuaLib();
                return;
            }

            _luaEnv = new LuaEnv();
            _luaEnv.SetModuleLoader(moduleLoader);
            _luaEnv.LoadBuiltinGlobals();
            LuaMarshalAsValidation.ReportInvalidConfiguration = LuaPrintBuffer.EnqueueEditorError;
#if UNITY_EDITOR
            // Settings-backed XML; Editor bootstrap also loads on domain reload.
            try
            {
                Type settingsType = Type.GetType("ZLua.Settings, ZLua.Editor");
                if (settingsType != null)
                {
                    var instanceProp = settingsType.GetProperty("Instance", System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
                    object settings = instanceProp?.GetValue(null);
                    var pathsField = settingsType.GetField("marshalAsXmlPaths");
                    string[] paths = pathsField?.GetValue(settings) as string[];
                    string projectRoot = Path.GetDirectoryName(Application.dataPath);
                    LuaMarshalAsXmlRegistry.Load(paths, projectRoot);
                }
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] MarshalAs XML load failed at Initialize:\n" + ex.Message);
                throw;
            }
#endif
            AssemblyRegistry.EnsureCSharpRoot();
            ZLuaLib.RegisterGlobals(_luaEnv);
            _luaEnv.EnsureBuiltinZLuaLib();
            DelegateBridges.Warmup();
        }

        public static void Shutdown()
        {
            if (_luaEnv == null)
            {
                return;
            }

            ProcessPendingRefReleases();
            _luaEnv.Dispose();
            _luaEnv = null;
        }

        private static void ProcessPendingRefReleases()
        {
            _luaEnv?.ProcessPendingRefReleases();
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

            LuaEnv env = LuaEnv;
            env.EnsureModuleLoaded(luaModule);
            int sharedRef = env.GetOrCreateModuleFunctionRef(luaModule, luaMethodName);

            // Own a duplicate registry ref so LuaMethod dispose cannot unref the module cache entry.
            IntPtr L = env.L;
            int oldTop = LuaDll.lua_gettop(L);
            try
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, sharedRef);
                if (LuaDll.lua_type(L, -1) != LuaDataType.Function)
                {
                    throw new Exception($"Lua function '{luaModule}.{luaMethodName}' not found.");
                }

                int ownedRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
                return LuaDelegateBinder.Create(env, delegateType, ownedRef);
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
            }
        }

        private sealed class Runtime : ILuaRuntime
        {
            public void Initialize(Func<string, object> moduleLoader)
            {
                LuaMonoAppDomain.Initialize(moduleLoader);
            }

            public void ProcessPendingRefReleases()
            {
                LuaMonoAppDomain.ProcessPendingRefReleases();
            }

            public Delegate GetFunction(Type delegateType, string luaModule, string luaMethodName)
            {
                return LuaMonoAppDomain.GetFunction(delegateType, luaModule, luaMethodName);
            }
        }
    }
}
