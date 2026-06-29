using System;
using System.Runtime.InteropServices;
using System.Text;

namespace ZLua
{
    internal static class LuaMarshalBytes
    {
        internal static void PushByteArray(IntPtr luaState, byte[] bytes)
        {
            if (bytes == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            if (bytes.Length == 0)
            {
                LuaDll.lua_pushlstring(luaState, IntPtr.Zero, UIntPtr.Zero);
                return;
            }

            unsafe
            {
                fixed (byte* data = bytes)
                {
                    LuaDll.lua_pushlstring(luaState, (IntPtr)data, (UIntPtr)bytes.Length);
                }
            }
        }

        internal static byte[] ReadByteArray(IntPtr luaState, int index)
        {
            if (LuaDll.lua_type(luaState, index) == LuaDataType.Nil)
            {
                return null;
            }

            IntPtr strPtr = LuaDll.lua_tolstring(luaState, index, out UIntPtr len);
            if (strPtr == IntPtr.Zero)
            {
                return Array.Empty<byte>();
            }

            int length = (int)len;
            byte[] bytes = new byte[length];
            Marshal.Copy(strPtr, bytes, 0, length);
            return bytes;
        }

        internal static void PushStringAsOctets(IntPtr luaState, string value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            byte[] bytes = Encoding.UTF8.GetBytes(value);
            PushByteArray(luaState, bytes);
        }

        internal static string ReadStringAsOctets(IntPtr luaState, int index)
        {
            if (LuaDll.lua_type(luaState, index) == LuaDataType.Nil)
            {
                return null;
            }

            byte[] bytes = ReadByteArray(luaState, index);
            return bytes == null ? null : Encoding.UTF8.GetString(bytes);
        }
    }
}
