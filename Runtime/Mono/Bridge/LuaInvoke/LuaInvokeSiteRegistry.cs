using System;
using ZLua.Lvm;

namespace ZLua.Bridge
{
    /// <summary>
    /// Resolves Lua registry refs for woven <c>[LuaInvoke]</c> sites.
    /// </summary>
    public static class LuaInvokeSiteRegistry
    {
        public static int GetOrCreateFunctionRef(string moduleName, string functionName)
        {
            if (string.IsNullOrWhiteSpace(moduleName))
            {
                throw new ArgumentException("moduleName is required.", nameof(moduleName));
            }

            if (string.IsNullOrWhiteSpace(functionName))
            {
                throw new ArgumentException("functionName is required.", nameof(functionName));
            }

            try
            {
                LuaEnv env = LuaMonoAppDomain.LuaEnv;
                env.EnsureModuleLoaded(moduleName);
                return env.GetOrCreateModuleFunctionRef(moduleName, functionName);
            }
            catch (Exception)
            {
                // Site cctor may resolve every [LuaInvoke] on the type. Throwing here while
                // inside a Lua→C# callback SIGSEGVs on Unity Mono (exception stack walk).
                if (StructOpaqueScope.IsInsideLuaToCSharp)
                {
                    return LuaConsts.LuaNoRef;
                }

                throw;
            }
        }
    }
}
