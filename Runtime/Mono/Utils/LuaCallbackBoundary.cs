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
using ZLua.Lvm;

namespace ZLua.Utils
{
    /// <summary>
    /// Lua C 回调边界：将托管异常转为 Lua error。
    /// Editor Mono：不直接 <c>lua_error</c>（经 <see cref="LuaCallbackGate"/> sentinel），
    /// 避免 reverse-P/Invoke 帧上展开 / 堆栈抓取导致 SIGSEGV。
    /// </summary>
    internal static class LuaCallbackBoundary
    {
        [ThreadStatic]
        private static int s_callbackDepth;

        internal static bool IsInsideCallback => s_callbackDepth > 0;

        internal static void Enter()
        {
            s_callbackDepth++;
        }

        internal static void Leave()
        {
            if (s_callbackDepth > 0)
            {
                s_callbackDepth--;
            }
        }

        /// <summary>
        /// Clear before error unwind (Mono may skip finally on longjmp/SEH).
        /// </summary>
        internal static void ResetDepth()
        {
            s_callbackDepth = 0;
        }

        internal static int ToLuaError(IntPtr luaState, Exception ex)
        {
            ResetDepth();
            return LuaDllExtension.error(luaState, FormatMessage(ex));
        }

        internal static string FormatMessage(Exception ex)
        {
            ex = Unwrap(ex);
            return ex.Message;
        }

        internal static Exception Unwrap(Exception ex)
        {
            while (ex is TargetInvocationException tie && tie.InnerException != null)
            {
                ex = tie.InnerException;
            }

            return ex;
        }

        /// <summary>
        /// Signal a Lua-facing error. Under Editor Mono + gate, throw so the Lua→C#
        /// entry <c>catch</c> converts via sentinel (never <c>lua_error</c> on this frame).
        /// </summary>
        internal static void Throw(string message)
        {
            throw new LuaScriptException(message);
        }
    }
}
