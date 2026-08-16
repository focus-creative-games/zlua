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
using System.Runtime.InteropServices;

namespace ZLua.Utils
{
    /// <summary>
    /// Stable lightuserdata tokens for <see cref="Type"/> (Il2Cpp <c>Il2CppClass*</c> equivalent).
    /// </summary>
    internal static class TypeHandleStore
    {
        private static readonly Dictionary<Type, IntPtr> TypeToHandle = new Dictionary<Type, IntPtr>();
        private static readonly Dictionary<IntPtr, Type> HandleToType = new Dictionary<IntPtr, Type>();
        private static readonly object Sync = new object();

        internal static IntPtr GetHandle(Type type)
        {
            if (type == null)
            {
                throw new ArgumentNullException(nameof(type));
            }

            lock (Sync)
            {
                if (TypeToHandle.TryGetValue(type, out IntPtr existing))
                {
                    return existing;
                }

                GCHandle handle = GCHandle.Alloc(type, GCHandleType.Normal);
                IntPtr ptr = GCHandle.ToIntPtr(handle);
                TypeToHandle[type] = ptr;
                HandleToType[ptr] = type;
                return ptr;
            }
        }

        internal static Type GetType(IntPtr handle)
        {
            if (handle == IntPtr.Zero)
            {
                return null;
            }

            lock (Sync)
            {
                return HandleToType.TryGetValue(handle, out Type type) ? type : null;
            }
        }

        internal static void PushLightUserData(IntPtr luaState, Type type)
        {
            LuaDll.lua_pushlightuserdata(luaState, GetHandle(type));
        }

        internal static Type ReadLightUserData(IntPtr luaState, int index)
        {
            IntPtr ptr = LuaDll.lua_touserdata(luaState, index);
            return GetType(ptr);
        }
    }
}
