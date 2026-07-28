using System;

namespace ZLua
{
    /// <summary>
    /// Backend contract for <see cref="LuaAppDomain"/>. Implemented by nested
    /// <c>Runtime</c> types in Mono / Il2Cpp assemblies; Common creates them by reflection
    /// (no Common→backend compile-time reference).
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
