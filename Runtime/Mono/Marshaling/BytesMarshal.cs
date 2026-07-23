using System;
using System.Runtime.InteropServices;
using System.Text;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class BytesMarshal
    {
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

        internal static void PushStringAsOctets(IntPtr L, string value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            PushByteArray(L, Encoding.UTF8.GetBytes(value));
        }

        internal static string ReadStringAsOctets(IntPtr L, int index)
        {
            if (LuaDll.lua_type(L, index) == LuaDataType.Nil)
            {
                return null;
            }

            byte[] bytes = ReadByteArray(L, index);
            return bytes == null ? null : Encoding.UTF8.GetString(bytes);
        }
    }
}
