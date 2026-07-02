using System;
using ZLua;
using ZLua.Marshal;
using ZLua.Mt;
using ZLua.DelegateImpl;

namespace ZLua.MethodBridge
{
    /// <summary>
    /// Resolves Lua registry refs for woven <see cref="LuaInvoke"/> sites (called from injected static ctors).
    /// </summary>
    internal static class LuaInvokeSiteRegistry
    {
        internal static int GetOrCreateFunctionRef(string moduleName, string luaMethodName)
        {
            if (string.IsNullOrWhiteSpace(moduleName))
            {
                throw new ArgumentException("moduleName is required.", nameof(moduleName));
            }

            if (string.IsNullOrWhiteSpace(luaMethodName))
            {
                throw new ArgumentException("luaMethodName is required.", nameof(luaMethodName));
            }

            LuaEnv env = LuaMonoAppDomain.LuaEnv;
            env.EnsureModuleLoaded(moduleName);
            return env.GetOrCreateModuleFunctionRef(moduleName, luaMethodName);
        }
    }
}
