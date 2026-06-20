using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UnityEngine;

namespace NextLua
{
    public sealed class LuaEnv : IDisposable
    {
        private static readonly StringBuilder SharedBuilder = new StringBuilder();

        private readonly List<int> _freePendingRefs = new List<int>();
        private readonly Dictionary<string, int> _moduleRefs = new Dictionary<string, int>();
        private readonly Dictionary<string, int> _moduleFunctionRefs = new Dictionary<string, int>();
        private bool _disposed;

        private Func<string, string> _moduleLoader;

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
            // LoadBuiltinNextLuaLib();
        }

        public void SetModuleLoader(Func<string, string> moduleLoader)
        {
            _moduleLoader = moduleLoader;
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

            string source = _moduleLoader(moduleName);
            if (string.IsNullOrEmpty(source))
            {
                throw new Exception($"Lua module '{moduleName}' cannot be loaded.");
            }

            int oldTop = LuaDll.lua_gettop(LuaState);
            try
            {
                int loadResult = LuaDllExtension.loadstring(LuaState, source);
                if (loadResult != 0)
                {
                    string error = LuaDllExtension.tostring(LuaState, -1);
                    throw new Exception($"Error loading lua module '{moduleName}': {error}");
                }

                int execResult = LuaDll.lua_pcall(LuaState, 0, 1, 0);
                if (execResult != 0)
                {
                    string error = LuaDllExtension.tostring(LuaState, -1);
                    throw new Exception($"Error executing lua module '{moduleName}': {error}");
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
            LuaDll.lua_getglobal(LuaState, "__nextluaErrorHandler");
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
                LuaDll.lua_getglobal(LuaState, "__nextluaErrorHandler");
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

        public void LoadBuiltinGlobals()
        {
            TextAsset globals = Resources.Load<TextAsset>("nextlua/globals.lua");
            string source = globals != null ? globals.text : null;
            if (string.IsNullOrWhiteSpace(source))
            {
                throw new Exception("nextlua built-in globals not found: Resources/nextlua/globals.lua or Packages/com.code-philosophy.nextlua/Resources/nextlua/globals.lua");
            }
            DoStringIgnoreResult(source);
        }

        public void LoadBuiltinNextLuaLib()
        {
            TextAsset lib = Resources.Load<TextAsset>("nextlua/nextlualib.lua");
            string source = lib != null ? lib.text : null;
            if (string.IsNullOrWhiteSpace(source))
            {
                throw new Exception("nextlua built-in library not found: Resources/nextlua/nextlualib.lua or Packages/com.code-philosophy.nextlua/Resources/nextlua/nextlualib.lua");
            }

            DoStringIgnoreResult(source);
        }

        [LuaCallback]
        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int Print(IntPtr luaState)
        {
            int count = LuaDll.lua_gettop(luaState);
            SharedBuilder.Clear();
            SharedBuilder.Append("[NextLua] ");
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

            _disposed = true;
            GC.SuppressFinalize(this);
        }

        ~LuaEnv()
        {
            Dispose();
        }
    }
}
