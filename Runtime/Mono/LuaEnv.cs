using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace ZLua
{
    public sealed class LuaEnv : IDisposable
    {
        private static readonly List<LuaCSFunction> ModuleLoaderCallbackRefs = new List<LuaCSFunction>();
        private static readonly LuaCSFunction LoadModuleCallback = LoadModule;
        private static LuaEnv s_activeEnv;

        private readonly List<int> _freePendingRefs = new List<int>();
        private readonly Dictionary<string, int> _moduleRefs = new Dictionary<string, int>();
        private readonly Dictionary<string, int> _moduleFunctionRefs = new Dictionary<string, int>();
        private bool _disposed;
        private bool _moduleLoaderHooksInstalled;
        private int _errorHandlerRef = -1;

        private Func<string, object> _moduleLoader;

        public IntPtr LuaState { get; private set; }

        internal int ErrorHandlerRef => EnsureErrorHandlerRef();

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
            LuaPrintBuffer.EnterManagedPcall();
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
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        public void RunLuaFunc(string moduleName, string methodName, object[] args = null)
        {
            RunLuaFuncInternal(null, moduleName, methodName, typeof(void), args);
        }

        public void RunLuaFunc(MethodInfo invokeMethod, string moduleName, string methodName, object[] args = null)
        {
            RunLuaFuncInternal(invokeMethod, moduleName, methodName, typeof(void), args);
        }

        public T RunLuaFunc<T>(string moduleName, string methodName, object[] args = null)
        {
            object result = RunLuaFuncInternal(null, moduleName, methodName, typeof(T), args);
            return result == null ? default : (T)result;
        }

        public T RunLuaFunc<T>(MethodInfo invokeMethod, string moduleName, string methodName, object[] args = null)
        {
            object result = RunLuaFuncInternal(invokeMethod, moduleName, methodName, typeof(T), args);
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
            lock (_freePendingRefs)
            {
                if (_freePendingRefs.Count == 0)
                {
                    return;
                }

                foreach (int refIndex in _freePendingRefs)
                {
                    LuaDll.luaL_unref(LuaState, LuaConsts.LuaRegistryIndex, refIndex);
                }

                _freePendingRefs.Clear();
            }
        }

        public void DoStringIgnoreResult(string chunk)
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            LuaPrintBuffer.EnterManagedPcall();
            try
            {
                int result = LuaDllExtension.dostring(LuaState, chunk);
                if (result != 0)
                {
                    string error = LuaDllExtension.tostring(LuaState, -1);
                    LuaDll.lua_settop(LuaState, oldTop);
                    throw new Exception(error);
                }

                LuaDll.lua_settop(LuaState, oldTop);
            }
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        public void LoadLuaChunk(string source, string chunkName = "chunk")
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
            LuaPrintBuffer.EnterManagedPcall();
            try
            {
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
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        public void CallLuaGlobal(string functionName, Action<IntPtr> pushArgs = null)
        {
            int oldTop = LuaDll.lua_gettop(LuaState);
                LuaPrintBuffer.EnterManagedPcall();
            try
            {
                PushErrorHandler(LuaState);
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
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        private object RunLuaFuncInternal(MethodInfo invokeMethod, string moduleName, string methodName, Type returnType, object[] args)
        {
            invokeMethod ??= LuaInvokeMethodRegistry.Resolve(moduleName, methodName);

            EnsureModuleLoaded(moduleName);
            int oldTop = LuaDll.lua_gettop(LuaState);
            string key = moduleName + "::" + methodName;

            LuaPrintBuffer.EnterManagedPcall();
            try
            {
                try
                {
                    StructOpaqueScope.EnterStandaloneCSharpToLua();

                    PushErrorHandler(LuaState);
                    int functionRef = GetOrCreateModuleFunctionRef(moduleName, methodName, key);
                    LuaDll.lua_rawgeti(LuaState, LuaConsts.LuaRegistryIndex, functionRef);

                    ParameterInfo[] parameters = invokeMethod?.GetParameters();
                    if (args != null)
                    {
                        for (int i = 0; i < args.Length; i++)
                        {
                            LuaInvokeMarshaling.PushArgument(LuaState, args[i], parameters, invokeMethod, i);
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

                    return LuaInvokeMarshaling.PopReturn(LuaState, invokeMethod, returnType, -1);
                }
                finally
                {
                    LuaDll.lua_settop(LuaState, oldTop);
                }
            }
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        internal int GetOrCreateModuleFunctionRef(string moduleName, string methodName)
        {
            string key = moduleName + "::" + methodName;
            return GetOrCreateModuleFunctionRef(moduleName, methodName, key);
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
            try
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

                object loaded = env._moduleLoader(moduleName);
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

                LuaCallbackBoundary.Throw(
                    $"moduleLoader returned unsupported type '{loaded.GetType().FullName}' for '{moduleName}'");
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        public void LoadBuiltinGlobals()
        {
            DoStringIgnoreResult(ZLuaBuiltinScriptLoader.Load("globals.lua"));
            EnsureErrorHandlerRef();
        }

        internal int EnsureErrorHandlerRef()
        {
            if (_errorHandlerRef >= 0)
            {
                return _errorHandlerRef;
            }

            int oldTop = LuaDll.lua_gettop(LuaState);
            try
            {
                LuaDataType handlerType = LuaDll.lua_getglobal(LuaState, "__zluaErrorHandler");
                if (handlerType != LuaDataType.Function)
                {
                    throw new Exception("Lua global '__zluaErrorHandler' is not available.");
                }

                _errorHandlerRef = LuaDll.luaL_ref(LuaState, LuaConsts.LuaRegistryIndex);
                return _errorHandlerRef;
            }
            finally
            {
                LuaDll.lua_settop(LuaState, oldTop);
            }
        }

        internal void PushErrorHandler(IntPtr luaState)
        {
            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, EnsureErrorHandlerRef());
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
            LuaPrintBuffer.EnqueueFromLuaPrint(luaState);
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
                LuaPrintBuffer.ForceFlushAll();
                ClearPendingRefs();
                if (_errorHandlerRef >= 0)
                {
                    LuaDll.luaL_unref(LuaState, LuaConsts.LuaRegistryIndex, _errorHandlerRef);
                    _errorHandlerRef = -1;
                }
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
