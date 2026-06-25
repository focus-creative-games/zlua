using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

namespace ZLua
{
    public sealed class LuaEnv : IDisposable
    {
        private static readonly StringBuilder SharedBuilder = new StringBuilder();
        private static readonly List<LuaCSFunction> ModuleLoaderCallbackRefs = new List<LuaCSFunction>();
        private static readonly LuaCSFunction LoadModuleCallback = LoadModule;
        private static LuaEnv s_activeEnv;

        private readonly List<int> _freePendingRefs = new List<int>();
        private readonly Dictionary<string, int> _moduleRefs = new Dictionary<string, int>();
        private readonly Dictionary<string, int> _moduleFunctionRefs = new Dictionary<string, int>();
        private bool _disposed;
        private bool _moduleLoaderHooksInstalled;

        private Func<string, object> _moduleLoader;

        public IntPtr LuaState { get; private set; }

        public LuaEnv()
        {
            LuaState = LuaDll.luaL_newstate();
            if (LuaState == IntPtr.Zero)
            {
                throw new Exception("Failed to create Lua state.");
            }

            LuaDll.luaL_openlibs(LuaState);
            RegisterCallbacks();
            // LoadBuiltinZLuaLib();
        }

        public void SetModuleLoader(Func<string, object> moduleLoader)
        {
            _moduleLoader = moduleLoader;
            s_activeEnv = this;
            if (moduleLoader != null)
            {
                InstallModuleLoaderHooks();
            }
        }

        public void EnsureModuleLoaded(string moduleName)
        {
            if (string.IsNullOrWhiteSpace(moduleName) || _moduleRefs.ContainsKey(moduleName))
            {
                return;
            }

            if (_moduleLoader == null)
            {
                throw new InvalidOperationException("Lua module loader is not configured.");
            }

            int oldTop = LuaDll.lua_gettop(LuaState);
            try
            {
                LuaDataType requireType = LuaDll.lua_getglobal(LuaState, "require");
                if (requireType != LuaDataType.Function)
                {
                    throw new InvalidOperationException("Lua global 'require' is not available.");
                }

                LuaDll.lua_pushstring(LuaState, moduleName);
                int execResult = LuaDll.lua_pcall(LuaState, 1, 1, 0);
                if (execResult != 0)
                {
                    string error = LuaDllExtension.tostring(LuaState, -1);
                    throw new Exception($"Error requiring lua module '{moduleName}': {error}");
                }

                LuaDataType returnType = LuaDll.lua_type(LuaState, -1);
                if (returnType != LuaDataType.Table)
                {
                    throw new Exception($"Lua module '{moduleName}' must return a table.");
                }

                LuaDll.lua_pushvalue(LuaState, -1);
                int moduleRef = LuaDll.luaL_ref(LuaState, LuaConsts.LuaRegistryIndex);
                _moduleRefs[moduleName] = moduleRef;
            }
            finally
            {
                LuaDll.lua_settop(LuaState, oldTop);
            }
        }

        public void RunLuaFunc(string moduleName, string methodName, object[] args = null)
        {
            RunLuaFuncInternal(moduleName, methodName, typeof(void), args);
        }

        public T RunLuaFunc<T>(string moduleName, string methodName, object[] args = null)
        {
            object result = RunLuaFuncInternal(moduleName, methodName, typeof(T), args);
            return result == null ? default : (T)result;
        }

        public LuaMethod GetFunction(string globalName)
        {
            LuaDataType type = LuaDll.lua_getglobal(LuaState, globalName);
            if (type != LuaDataType.Function)
            {
                throw new Exception($"Global '{globalName}' is not a function.");
            }

            int refIndex = LuaDll.luaL_ref(LuaState, LuaConsts.LuaRegistryIndex);
            if (refIndex < 0)
            {
                throw new Exception($"Failed to create lua function reference: {globalName}");
            }

            return new LuaMethod(this, refIndex);
        }

        public void AddPendingRef(int refIndex)
        {
            lock (_freePendingRefs)
            {
                _freePendingRefs.Add(refIndex);
            }
        }

        public void ClearPendingRefs()
        {
            lock (_freePendingRefs)
            {
                foreach (int refIndex in _freePendingRefs)
                {
                    LuaDll.luaL_unref(LuaState, LuaConsts.LuaRegistryIndex, refIndex);
                }

                _freePendingRefs.Clear();
            }
        }

        public void ProcessPendingRefReleases()
        {
            ClearPendingRefs();
        }

        public void DoStringIgnoreResult(string chunk)
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            int result = LuaDllExtension.dostring(LuaState, chunk);
            if (result != 0)
            {
                string error = LuaDllExtension.tostring(LuaState, -1);
                LuaDll.lua_settop(LuaState, oldTop);
                throw new Exception(error);
            }

            LuaDll.lua_settop(LuaState, oldTop);
        }

        public void LoadLuaChunk(string source, string chunkName = "chunk")
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            int loadResult = LuaDllExtension.loadstring(LuaState, source);
            if (loadResult != 0)
            {
                string error = LuaDllExtension.tostring(LuaState, -1);
                LuaDll.lua_settop(LuaState, oldTop);
                throw new Exception($"Error loading lua chunk '{chunkName}': {error}");
            }

            int execResult = LuaDll.lua_pcall(LuaState, 0, LuaConsts.LuaMultiRet, 0);
            if (execResult != 0)
            {
                string error = LuaDllExtension.tostring(LuaState, -1);
                LuaDll.lua_settop(LuaState, oldTop);
                throw new Exception($"Error executing lua chunk '{chunkName}': {error}");
            }

            LuaDll.lua_settop(LuaState, oldTop);
        }

        public void CallLuaGlobal(string functionName, Action<IntPtr> pushArgs = null)
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            LuaDll.lua_getglobal(LuaState, "__zluaErrorHandler");
            LuaDataType type = LuaDll.lua_getglobal(LuaState, functionName);
            if (type != LuaDataType.Function)
            {
                LuaDll.lua_settop(LuaState, oldTop);
                throw new Exception($"Lua function '{functionName}' not found.");
            }

            pushArgs?.Invoke(LuaState);
            int nArgs = LuaDll.lua_gettop(LuaState) - oldTop - 2;
            int err = LuaDll.lua_pcall(LuaState, nArgs, 0, oldTop + 1);
            if (err != 0)
            {
                string error = LuaDllExtension.tostring(LuaState, -1);
                LuaDll.lua_settop(LuaState, oldTop);
                throw new Exception(error);
            }

            LuaDll.lua_settop(LuaState, oldTop);
        }

        private object RunLuaFuncInternal(string moduleName, string methodName, Type returnType, object[] args)
        {
            EnsureModuleLoaded(moduleName);
            int oldTop = LuaDll.lua_gettop(LuaState);
            string key = moduleName + "::" + methodName;

            try
            {
                LuaDll.lua_getglobal(LuaState, "__zluaErrorHandler");
                int functionRef = GetOrCreateModuleFunctionRef(moduleName, methodName, key);
                LuaDll.lua_rawgeti(LuaState, LuaConsts.LuaRegistryIndex, functionRef);

                if (args != null)
                {
                    for (int i = 0; i < args.Length; i++)
                    {
                        LuaMarshal.PushObject(LuaState, args[i]);
                    }
                }

                int nArgs = args?.Length ?? 0;
                int nRet = returnType == typeof(void) ? 0 : 1;
                int err = LuaDll.lua_pcall(LuaState, nArgs, nRet, oldTop + 1);
                if (err != 0)
                {
                    string error = LuaDllExtension.tostring(LuaState, -1);
                    throw new Exception(error);
                }

                if (returnType == typeof(void))
                {
                    return null;
                }

                return LuaMarshal.PopObject(LuaState, returnType, -1);
            }
            finally
            {
                LuaDll.lua_settop(LuaState, oldTop);
                ProcessPendingRefReleases();
            }
        }

        private int GetOrCreateModuleFunctionRef(string moduleName, string methodName, string key)
        {
            if (_moduleFunctionRefs.TryGetValue(key, out int cachedRef))
            {
                return cachedRef;
            }

            int moduleRef = _moduleRefs[moduleName];
            int oldTop = LuaDll.lua_gettop(LuaState);
            try
            {
                LuaDll.lua_rawgeti(LuaState, LuaConsts.LuaRegistryIndex, moduleRef);
                LuaDataType fieldType = LuaDll.lua_getfield(LuaState, -1, methodName);
                if (fieldType != LuaDataType.Function)
                {
                    throw new Exception($"Lua function '{moduleName}.{methodName}' not found.");
                }

                int functionRef = LuaDll.luaL_ref(LuaState, LuaConsts.LuaRegistryIndex);
                _moduleFunctionRefs[key] = functionRef;
                return functionRef;
            }
            finally
            {
                LuaDll.lua_settop(LuaState, oldTop);
            }
        }

        private void RegisterCallbacks()
        {
            LuaDllExtension.RegisterCallback(LuaState, "print", Print);
        }

        private void InstallModuleLoaderHooks()
        {
            if (_moduleLoaderHooksInstalled)
            {
                return;
            }

            ModuleLoaderCallbackRefs.Add(LoadModuleCallback);
            LuaDllExtension.RegisterCallback(LuaState, "__zlua_load_module", LoadModuleCallback);

            const string installSearcherChunk = @"
local function zlua_module_searcher(modname)
    local src = __zlua_load_module(modname)
    if src == nil then
        return nil
    end
    local chunk, err = load(src, '@' .. modname:gsub('%.', '/') .. '.lua')
    if not chunk then
        error(err, 2)
    end
    return chunk
end

table.insert(package.searchers, 2, zlua_module_searcher)
";
            DoStringIgnoreResult(installSearcherChunk);
            _moduleLoaderHooksInstalled = true;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int LoadModule(IntPtr luaState)
        {
            LuaEnv env = s_activeEnv;
            if (env == null || env._moduleLoader == null)
            {
                LuaDll.lua_pushnil(luaState);
                return 1;
            }

            string moduleName = LuaDllExtension.tostring(luaState, 1);
            if (string.IsNullOrWhiteSpace(moduleName))
            {
                LuaDll.lua_pushnil(luaState);
                return 1;
            }

            object loaded;
            try
            {
                loaded = env._moduleLoader(moduleName);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"moduleLoader error for '{moduleName}': {ex.Message}");
            }

            if (loaded == null)
            {
                LuaDll.lua_pushnil(luaState);
                return 1;
            }

            if (loaded is string source)
            {
                if (string.IsNullOrEmpty(source))
                {
                    LuaDll.lua_pushnil(luaState);
                    return 1;
                }

                LuaDll.lua_pushstring(luaState, source);
                return 1;
            }

            if (loaded is byte[] bytes)
            {
                if (bytes.Length == 0)
                {
                    LuaDll.lua_pushnil(luaState);
                    return 1;
                }

                LuaDll.lua_pushstring(luaState, Encoding.UTF8.GetString(bytes));
                return 1;
            }

            return LuaDllExtension.error(luaState,
                $"moduleLoader returned unsupported type '{loaded.GetType().FullName}' for '{moduleName}'");
        }

        public void LoadBuiltinGlobals()
        {
            DoStringIgnoreResult(ZLuaBuiltinScriptLoader.Load("globals.lua"));
        }

        public void LoadBuiltinZLuaLib()
        {
            DoStringIgnoreResult(ZLuaBuiltinScriptLoader.Load("zlualib.lua"));
        }

        public void EnsureBuiltinZLuaLib()
        {
            if (HasZLuaTypesTable())
            {
                return;
            }

            LoadBuiltinZLuaLib();
            if (!HasZLuaTypesTable())
            {
                throw new Exception("zlua.types was not initialized after loading zlualib.lua");
            }
        }

        private bool HasZLuaTypesTable()
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            try
            {
                LuaDataType zluaType = LuaDll.lua_getglobal(LuaState, "zlua");
                if (zluaType != LuaDataType.Table)
                {
                    LuaDll.lua_settop(LuaState, oldTop);
                    return false;
                }

                LuaDataType typesType = LuaDll.lua_getfield(LuaState, -1, "types");
                LuaDll.lua_settop(LuaState, oldTop);
                return typesType == LuaDataType.Table;
            }
            finally
            {
                LuaDll.lua_settop(LuaState, oldTop);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int Print(IntPtr luaState)
        {
            int count = LuaDll.lua_gettop(luaState);
            SharedBuilder.Clear();
            SharedBuilder.Append("[ZLua] ");
            for (int i = 1; i <= count; i++)
            {
                if (i > 1)
                {
                    SharedBuilder.Append('\t');
                }

                SharedBuilder.Append(LuaDllExtension.tostring(luaState, i));
            }

            Debug.Log(SharedBuilder.ToString());
            return 0;
        }

        public void Dispose()
        {
            if (_disposed)
            {
                return;
            }

            if (LuaState != IntPtr.Zero)
            {
                ClearPendingRefs();
                foreach (var functionRef in _moduleFunctionRefs.Values)
                {
                    LuaDll.luaL_unref(LuaState, LuaConsts.LuaRegistryIndex, functionRef);
                }
                _moduleFunctionRefs.Clear();

                foreach (var moduleRef in _moduleRefs.Values)
                {
                    LuaDll.luaL_unref(LuaState, LuaConsts.LuaRegistryIndex, moduleRef);
                }
                _moduleRefs.Clear();

                LuaDll.lua_close(LuaState);
                LuaState = IntPtr.Zero;
            }

            if (s_activeEnv == this)
            {
                s_activeEnv = null;
            }

            _disposed = true;
            GC.SuppressFinalize(this);
        }

        ~LuaEnv()
        {
            Dispose();
        }
    }
}
