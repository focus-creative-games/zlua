using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using UnityEngine.Scripting;

namespace ZLua
{
    public static class LuaIl2CppAppDomain
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeInternal(Func<string, object> moduleLoader);

        public static void Initialize(Func<string, object> moduleLoader)
        {
            InitializeInternal(moduleLoader);
        }

        internal static void ProcessPendingRefReleases()
        {
        }
    }
}
