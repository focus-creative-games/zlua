using System;
using System.Reflection;
using ZLua.DelegateImpl;
using ZLua.Mt;
using ZLua.MethodBridge;
using ZLua.Marshal;

namespace ZLua
{
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

        public static void RegisterType(Type type)
        {
            EnsureInitialized();
            TypeRegistry.RegisterType(_luaEnv, type);
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

        private static void ProcessPendingRefReleases()
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
