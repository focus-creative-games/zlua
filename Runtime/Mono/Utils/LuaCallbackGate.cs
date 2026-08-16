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
using System.Runtime.InteropServices;

namespace ZLua.Utils
{
    /// <summary>
    /// Editor Mono: route Lua→C# callbacks through a native gate so
    /// <c>lua_error</c> never runs on a managed reverse-P/Invoke frame
    /// (LuaJIT SEH; Unity Mono stack-crawl / longjmp edge cases on PUC-Rio).
    /// <see cref="ZLua.Mono"/> is Editor-only, so this applies to all Lua series in Editor.
    /// </summary>
    internal static class LuaCallbackGate
    {
        private const string GateDll = "zlua_mono_gate";

        /// <summary>
        /// Managed returns this instead of calling <c>lua_error</c>; gate then throws.
        /// Must match <c>ZLUA_CALLBACK_ERROR_SENTINEL</c> in <c>zlua_mono_gate.c</c>.
        /// </summary>
        internal const int ErrorSentinel = unchecked((int)0xFFFF5A11);

        private static bool s_ready;
        private static IntPtr s_gateFn;

        [DllImport(GateDll, CallingConvention = CallingConvention.Cdecl)]
        private static extern void zlua_gate_init(IntPtr touserdata, IntPtr luaError, int upvaluePseudoBase);

        [DllImport(GateDll, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr zlua_get_callback_gate();

        internal static bool IsEnabled => true;

        /// <summary>
        /// While gated, managed bodies see upvalues of the gate CClosure:
        /// uv1 = managed fn lightuserdata, uv2.. = original upvalues.
        /// </summary>
        internal static int ManagedUpvalueIndex(int logicalIndex) => logicalIndex + 1;

        /// <summary>
        /// <c>lua_getupvalue</c> slot for the first logical upvalue on a gated closure.
        /// </summary>
        internal static int FirstLogicalUpvalueSlot => 2;

        /// <summary>
        /// Pseudo-index base for <c>lua_upvalueindex</c> (5.1/JIT: globals; 5.2+: registry).
        /// </summary>
        private static int UpvaluePseudoBase
        {
            get
            {
#if ZLUA_USE_LUAJIT || ZLUA_LUA_5_1
                return LuaConsts.LuaGlobalsIndex;
#else
                return LuaConsts.LuaRegistryIndex;
#endif
            }
        }

        internal static void EnsureInitialized()
        {
            if (s_ready)
            {
                return;
            }

            IntPtr touserdata = NativeExport.Find("lua_touserdata");
            IntPtr luaError = NativeExport.Find("lua_error");
            if (touserdata == IntPtr.Zero || luaError == IntPtr.Zero)
            {
                throw new InvalidOperationException(
                    "LuaCallbackGate: lua_touserdata / lua_error not found in process — load Lua DLL first (" +
                    LuaDll.LUA_DLL + ").");
            }

            zlua_gate_init(touserdata, luaError, UpvaluePseudoBase);
            s_gateFn = zlua_get_callback_gate();
            if (s_gateFn == IntPtr.Zero)
            {
                throw new InvalidOperationException("LuaCallbackGate: zlua_get_callback_gate returned null");
            }

            s_ready = true;
        }

        internal static void PushCFunction(IntPtr L, IntPtr managedFn)
        {
            EnsureInitialized();
            LuaDll.lua_pushlightuserdata(L, managedFn);
            LuaDll.lua_pushcclosure(L, s_gateFn, 1);
        }

        internal static void PushCClosure(IntPtr L, IntPtr managedFn, int nLogicalUpvalues)
        {
            EnsureInitialized();
            if (nLogicalUpvalues < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(nLogicalUpvalues));
            }

            LuaDll.lua_pushlightuserdata(L, managedFn);
            if (nLogicalUpvalues > 0)
            {
                LuaDll.lua_insert(L, -(nLogicalUpvalues + 1));
            }

            LuaDll.lua_pushcclosure(L, s_gateFn, nLogicalUpvalues + 1);
        }

        private static class NativeExport
        {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
            [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
            private static extern IntPtr GetModuleHandle(string lpModuleName);

            [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
            private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

            internal static IntPtr Find(string symbol)
            {
                string baseName = LuaDll.LUA_DLL;
                string[] candidates =
                {
                    baseName,
                    baseName + ".dll",
                    "lib" + baseName,
                    "lib" + baseName + ".dll",
                };

                for (int i = 0; i < candidates.Length; i++)
                {
                    IntPtr module = GetModuleHandle(candidates[i]);
                    if (module == IntPtr.Zero)
                    {
                        continue;
                    }

                    IntPtr proc = GetProcAddress(module, symbol);
                    if (proc != IntPtr.Zero)
                    {
                        return proc;
                    }
                }

                return IntPtr.Zero;
            }
#elif UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
            private static readonly IntPtr RTLD_DEFAULT = (IntPtr)(-2);

            [DllImport("libdl.dylib", EntryPoint = "dlsym")]
            private static extern IntPtr dlsym(IntPtr handle, string symbol);

            internal static IntPtr Find(string symbol) => dlsym(RTLD_DEFAULT, symbol);
#else
            private static readonly IntPtr RTLD_DEFAULT = IntPtr.Zero;

            [DllImport("libdl.so.2", EntryPoint = "dlsym")]
            private static extern IntPtr dlsym(IntPtr handle, string symbol);

            internal static IntPtr Find(string symbol) => dlsym(RTLD_DEFAULT, symbol);
#endif
        }
    }
}
