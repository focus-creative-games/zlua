using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Tracks non-blittable ByVal userdata via boxed companions (Mono GC equivalent of Il2Cpp roots).
    /// </summary>
    internal static class StructRegistry
    {
        private static readonly Dictionary<IntPtr, object> s_boxedByUserData = new Dictionary<IntPtr, object>();
        private static readonly LuaCSFunction s_onRelease = OnReleaseByValUserData;
        private static bool s_releasePinned;

        internal static void Initialize(IntPtr L)
        {
            _ = L;
            if (!s_releasePinned)
            {
                GCHandle.Alloc(s_onRelease);
                s_releasePinned = true;
            }
        }

        internal static void Shutdown(IntPtr L)
        {
            _ = L;
            s_boxedByUserData.Clear();
        }

        internal static void RegisterBoxed(IntPtr userdataPtr, object boxed)
        {
            s_boxedByUserData[userdataPtr] = boxed;
        }

        internal static bool TryGetBoxed(IntPtr userdataPtr, out object boxed)
        {
            return s_boxedByUserData.TryGetValue(userdataPtr, out boxed);
        }

        internal static void Unregister(IntPtr userdataPtr)
        {
            s_boxedByUserData.Remove(userdataPtr);
        }

        internal static IntPtr GetOnReleaseFunctionPointer()
        {
            return global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_onRelease);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int OnReleaseByValUserData(IntPtr L)
        {
            IntPtr ud = LuaDll.lua_touserdata(L, 1);
            if (ud != IntPtr.Zero)
            {
                Unregister(ud);
            }

            return 0;
        }
    }
}
