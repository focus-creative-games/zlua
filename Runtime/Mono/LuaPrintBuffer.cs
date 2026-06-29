using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Lua <c>print</c> 在 native 回调里不能直接 <see cref="Debug.Log"/>（Editor/Mono 在
    /// 仍有活跃 <c>lua_pcall</c> 时抓堆栈可能 SIGSEGV）。
    /// 回调里只入队；仅在最外层 C# 侧 <c>lua_pcall</c> 返回后刷到 Unity Console。
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
            // Debug.LogFormat(LogType.Log, LogOption.NoStacktrace, null, sb.ToString());

            lock (Sync)
            {
                PendingLines.Add(sb.ToString());
            }
        }

        /// <summary>Editor 诊断：Lua 回调内不得直接 Debug.LogError（同 print 的 SIGSEGV 风险）。</summary>
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

        /// <summary>仅当当前不在任何托管 pcall 区间内时刷日志。</summary>
        public static void Flush()
        {
            if (_managedPcallDepth == 0)
            {
                FlushCore();
            }
        }

        /// <summary>关闭 LuaEnv 等场景：无视深度，清空待输出队列。</summary>
        public static void ForceFlushAll()
        {
            _managedPcallDepth = 0;
            FlushCore();
        }

        private static void FlushCore()
        {
            List<string> batch;
            List<string> errorBatch;
            lock (Sync)
            {
                if (PendingLines.Count == 0 && PendingErrors.Count == 0)
                {
                    return;
                }

                batch = PendingLines.Count > 0 ? new List<string>(PendingLines) : null;
                errorBatch = PendingErrors.Count > 0 ? new List<string>(PendingErrors) : null;
                PendingLines.Clear();
                PendingErrors.Clear();
            }

            if (batch != null)
            {
                for (int i = 0; i < batch.Count; i++)
                {
                    Debug.Log(batch[i]);
                }
            }

            if (errorBatch != null)
            {
                for (int i = 0; i < errorBatch.Count; i++)
                {
                    Debug.LogError(errorBatch[i]);
                }
            }
        }
    }
}
