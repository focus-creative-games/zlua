using System;

namespace NovaLua
{
    public sealed class LuaMethod : IDisposable
    {
        private bool _disposed;
        private readonly LuaEnv _env;

        public IntPtr LuaState => _env.LuaState;

        public int RefIndex { get; }

        internal LuaMethod(LuaEnv env, int refIndex)
        {
            _env = env;
            RefIndex = refIndex;
        }

        public void Dispose()
        {
            if (_disposed)
            {
                return;
            }

            _env.AddPendingRef(RefIndex);
            _disposed = true;
        }
    }
}
