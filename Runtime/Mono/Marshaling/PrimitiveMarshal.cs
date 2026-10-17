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
using System.Runtime.CompilerServices;
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

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static void ThrowIntegerOutOfRange(int idx, long value, long minV, long maxV, string typeName)
        {
            LuaCallbackBoundary.Throw(
                $"zlua argument mismatch: integer {value} out of {typeName} range [{minV}, {maxV}] at index {idx}");
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
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < sbyte.MinValue || value > sbyte.MaxValue)
                ThrowIntegerOutOfRange(idx, value, sbyte.MinValue, sbyte.MaxValue, "System.SByte");
            return (sbyte)value;
        }

        internal static void PushUInt8(IntPtr L, byte v) => LuaDll.lua_pushinteger(L, v);
        internal static byte PopUInt8(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < byte.MinValue || value > byte.MaxValue)
                ThrowIntegerOutOfRange(idx, value, byte.MinValue, byte.MaxValue, "System.Byte");
            return (byte)value;
        }

        internal static void PushInt16(IntPtr L, short v) => LuaDll.lua_pushinteger(L, v);
        internal static short PopInt16(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < short.MinValue || value > short.MaxValue)
                ThrowIntegerOutOfRange(idx, value, short.MinValue, short.MaxValue, "System.Int16");
            return (short)value;
        }

        internal static void PushUInt16(IntPtr L, ushort v) => LuaDll.lua_pushinteger(L, v);
        internal static ushort PopUInt16(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < ushort.MinValue || value > ushort.MaxValue)
                ThrowIntegerOutOfRange(idx, value, ushort.MinValue, ushort.MaxValue, "System.UInt16");
            return (ushort)value;
        }

        internal static void PushInt32(IntPtr L, int v) => LuaDll.lua_pushinteger(L, v);
        internal static int PopInt32(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < int.MinValue || value > int.MaxValue)
                ThrowIntegerOutOfRange(idx, value, int.MinValue, int.MaxValue, "System.Int32");
            return (int)value;
        }

        internal static void PushUInt32(IntPtr L, uint v) => LuaDll.lua_pushinteger(L, v);
        internal static uint PopUInt32(IntPtr L, int idx)
        {
            Expect(L, idx, LuaDataType.Number, "number");
            long value = LuaDll.lua_tointeger(L, idx);
            if (value < uint.MinValue || value > uint.MaxValue)
                ThrowIntegerOutOfRange(idx, value, uint.MinValue, uint.MaxValue, "System.UInt32");
            return (uint)value;
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
                long signedValue = LuaDll.lua_tointeger(L, idx);
                if (signedValue < 0)
                    ThrowIntegerOutOfRange(idx, signedValue, 0, long.MaxValue, "System.UInt64");
                return (ulong)signedValue;
            }

            double number = LuaDll.lua_tonumber(L, idx);
            if (number < 0)
                ThrowIntegerOutOfRange(idx, (long)number, 0, long.MaxValue, "System.UInt64");
            return (ulong)number;
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
