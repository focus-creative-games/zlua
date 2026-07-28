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
    /// Editor Mono backend entry. Invoked by <see cref="LuaAppDomain"/> via reflective
    /// construction of nested <see cref="Runtime"/>.
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
            LoadMarshalAsXmlFromSettings();
#endif
            AssemblyRegistry.EnsureCSharpRoot();
            ZLuaLib.RegisterGlobals(_luaEnv);
            _luaEnv.EnsureBuiltinZLuaLib();
            DelegateBridges.Warmup();
#if UNITY_EDITOR
            TryStartEmmyDebuggerFromSettings();
#endif
        }

#if UNITY_EDITOR
        private const System.Reflection.BindingFlags StaticPublic =
            System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static;

        /// <summary>
        /// <see cref="ZLua.Settings"/> lives in ZLua.Editor; Mono resolves it by reflection to avoid an Editor asmref cycle.
        /// </summary>
        private static bool TryGetEditorSettings(out object settings, out Type settingsType)
        {
            settings = null;
            settingsType = Type.GetType("ZLua.Settings, ZLua.Editor");
            if (settingsType == null)
            {
                return false;
            }

            var instanceProp = settingsType.GetProperty("Instance", StaticPublic);
            settings = instanceProp?.GetValue(null);
            return settings != null;
        }

        private static bool TryGetSettingsField<T>(object settings, Type settingsType, string fieldName, out T value)
        {
            value = default;
            var field = settingsType.GetField(fieldName);
            if (field == null)
            {
                return false;
            }

            object raw = field.GetValue(settings);
            if (raw is T typed)
            {
                value = typed;
                return true;
            }

            return false;
        }

        private static void LoadMarshalAsXmlFromSettings()
        {
            try
            {
                if (!TryGetEditorSettings(out object settings, out Type settingsType))
                {
                    return;
                }

                TryGetSettingsField(settings, settingsType, "marshalAsXmlPaths", out string[] paths);
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaMarshalAsXmlRegistry.Load(paths, projectRoot);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] MarshalAs XML load failed at Initialize:\n" + ex.Message);
                throw;
            }
        }

        private static void TryStartEmmyDebuggerFromSettings()
        {
            try
            {
                if (!TryGetEditorSettings(out object settings, out Type settingsType))
                {
                    return;
                }

                if (!TryGetSettingsField(settings, settingsType, "enableDebugger", out bool enable) || !enable)
                {
                    return;
                }

                int port = 9966;
                if (TryGetSettingsField(settings, settingsType, "debuggerPort", out int configuredPort)
                    && configuredPort > 0
                    && configuredPort <= 65535)
                {
                    port = configuredPort;
                }

                TryGetSettingsField(settings, settingsType, "debuggerWaitIDE", out bool waitIde);
                _luaEnv.StartDebugger(port, waitIde);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] EmmyLua debugger settings read failed:\n" + ex.Message);
            }
        }
#endif

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
