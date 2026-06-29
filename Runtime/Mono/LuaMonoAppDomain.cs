using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    public static class LuaMonoAppDomain
    {
        private static LuaEnv _luaEnv;
        private static LuaManagerObject _managerObject;

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
            _managerObject = new LuaManagerObject(_luaEnv);
            _luaEnv.LoadBuiltinGlobals();
            _managerObject.RegisterZLuaApi();
            _luaEnv.EnsureBuiltinZLuaLib();
            WarmDelegateBridges();
        }

        private static void WarmDelegateBridges()
        {
            // Expression.Compile during native Lua callbacks can SIGSEGV on Unity Editor Mono.
            DynamicBridgeFactory.Warmup(typeof(Action));
            DynamicBridgeFactory.Warmup(typeof(Action<int>));
            DynamicBridgeFactory.Warmup(typeof(Func<int, int>));
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
            _managerObject = null;
        }

        public static void RegisterType(Type type)
        {
            EnsureInitialized();
            _managerObject.RegisterType(type);
        }

        public static void RegisterType<T>()
        {
            RegisterType(typeof(T));
        }

        public static void RunLuaFunc(string moduleName, string methodName, params object[] args)
        {
            EnsureInitialized();
            _luaEnv.RunLuaFunc(moduleName, methodName, args);
        }

        public static void RunLuaFunc(MethodInfo invokeMethod, string moduleName, string methodName, params object[] args)
        {
            EnsureInitialized();
            _luaEnv.RunLuaFunc(invokeMethod, moduleName, methodName, args);
        }

        public static T RunLuaFunc<T>(string moduleName, string methodName, params object[] args)
        {
            EnsureInitialized();
            return _luaEnv.RunLuaFunc<T>(moduleName, methodName, args);
        }

        public static T RunLuaFunc<T>(MethodInfo invokeMethod, string moduleName, string methodName, params object[] args)
        {
            EnsureInitialized();
            return _luaEnv.RunLuaFunc<T>(invokeMethod, moduleName, methodName, args);
        }

        internal static void ProcessPendingRefReleases()
        {
            if (_luaEnv != null)
            {
                _luaEnv.ProcessPendingRefReleases();
            }
        }

        private static void EnsureInitialized()
        {
            if (_luaEnv == null)
            {
                throw new InvalidOperationException("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
            }
        }
    }
}
