using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

namespace ZLua.Utils
{
    /// <summary>
    /// Lua <c>print</c> 在 native 回调里不能直接 <see cref="Debug.Log"/>（Editor 在活跃
    /// <c>lua_pcall</c> 时抓堆栈可能 SIGSEGV）。回调入队，最外层托管 pcall 返回后刷出。
    /// </summary>
    internal static class LuaPrintBuffer
    {
        private static readonly List<string> PendingLines = new List<string>();
        private static readonly List<string> PendingErrors = new List<string>();
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
        /// True when this CallScope sits inside another managed pcall (Lua→C#→LuaInvoke).
        /// </summary>
        internal static bool IsNestedManagedPcall => _managedPcallDepth > 1;

        public static void EnqueueFromLuaPrint(IntPtr luaState)
        {
            int count = LuaDll.lua_gettop(luaState);
            StringBuilder sb = new StringBuilder("[ZLua] ");

            LuaDll.luaL_where(luaState, 1);
            string where = LuaDllExtension.tostring(luaState, -1);
            LuaDll.lua_pop(luaState, 1);
            if (!string.IsNullOrEmpty(where))
            {
                sb.Append(where.Trim());
                sb.Append(' ');
            }

            for (int i = 1; i <= count; i++)
            {
                if (i > 1)
                {
                    sb.Append('\t');
                }

                sb.Append(LuaDllExtension.tostring(luaState, i) ?? "null");
            }

            lock (Sync)
            {
                PendingLines.Add(sb.ToString());
            }
        }

        internal static void EnqueueEditorError(string message)
        {
            if (string.IsNullOrEmpty(message))
            {
                return;
            }

            lock (Sync)
            {
                PendingErrors.Add(message);
            }
        }

        public static void ForceFlushAll()
        {
            FlushCore();
        }

        private static void FlushCore()
        {
            string[] lines;
            string[] errors;
            lock (Sync)
            {
                lines = PendingLines.Count == 0 ? Array.Empty<string>() : PendingLines.ToArray();
                errors = PendingErrors.Count == 0 ? Array.Empty<string>() : PendingErrors.ToArray();
                PendingLines.Clear();
                PendingErrors.Clear();
            }

            for (int i = 0; i < lines.Length; i++)
            {
                Debug.LogFormat(LogType.Log, LogOption.NoStacktrace, null, "{0}", lines[i]);
            }

            for (int i = 0; i < errors.Length; i++)
            {
                Debug.LogFormat(LogType.Error, LogOption.NoStacktrace, null, "{0}", errors[i]);
            }
        }
    }
}
