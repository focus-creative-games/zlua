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
                throw new InvalidOperationException(
                    "ZLua is already initialized. Call LuaAppDomain.Reset to rebuild the Lua domain.");
            }

            CreateEnv(moduleLoader);
        }

        public static void Reset(Func<string, object> moduleLoader)
        {
            ShutdownState();
            CreateEnv(moduleLoader);
        }

        private static void CreateEnv(Func<string, object> moduleLoader)
        {
            _luaEnv = new LuaEnv();
            _luaEnv.SetModuleLoader(moduleLoader);
            _luaEnv.LoadBuiltinGlobals();
#if UNITY_EDITOR
            LoadMarshalAsXmlFromSettings();
            LoadLuaAliasXmlFromSettings();
            LoadLuaExtensionXmlFromSettings();
#endif
            AssemblyRegistry.EnsureCSharpRoot();
            ZLuaLib.RegisterGlobals(_luaEnv);
            _luaEnv.EnsureBuiltinZLuaLib();
            DelegateBridges.Warmup();
#if UNITY_EDITOR
            TryStartEmmyDebuggerFromSettings();
#endif
        }

        private static void ShutdownState()
        {
            if (_luaEnv == null)
            {
                return;
            }

            ProcessPendingRefReleases();
            _luaEnv.Dispose();
            _luaEnv = null;
            LuaCallbackBoundary.ResetDepth();
        }

#if UNITY_EDITOR
        private static void LoadMarshalAsXmlFromSettings()
        {
            try
            {
                if (!EditorSettingsAccess.TryGetInstance(out object settings, out Type settingsType))
                {
                    return;
                }

                EditorSettingsAccess.TryGetField(settings, settingsType, "marshalAsXmlPaths", out string[] paths);
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaMarshalAsXmlRegistry.Load(paths, projectRoot);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] MarshalAs XML load failed at Initialize:\n" + ex.Message);
                throw;
            }
        }

        private static void LoadLuaAliasXmlFromSettings()
        {
            try
            {
                if (!EditorSettingsAccess.TryGetInstance(out object settings, out Type settingsType))
                {
                    return;
                }

                EditorSettingsAccess.TryGetField(settings, settingsType, "luaAliasXmlPaths", out string[] paths);
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaAliasXmlRegistry.Load(paths, projectRoot);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] LuaAlias XML load failed at Initialize:\n" + ex.Message);
                throw;
            }
        }

        private static void LoadLuaExtensionXmlFromSettings()
        {
            try
            {
                if (!EditorSettingsAccess.TryGetInstance(out object settings, out Type settingsType))
                {
                    return;
                }

                EditorSettingsAccess.TryGetField(settings, settingsType, "luaExtensionXmlPaths", out string[] paths);
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaExtensionXmlRegistry.Load(paths, projectRoot);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] LuaExtension XML load failed at Initialize:\n" + ex.Message);
                throw;
            }
        }

        private static void TryStartEmmyDebuggerFromSettings()
        {
            try
            {
                if (!EditorSettingsAccess.TryGetInstance(out object settings, out Type settingsType))
                {
                    return;
                }

                if (!EditorSettingsAccess.TryGetField(settings, settingsType, "enableDebugger", out bool enable) || !enable)
                {
                    return;
                }

                int port = 9966;
                if (EditorSettingsAccess.TryGetField(settings, settingsType, "debuggerPort", out int configuredPort)
                    && configuredPort > 0
                    && configuredPort <= 65535)
                {
                    port = configuredPort;
                }

                EditorSettingsAccess.TryGetField(settings, settingsType, "debuggerWaitIDE", out bool waitIde);
                _luaEnv.StartDebugger(port, waitIde);
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] EmmyLua debugger settings read failed:\n" + ex.Message);
            }
        }
#endif

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

            public void Reset(Func<string, object> moduleLoader)
            {
                LuaMonoAppDomain.Reset(moduleLoader);
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
