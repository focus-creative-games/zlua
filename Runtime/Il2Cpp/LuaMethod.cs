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
