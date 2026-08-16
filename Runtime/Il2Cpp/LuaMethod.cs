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

namespace ZLua
{
    public sealed class LuaMethod : IDisposable
    {
        private bool _disposed;
        private readonly IntPtr _L;
        private readonly int _refIndex;
        // Set from native when binding a Lua function to a closed delegate (MethodMarshalCtx*).
        private IntPtr _methodMarshalCtx;

        internal LuaMethod(IntPtr L, int refIndex)
        {
            _L = L;
            _refIndex = refIndex;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ~LuaMethod()
        {
            Dispose(false);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern private static void AddPendingRef(IntPtr L, int refIndex);

        private void Dispose(bool disposing)
        {
            if (_disposed)
            {
                return;
            }
            AddPendingRef(_L, _refIndex);
            _disposed = true;
        }
    }
}
