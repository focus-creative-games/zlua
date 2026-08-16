// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Host-facing facade. Concrete work is performed by an <see cref="ILuaRuntime"/>
    /// created on demand from <c>ZLua.Mono</c> (Editor) or <c>ZLua.Il2Cpp</c> (Player).
    /// </summary>
    public static class LuaAppDomain
    {
        private static ILuaRuntime s_runtime;
        private static Func<string, object> s_pendingResetLoader;

        public static void Initialize(Func<string, object> moduleLoader)
        {
            EnsureRuntime();
            s_runtime.Initialize(moduleLoader);
            LuaFramePump.EnsureRegistered();
        }

        /// <summary>
        /// Schedule a rebuild of the single main <c>lua_State</c>.
        /// Teardown / re-init runs at Unity EndOfFrame via <see cref="LuaFramePump"/> (not immediately).
        /// After it applies, prior <see cref="GetFunction{T}"/> delegates are invalid.
        /// </summary>
        public static void Reset(Func<string, object> moduleLoader)
        {
            if (moduleLoader == null)
            {
                throw new ArgumentNullException(nameof(moduleLoader));
            }

            EnsureRuntime();
            s_pendingResetLoader = moduleLoader;
            LuaFramePump.EnsureRegistered();
        }

        /// <summary>
        /// Bind a Lua module function to a closed delegate of type <typeparamref name="T"/>.
        /// Must be called after <see cref="Initialize"/>. Does not guarantee instance reuse across calls.
        /// </summary>
        public static T GetFunction<T>(string luaModule, string luaMethodName)
            where T : MulticastDelegate
        {
            EnsureRuntime();
            if (string.IsNullOrEmpty(luaModule))
            {
                throw new ArgumentException("luaModule must be non-empty.", nameof(luaModule));
            }

            if (string.IsNullOrEmpty(luaMethodName))
            {
                throw new ArgumentException("luaMethodName must be non-empty.", nameof(luaMethodName));
            }

            Delegate bound = s_runtime.GetFunction(typeof(T), luaModule, luaMethodName);
            return (T)bound;
        }

        internal static void ProcessPendingRefReleases()
        {
            s_runtime?.ProcessPendingRefReleases();
        }

        /// <summary>
        /// Apply a scheduled <see cref="Reset"/> if any. Called from <see cref="LuaFramePump"/> at EndOfFrame.
        /// </summary>
        internal static void FlushPendingReset()
        {
            if (s_pendingResetLoader == null || s_runtime == null)
            {
                return;
            }

            Func<string, object> loader = s_pendingResetLoader;
            s_pendingResetLoader = null;
            s_runtime.Reset(loader);
            LuaFramePump.EnsureRegistered();
        }

        private static void EnsureRuntime()
        {
            if (s_runtime != null)
            {
                return;
            }

#if UNITY_EDITOR
            const string hostTypeName = "ZLua.LuaMonoAppDomain, ZLua.Mono";
#else
            const string hostTypeName = "ZLua.LuaIl2CppAppDomain, ZLua.Il2Cpp";
#endif
            Type hostType = Type.GetType(hostTypeName);
            if (hostType == null)
            {
                throw new InvalidOperationException(
                    $"ZLua backend type not found: '{hostTypeName}'. Ensure the matching package assembly is loaded.");
            }

            Type runtimeType = hostType.GetNestedType("Runtime", BindingFlags.NonPublic);
            if (runtimeType == null || !typeof(ILuaRuntime).IsAssignableFrom(runtimeType))
            {
                throw new InvalidOperationException(
                    $"ZLua backend '{hostTypeName}' is missing a non-public nested Runtime : ILuaRuntime.");
            }

            s_runtime = (ILuaRuntime)Activator.CreateInstance(runtimeType, nonPublic: true);
        }
    }
}
