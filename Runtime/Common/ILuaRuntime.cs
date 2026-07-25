using System;

namespace ZLua
{
    /// <summary>
    /// Backend contract for <see cref="LuaAppDomain"/>. Implemented by Mono / Il2Cpp assemblies
    /// and registered via <see cref="LuaAppDomain.SetRuntime"/> (no Common→backend reference).
    /// </summary>
    public interface ILuaRuntime
    {
        void Initialize(Func<string, object> moduleLoader);

        void ProcessPendingRefReleases();

        /// <summary>
        /// Bind Lua module function to a closed <paramref name="delegateType"/> instance.
        /// </summary>
        Delegate GetFunction(Type delegateType, string luaModule, string luaMethodName);
    }
}
