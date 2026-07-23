using System;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class PrimitiveMarshal
    {
        private static void Expect(IntPtr L, int idx, LuaDataType expected, string name)
        {
            if (LuaDll.lua_type(L, idx) != expected)
            {
                LuaCallbackBoundary.Throw($"zlua argument mismatch: expected {name} at index {idx}");
            }
        }

        internal static void PushBool(IntPtr L, bool v) => LuaDll.lua_pushboolean(L, v ? 1 : 0);
        internal static bool PopBool(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Boolean, "boolean");
            return LuaDll.lua_toboolean(L, idx) != 0;
        }

        internal static void PushInt8(IntPtr L, sbyte v) => LuaDll.lua_pushinteger(L, v);
        internal static sbyte PopInt8(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (sbyte)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushUInt8(IntPtr L, byte v) => LuaDll.lua_pushinteger(L, v);
        internal static byte PopUInt8(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (byte)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushInt16(IntPtr L, short v) => LuaDll.lua_pushinteger(L, v);
        internal static short PopInt16(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (short)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushUInt16(IntPtr L, ushort v) => LuaDll.lua_pushinteger(L, v);
        internal static ushort PopUInt16(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (ushort)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushInt32(IntPtr L, int v) => LuaDll.lua_pushinteger(L, v);
        internal static int PopInt32(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (int)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushUInt32(IntPtr L, uint v) => LuaDll.lua_pushinteger(L, v);
        internal static uint PopUInt32(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (uint)LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushInt64(IntPtr L, long v) => LuaDll.lua_pushinteger(L, v);
        internal static long PopInt64(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return LuaDll.lua_tointeger(L, idx);
        }

        internal static void PushUInt64(IntPtr L, ulong v) => LuaDll.lua_pushinteger(L, (long)v);
        internal static ulong PopUInt64(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            if (LuaDll.lua_isinteger(L, idx) != 0)
            {
                return (ulong)LuaDll.lua_tointeger(L, idx);
            }

            return (ulong)LuaDll.lua_tonumber(L, idx);
        }

        internal static void PushFloat(IntPtr L, float v) => LuaDll.lua_pushnumber(L, v);
        internal static float PopFloat(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return (float)LuaDll.lua_tonumber(L, idx);
        }

        internal static void PushDouble(IntPtr L, double v) => LuaDll.lua_pushnumber(L, v);
        internal static double PopDouble(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return LuaDll.lua_tonumber(L, idx);
        }

        internal static void PushIntPtr(IntPtr L, IntPtr v) => LuaDll.lua_pushinteger(L, v.ToInt64());
        internal static IntPtr PopIntPtr(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return new IntPtr(LuaDll.lua_tointeger(L, idx));
        }

        internal static void PushUIntPtr(IntPtr L, UIntPtr v) => LuaDll.lua_pushinteger(L, (long)v.ToUInt64());
        internal static UIntPtr PopUIntPtr(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            return new UIntPtr((ulong)LuaDll.lua_tointeger(L, idx));
        }

        internal static void PushString(IntPtr L, string v)
        {
            if (v == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            LuaDll.lua_pushstring(L, v);
        }

        internal static string PopString(IntPtr L, int idx)
        {
            if (LuaDll.lua_isnil(L, idx))
            {
                return null;
            }

            Expect(L, idx, LuaDataType.String, "string");
            return LuaDllExtension.tostring(L, idx);
        }
    }
}
