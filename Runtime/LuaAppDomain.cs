using System;
using System.Collections.Generic;

namespace NextLua
{
    public static class LuaAppDomain
    {
        private static LuaEnv _luaEnv;
        private static LuaManagerObject _managerObject;

        public static LuaEnv LuaEnv
        {
            get
            {
                if (_luaEnv == null)
                {
                    throw new InvalidOperationException("NextLua is not initialized. Call LuaAppDomain.Initialize first.");
                }

                return _luaEnv;
            }
        }

        public static void Initialize(Func<string, string> moduleLoader)
        {
            if (_luaEnv != null)
            {
                _luaEnv.SetModuleLoader(moduleLoader);
                return;
            }

            _luaEnv = new LuaEnv();
            _luaEnv.SetModuleLoader(moduleLoader);
            _managerObject = new LuaManagerObject(_luaEnv);
            _luaEnv.LoadBuiltinGlobals();
            _managerObject.RegisterNextLuaApi();
            _luaEnv.LoadBuiltinNextLuaLib();
        }

        public static void Shutdown()
        {
            if (_luaEnv == null)
            {
                return;
            }

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

        public static T RunLuaFunc<T>(string moduleName, string methodName, params object[] args)
        {
            EnsureInitialized();
            return _luaEnv.RunLuaFunc<T>(moduleName, methodName, args);
        }

        private static void EnsureInitialized()
        {
            if (_luaEnv == null)
            {
                throw new InvalidOperationException("NextLua is not initialized. Call LuaAppDomain.Initialize first.");
            }
        }
    }
}
