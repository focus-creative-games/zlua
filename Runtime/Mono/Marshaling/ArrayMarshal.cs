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
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class ArrayMarshal
    {
        internal static bool TryGetConsecutiveTableLength(IntPtr L, int index, out int length)
        {
            length = 0;
            int absIndex = LuaDll.lua_absindex(L, index);
            if (LuaDll.lua_type(L, absIndex) != LuaDataType.Table)
            {
                return false;
            }

            for (int i = 1; ; i++)
            {
                LuaDll.lua_rawgeti(L, absIndex, i);
                if (LuaDll.lua_type(L, -1) == LuaDataType.Nil)
                {
                    LuaDll.lua_pop(L, 1);
                    break;
                }

                LuaDll.lua_pop(L, 1);
                length = i;
            }

            return true;
        }

        internal static Array PopSzArray(IntPtr L, int index, Type arrayType)
        {
            if (arrayType == null || !arrayType.IsArray || arrayType.GetArrayRank() != 1)
            {
                LuaCallbackBoundary.Throw("zlua internal error: PopSzArray expects szarray type");
            }

            LuaDataType valueType = LuaDll.lua_type(L, index);
            switch (valueType)
            {
                case LuaDataType.Nil:
                    return null;

                case LuaDataType.Table:
                    return PopFromTable(L, index, arrayType);

                case LuaDataType.UserData:
                    return PopFromArrayObject(L, index, arrayType);

                default:
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: expected array or table, got: {valueType}");
                    return null;
            }
        }

        private static Array PopFromArrayObject(IntPtr L, int index, Type arrayType)
        {
            object obj = ObjectRegistry.Pop(L, index);
            if (obj == null)
            {
                return null;
            }

            if (!arrayType.IsInstanceOfType(obj))
            {
                LuaCallbackBoundary.Throw("zlua argument mismatch: array type mismatch");
            }

            return (Array)obj;
        }

        private static Array PopFromTable(IntPtr L, int index, Type arrayType)
        {
            if (!TryGetConsecutiveTableLength(L, index, out int length))
            {
                LuaCallbackBoundary.Throw("zlua argument mismatch: table value must be a table");
            }

            Type elementType = arrayType.GetElementType();
            Array newArray = Array.CreateInstance(elementType, length);
            int absIndex = LuaDll.lua_absindex(L, index);
            for (int i = 0; i < length; i++)
            {
                LuaDll.lua_rawgeti(L, absIndex, i + 1);
                SetElement1D(L, newArray, i, -1, elementType);
                LuaDll.lua_pop(L, 1);
            }

            return newArray;
        }

        /// <summary>
        /// Rank-1 element push without <see cref="Array.GetValue(int[])"/> index allocation.
        /// Dispatch by exact element Type — do not use <c>is byte[]</c>/<c>is sbyte[]</c>
        /// (CLR treats same-width signed/unsigned arrays as assignment-compatible).
        /// </summary>
        internal static void PushElement1D(IntPtr L, Array array, int index)
        {
            Type elementType = array.GetType().GetElementType();
            if (elementType == typeof(bool))
            {
                PrimitiveMarshal.PushBool(L, ((bool[])array)[index]);
                return;
            }

            if (elementType == typeof(byte))
            {
                PrimitiveMarshal.PushUInt8(L, ((byte[])array)[index]);
                return;
            }

            if (elementType == typeof(sbyte))
            {
                PrimitiveMarshal.PushInt8(L, ((sbyte[])array)[index]);
                return;
            }

            if (elementType == typeof(short))
            {
                PrimitiveMarshal.PushInt16(L, ((short[])array)[index]);
                return;
            }

            if (elementType == typeof(ushort))
            {
                PrimitiveMarshal.PushUInt16(L, ((ushort[])array)[index]);
                return;
            }

            if (elementType == typeof(char))
            {
                PrimitiveMarshal.PushUInt16(L, ((char[])array)[index]);
                return;
            }

            if (elementType == typeof(int))
            {
                PrimitiveMarshal.PushInt32(L, ((int[])array)[index]);
                return;
            }

            if (elementType == typeof(uint))
            {
                PrimitiveMarshal.PushUInt32(L, ((uint[])array)[index]);
                return;
            }

            if (elementType == typeof(long))
            {
                PrimitiveMarshal.PushInt64(L, ((long[])array)[index]);
                return;
            }

            if (elementType == typeof(ulong))
            {
                PrimitiveMarshal.PushUInt64(L, ((ulong[])array)[index]);
                return;
            }

            if (elementType == typeof(float))
            {
                PrimitiveMarshal.PushFloat(L, ((float[])array)[index]);
                return;
            }

            if (elementType == typeof(double))
            {
                PrimitiveMarshal.PushDouble(L, ((double[])array)[index]);
                return;
            }

            if (elementType == typeof(string))
            {
                PrimitiveMarshal.PushString(L, ((string[])array)[index]);
                return;
            }

            if (elementType == typeof(object))
            {
                ObjectMarshal.Push(L, ((object[])array)[index]);
                return;
            }

            TypedMarshal.PushObject(L, array.GetValue(index), elementType);
        }

        /// <summary>
        /// Rank-1 element store without <see cref="Array.SetValue(object,int[])"/> index allocation.
        /// </summary>
        internal static void SetElement1D(IntPtr L, Array array, int index, int valueStackIndex, Type elementType)
        {
            if (elementType == typeof(bool))
            {
                ((bool[])array)[index] = PrimitiveMarshal.PopBool(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(byte))
            {
                ((byte[])array)[index] = PrimitiveMarshal.PopUInt8(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(sbyte))
            {
                ((sbyte[])array)[index] = PrimitiveMarshal.PopInt8(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(short))
            {
                ((short[])array)[index] = PrimitiveMarshal.PopInt16(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(ushort))
            {
                ((ushort[])array)[index] = PrimitiveMarshal.PopUInt16(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(char))
            {
                ((char[])array)[index] = (char)PrimitiveMarshal.PopUInt16(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(int))
            {
                ((int[])array)[index] = PrimitiveMarshal.PopInt32(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(uint))
            {
                ((uint[])array)[index] = PrimitiveMarshal.PopUInt32(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(long))
            {
                ((long[])array)[index] = PrimitiveMarshal.PopInt64(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(ulong))
            {
                ((ulong[])array)[index] = PrimitiveMarshal.PopUInt64(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(float))
            {
                ((float[])array)[index] = PrimitiveMarshal.PopFloat(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(double))
            {
                ((double[])array)[index] = PrimitiveMarshal.PopDouble(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(string))
            {
                ((string[])array)[index] = PrimitiveMarshal.PopString(L, valueStackIndex);
                return;
            }

            if (elementType == typeof(object))
            {
                ((object[])array)[index] = ObjectMarshal.Pop(L, valueStackIndex, typeof(object));
                return;
            }

            object value = TypedMarshal.PopObject(L, valueStackIndex, elementType);
            value = CoerceToElementType(value, elementType);
            array.SetValue(value, index);
        }

        /// <summary>
        /// Push rank-1 array contents as a Lua table (1-based consecutive keys).
        /// </summary>
        internal static void PushSzArrayAsTable(IntPtr L, Array array)
        {
            int length = array.Length;
            LuaDll.lua_createtable(L, length, 0);
            for (int i = 0; i < length; i++)
            {
                int absoluteIndex = array.GetLowerBound(0) + i;
                PushElement1D(L, array, absoluteIndex);
                LuaDll.lua_rawseti(L, -2, i + 1);
            }
        }

        /// <summary>
        /// Coerce Lua-numeric / boxed values into the array element type before <see cref="Array.SetValue"/>.
        /// </summary>
        internal static object CoerceToElementType(object value, Type elementType)
        {
            if (value == null || elementType == null || elementType.IsInstanceOfType(value))
            {
                return value;
            }

            Type target = Nullable.GetUnderlyingType(elementType) ?? elementType;
            if (target.IsEnum)
            {
                if (value is string s)
                {
                    return Enum.Parse(target, s);
                }

                return Enum.ToObject(target, Convert.ChangeType(value, Enum.GetUnderlyingType(target)));
            }

            if (target.IsPrimitive)
            {
                return Convert.ChangeType(value, target);
            }

            return value;
        }
    }
}
