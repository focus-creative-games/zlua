using System;
using ZLua.DelegateImpl;
using ZLua.Lvm;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua
{
    /// <summary>
    /// Editor Mono backend entry (resolved by <see cref="LuaAppDomain"/>).
    /// Type name kept in root namespace for Common reflection.
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

        private static void EnsureInitialized()
        {
            if (_luaEnv == null)
            {
                throw new InvalidOperationException("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
            }
        }
    }
}
