using System;
using System.Collections.Generic;
using System.Text;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Lvm
{
    public sealed class LuaEnv : IDisposable
    {
        private static readonly List<LuaCSFunction> s_callbackPins = new List<LuaCSFunction>();
        private static readonly LuaCSFunction s_print = Print;
        private static readonly LuaCSFunction s_loadModule = LoadModule;
        private static LuaEnv s_activeEnv;

        private readonly List<int> _freePendingRefs = new List<int>();
        private readonly Dictionary<string, int> _moduleRefs = new Dictionary<string, int>(StringComparer.Ordinal);
        private readonly Dictionary<string, int> _moduleFunctionRefs = new Dictionary<string, int>(StringComparer.Ordinal);
        private IntPtr _state;
        private Func<string, object> _moduleLoader;
        private bool _disposed;
        private int _errorHandlerRef = LuaConsts.LuaNoRef;
        private bool _printRegistered;
        private bool _moduleLoaderHooksInstalled;

        public IntPtr L
        {
            get
            {
                EnsureNotDisposed();
                if (_state == IntPtr.Zero)
                {
                    throw new InvalidOperationException("Lua state is not created.");
                }

                return _state;
            }
        }

        internal static LuaEnv Active => s_activeEnv;

        internal int ErrorHandlerRef => EnsureErrorHandlerRef();

        public LuaEnv()
        {
            _state = LuaDll.luaL_newstate();
            if (_state == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create lua_State.");
            }

            LuaDll.luaL_openlibs(_state);
            RegisterPrint();
            ObjectRegistry.Initialize(_state);
            StructRegistry.Initialize(_state);
            MetaTableCache.Initialize(_state);
            TypeMemberLuaIndexer.EnsureLoaded(_state);
            s_activeEnv = this;
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

        public Func<string, object> ModuleLoader => _moduleLoader;

        public void LoadBuiltinGlobals()
        {
            DoStringIgnoreResult(BuiltinScripts.Load("globals.lua"));
            EnsureErrorHandlerRef();
        }

        public void LoadBuiltinZLuaLib()
        {
            DoStringIgnoreResult(BuiltinScripts.Load("zlualib.lua"));
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
                throw new InvalidOperationException("zlua.types was not initialized after loading zlualib.lua");
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

            int oldTop = LuaDll.lua_gettop(L);
            LuaPrintBuffer.EnterManagedPcall();
            try
            {
                LuaDataType requireType = LuaDll.lua_getglobal(L, "require");
                if (requireType != LuaDataType.Function)
                {
                    throw new InvalidOperationException("Lua global 'require' is not available.");
                }

                LuaDll.lua_pushstring(L, moduleName);
                int execResult = LuaDll.lua_pcall(L, 1, 1, 0);
                if (execResult != 0)
                {
                    string error = LuaDllExtension.tostring(L, -1);
                    throw new Exception($"Error requiring lua module '{moduleName}': {error}");
                }

                if (LuaDll.lua_type(L, -1) != LuaDataType.Table)
                {
                    throw new Exception($"Lua module '{moduleName}' must return a table.");
                }

                LuaDll.lua_pushvalue(L, -1);
                _moduleRefs[moduleName] = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        public int GetOrCreateModuleFunctionRef(string moduleName, string methodName)
        {
            string key = moduleName + "::" + methodName;
            if (_moduleFunctionRefs.TryGetValue(key, out int cachedRef))
            {
                return cachedRef;
            }

            if (!_moduleRefs.TryGetValue(moduleName, out int moduleRef))
            {
                throw new InvalidOperationException($"Lua module '{moduleName}' is not loaded.");
            }

            int oldTop = LuaDll.lua_gettop(L);
            try
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, moduleRef);
                LuaDataType fieldType = LuaDll.lua_getfield(L, -1, methodName);
                if (fieldType != LuaDataType.Function)
                {
                    throw new Exception($"Lua function '{moduleName}.{methodName}' not found.");
                }

                int functionRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
                _moduleFunctionRefs[key] = functionRef;
                return functionRef;
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
            }
        }

        public void DoStringIgnoreResult(string chunk)
        {
            int oldTop = LuaDll.lua_gettop(L);
            LuaPrintBuffer.EnterManagedPcall();
            try
            {
                int result = LuaDllExtension.dostring(L, chunk);
                if (result != 0)
                {
                    string error = LuaDllExtension.tostring(L, -1);
                    LuaDll.lua_settop(L, oldTop);
                    throw new Exception(error);
                }

                LuaDll.lua_settop(L, oldTop);
            }
            finally
            {
                LuaPrintBuffer.LeaveManagedPcall();
            }
        }

        public void AddPendingRef(int refIndex)
        {
            lock (_freePendingRefs)
            {
                _freePendingRefs.Add(refIndex);
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
                    LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, refIndex);
                }

                _freePendingRefs.Clear();
            }
        }

        internal int EnsureErrorHandlerRef()
        {
            if (_errorHandlerRef != LuaConsts.LuaNoRef)
            {
                return _errorHandlerRef;
            }

            int oldTop = LuaDll.lua_gettop(L);
            try
            {
                LuaDataType handlerType = LuaDll.lua_getglobal(L, "__zluaErrorHandler");
                if (handlerType != LuaDataType.Function)
                {
                    throw new InvalidOperationException("Lua global '__zluaErrorHandler' is not available.");
                }

                _errorHandlerRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
                return _errorHandlerRef;
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
            }
        }

        internal void PushErrorHandler(IntPtr luaState)
        {
            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, EnsureErrorHandlerRef());
        }

        public void Dispose()
        {
            if (_disposed)
            {
                return;
            }

            _disposed = true;
            if (_state != IntPtr.Zero)
            {
                LuaPrintBuffer.ForceFlushAll();
                ProcessPendingRefReleases();

                foreach (int functionRef in _moduleFunctionRefs.Values)
                {
                    LuaDll.luaL_unref(_state, LuaConsts.LuaRegistryIndex, functionRef);
                }

                _moduleFunctionRefs.Clear();

                foreach (int moduleRef in _moduleRefs.Values)
                {
                    LuaDll.luaL_unref(_state, LuaConsts.LuaRegistryIndex, moduleRef);
                }

                _moduleRefs.Clear();

                MetaTableCache.Shutdown(_state);
                StructRegistry.Shutdown(_state);
                ObjectRegistry.Shutdown(_state);

                if (_errorHandlerRef != LuaConsts.LuaNoRef)
                {
                    LuaDll.luaL_unref(_state, LuaConsts.LuaRegistryIndex, _errorHandlerRef);
                    _errorHandlerRef = LuaConsts.LuaNoRef;
                }

                LuaDll.lua_close(_state);
                _state = IntPtr.Zero;
            }

            if (s_activeEnv == this)
            {
                s_activeEnv = null;
            }

            GC.SuppressFinalize(this);
        }

        ~LuaEnv()
        {
            Dispose();
        }

        private bool HasZLuaTypesTable()
        {
            int oldTop = LuaDll.lua_gettop(L);
            try
            {
                LuaDataType zluaType = LuaDll.lua_getglobal(L, "zlua");
                if (zluaType != LuaDataType.Table)
                {
                    return false;
                }

                LuaDataType typesType = LuaDll.lua_getfield(L, -1, "types");
                return typesType == LuaDataType.Table;
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
            }
        }

        private void RegisterPrint()
        {
            if (_printRegistered)
            {
                return;
            }

            s_callbackPins.Add(s_print);
            LuaDllExtension.RegisterCallback(_state, "print", s_print);
            _printRegistered = true;
        }

        private void InstallModuleLoaderHooks()
        {
            if (_moduleLoaderHooksInstalled)
            {
                return;
            }

            s_callbackPins.Add(s_loadModule);
            LuaDllExtension.RegisterCallback(L, "__zlua_load_module", s_loadModule);

            // 5.1: package.loaders + loadstring; 5.2+: package.searchers + load(string).
            const string installSearcherChunk = @"
local function zlua_module_searcher(modname)
    local src = __zlua_load_module(modname)
    if src == nil then
        return nil
    end
    local name = '@' .. modname:gsub('%.', '/') .. '.lua'
    local chunk, err = (loadstring or load)(src, name)
    if not chunk then
        error(err, 2)
    end
    return chunk
end

local searchers = package.searchers or package.loaders
table.insert(searchers, 2, zlua_module_searcher)
";
            DoStringIgnoreResult(installSearcherChunk);
            _moduleLoaderHooksInstalled = true;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int Print(IntPtr luaState)
        {
            LuaPrintBuffer.EnqueueFromLuaPrint(luaState);
            return 0;
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

        private void EnsureNotDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(LuaEnv));
            }
        }
    }
}
