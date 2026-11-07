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
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using ZLua.Lvm;

namespace ZLua.Utils
{
    /// <summary>
    /// Lua <c>print</c> 在 native 回调里不能直接 <see cref="Debug.Log"/>（Editor 在活跃
    /// <c>lua_pcall</c> 时抓堆栈可能 SIGSEGV）。回调入队，最外层托管 pcall 返回后刷出。
    /// Mono gate 只解决 <c>lua_error</c> 不落在托管 reverse-P/Invoke 帧上，不替代本缓冲。
    /// </summary>
    internal static class LuaPrintBuffer
    {
        private static readonly List<string> PendingLines = new List<string>();
        private static readonly object Sync = new object();
        private static int _managedPcallDepth;

        public static void EnterManagedPcall()
        {
            _managedPcallDepth++;
        }

        public static void LeaveManagedPcall()
        {
            if (_managedPcallDepth <= 0)
            {
                return;
            }

            _managedPcallDepth--;
            if (_managedPcallDepth == 0)
            {
                FlushCore();
            }
        }

        /// <summary>
        /// Current nested managed-pcall depth (CallScope / LuaEnv DoString, etc.).
        /// </summary>
        internal static int ManagedPcallDepth => _managedPcallDepth;

        /// <summary>
        /// True when this CallScope sits inside another managed pcall (Lua→C#→C#→Lua).
        /// </summary>
        internal static bool IsNestedManagedPcall => _managedPcallDepth > 1;

        public static bool TryEnqueueFromLuaPrint(IntPtr luaState)
        {
            int count = LuaDll.lua_gettop(luaState);
            StringBuilder msg = new StringBuilder();

            for (int i = 1; i <= count; i++)
            {
                if (i > 1)
                {
                    msg.Append('\t');
                }

                // __tostring must run below a native protected-call boundary.
                // On failure leave its error object on top for the callback gate.
                if (!LuaDllExtension.TryPushDisplayString(luaState, i))
                {
                    return false;
                }
                try
                {
                    msg.Append(LuaDllExtension.tostring(luaState, -1));
                }
                finally
                {
                    LuaDll.lua_pop(luaState, 1);
                }
            }

            // Avoid mutable debug/global-table lookups in the Lua 5.1 traceback shim.
            // The extra native debug.traceback frame requires level 2 to reach print's caller.
            LuaEnv.Active.PushPrintTracebackFunction(luaState);
            LuaDll.lua_pushstring(luaState, msg.ToString());
            LuaDll.lua_pushinteger(luaState, 2);
            if (LuaDll.lua_pcall(luaState, 2, 1, 0) != 0)
            {
                return false;
            }
            string traceback = LuaDllExtension.tostring(luaState, -1) ?? msg.ToString();
            LuaDll.lua_pop(luaState, 1);

            lock (Sync)
            {
                PendingLines.Add("[ZLua] " + traceback);
            }
            return true;
        }

        public static void ForceFlushAll()
        {
            FlushCore();
        }

        private static void FlushCore()
        {
            string[] lines;
            lock (Sync)
            {
                lines = PendingLines.Count == 0 ? Array.Empty<string>() : PendingLines.ToArray();
                PendingLines.Clear();
            }

            for (int i = 0; i < lines.Length; i++)
            {
                Debug.Log(lines[i]);
            }
        }
    }
}
