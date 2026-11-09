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
using System.Runtime.CompilerServices;
using System.Threading;
using UnityEngine;

namespace ZLua
{
    internal static class LuaMainThread
    {
        private static int s_mainThreadId;

        // Unity invokes these callbacks on its main thread. Never infer ownership
        // from the first caller, which could itself be a worker thread.
#if UNITY_EDITOR
        [UnityEditor.InitializeOnLoadMethod]
#endif
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
        private static void CaptureMainThread()
        {
            Volatile.Write(ref s_mainThreadId, Thread.CurrentThread.ManagedThreadId);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void EnsureMainThread()
        {
            if (Thread.CurrentThread.ManagedThreadId != Volatile.Read(ref s_mainThreadId))
            {
                throw new InvalidOperationException("ZLua can only be called from the Unity main thread.");
            }
        }
    }
}
