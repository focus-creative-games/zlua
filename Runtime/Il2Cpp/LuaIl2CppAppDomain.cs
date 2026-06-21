using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace NextLua
{
    public static class LuaIl2CppAppDomain
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeInternal(Func<string, string> moduleLoader);

        public static void Initialize(Func<string, string> moduleLoader)
        {
            InitializeInternal(moduleLoader);
        }
    }
}
