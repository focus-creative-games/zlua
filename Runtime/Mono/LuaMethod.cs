using System;

namespace ZLua
{
    public sealed class LuaMethod : IDisposable
    {
        private bool _disposed;
        private readonly LuaEnv _env;

        public IntPtr LuaState => _env.LuaState;

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

            _env?.AddPendingRef(RefIndex);
            _disposed = true;
        }
    }
}
