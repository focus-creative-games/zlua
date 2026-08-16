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
using System.Runtime.InteropServices;

namespace ZLua.Marshaling
{
    internal enum UserDataKind : byte
    {
        Unknown = 0,
        ByObj = 1,
        ByVal = 2,
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct UserDataHeader
    {
        public UserDataKind Kind;
    }

    /// <summary>
    /// By-obj userdata header. Managed object / view type live in <see cref="ObjectRegistry"/> slots.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct ZLuaObjectUserData
    {
        public UserDataHeader Header;
        public uint SlotIndex;
    }

    /// <summary>
    /// By-val userdata header; payload bytes follow immediately after this struct.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct ByValUserDataHeader
    {
        public UserDataHeader Header;
        public IntPtr TypeHandle;
    }

    /// <summary>
    /// Injected by Mt so Marshal does not take a hard dependency on TypeRegistry internals.
    /// </summary>
    internal static class MetatableHooks
    {
        internal static Action<IntPtr, Type> PushByObjMetatable;
        internal static Action<IntPtr, Type> PushByValMetatable;
    }
}
