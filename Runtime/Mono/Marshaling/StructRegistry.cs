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
using System.Collections.Generic;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Tracks non-blittable ByVal userdata boxed companions (GC roots). Blittable ByVal uses payload only.
    /// </summary>
    internal static class StructRegistry
    {
        private static readonly Dictionary<IntPtr, object> s_boxedByUserData = new Dictionary<IntPtr, object>();
        private static readonly LuaCSFunction s_onRelease = OnReleaseByValUserData;

        internal static void Initialize(IntPtr L)
        {
            _ = L;
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
