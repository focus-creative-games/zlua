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
using System.Buffers;
using System.Runtime.InteropServices;
using System.Text;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class BytesMarshal
    {
        private const int StackBufferSize = 512;

        internal static void PushByteArray(IntPtr L, byte[] bytes)
        {
            if (bytes == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (bytes.Length == 0)
            {
                LuaDll.lua_pushlstring(L, IntPtr.Zero, UIntPtr.Zero);
                return;
            }

            unsafe
            {
                fixed (byte* data = bytes)
                {
                    LuaDll.lua_pushlstring(L, (IntPtr)data, (UIntPtr)bytes.Length);
                }
            }
        }

        internal static byte[] ReadByteArray(IntPtr L, int index)
        {
            if (LuaDll.lua_type(L, index) == LuaDataType.Nil)
            {
                return null;
            }

            IntPtr strPtr = LuaDll.lua_tolstring(L, index, out UIntPtr len);
            if (strPtr == IntPtr.Zero)
            {
                return Array.Empty<byte>();
            }

            int length = (int)len;
            byte[] bytes = new byte[length];
            Marshal.Copy(strPtr, bytes, 0, length);
            return bytes;
        }

        internal static unsafe void PushStringAsOctets(IntPtr L, string value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            int byteCount = Encoding.UTF8.GetByteCount(value);
            if (byteCount == 0)
            {
                LuaDll.lua_pushlstring(L, IntPtr.Zero, UIntPtr.Zero);
                return;
            }

            if (byteCount <= StackBufferSize)
            {
                byte* buffer = stackalloc byte[StackBufferSize];
                PushEncodedString(L, value, buffer, byteCount);
                return;
            }

            // Each invocation owns its lease until lua_pushlstring has copied
            // the bytes. Nested calls and other threads must rent independently.
            byte[] rented = ArrayPool<byte>.Shared.Rent(byteCount);
            try
            {
                fixed (byte* buffer = rented)
                {
                    PushEncodedString(L, value, buffer, byteCount);
                }
            }
            finally
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
        }

        private static unsafe void PushEncodedString(IntPtr L, string value, byte* buffer, int byteCount)
        {
            fixed (char* chars = value)
            {
                int written = Encoding.UTF8.GetBytes(chars, value.Length, buffer, byteCount);
                LuaDll.lua_pushlstring(L, (IntPtr)buffer, (UIntPtr)written);
            }
        }

        internal static unsafe string ReadStringAsOctets(IntPtr L, int index)
        {
            if (LuaDll.lua_type(L, index) == LuaDataType.Nil)
            {
                return null;
            }

            // lua_tolstring roots the resulting string in its stack slot. Decode
            // it before the stack changes; no copied byte[] or pinned lease is needed.
            IntPtr strPtr = LuaDll.lua_tolstring(L, index, out UIntPtr len);
            if (strPtr == IntPtr.Zero || len == UIntPtr.Zero)
            {
                return string.Empty;
            }

            int length = checked((int)len.ToUInt64());
            return Encoding.UTF8.GetString((byte*)strPtr, length);
        }
    }
}
