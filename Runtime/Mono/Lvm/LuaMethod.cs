using System;
using ZLua.Lvm;

namespace ZLua
{
    public sealed class LuaMethod : IDisposable
    {
        private bool _disposed;
        private readonly LuaEnv _env;

        public IntPtr LuaState
        {
            get
            {
                if (_env == null || !_env.IsAlive)
                {
                    throw new InvalidOperationException(
                        "ZLua domain was Reset; discard old GetFunction delegates and re-bind.");
                }

                return _env.L;
            }
        }

        public int RefIndex { get; }

        internal LuaEnv Env => _env;

        internal void PushErrorHandlerToStack()
        {
            _env.PushErrorHandler(LuaState);
        }

        internal LuaMethod(LuaEnv env, int refIndex)
        {
            _env = env;
            RefIndex = refIndex;
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

        private void Dispose(bool disposing)
        {
            if (_disposed)
            {
                return;
            }

            _disposed = true;
            if (_env != null && _env.IsAlive)
            {
                _env.AddPendingRef(RefIndex);
            }
        }
    }
}
