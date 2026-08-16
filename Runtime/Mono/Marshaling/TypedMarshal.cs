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
    /// <summary>
    /// Typed push/pop entry (declared-type dispatch to Primitive/Struct/Object marshalers).
    /// </summary>
    internal static class TypedMarshal
    {
        internal static void PushObject(IntPtr L, object value, Type declaredType)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            Type runtimeType = value.GetType();
            if (declaredType == null)
            {
                declaredType = runtimeType;
            }

            if (declaredType == typeof(string))
            {
                PrimitiveMarshal.PushString(L, value as string);
                return;
            }

            Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
            if (nullableUnderlying != null)
            {
                // Nullable<T> → push as underlying (or nil when null, handled above).
                object inner = value;
                Type valueType = value.GetType();
                if (valueType.IsGenericType && valueType.GetGenericTypeDefinition() == typeof(Nullable<>))
                {
                    inner = valueType.GetProperty("Value")?.GetValue(value) ?? value;
                }

                PushObject(L, inner, nullableUnderlying);
                return;
            }

            if (declaredType.IsEnum)
            {
                // Enum → Lua number (underlying); boxed enum ByObj via zlua.box.
                object underlying = Convert.ChangeType(value, Enum.GetUnderlyingType(declaredType));
                PushPrimitiveBoxed(L, underlying);
                return;
            }

            if (PointerMarshal.IsPointerLikeType(declaredType) || PointerMarshal.IsPointerLikeType(runtimeType))
            {
                PointerMarshal.PushPointer(L, PointerMarshal.CoerceToAddress(value));
                return;
            }

            if (StructMarshal.IsStructType(declaredType))
            {
                StructMarshal.PushValue(L, value, declaredType);
                return;
            }

            if (declaredType.IsValueType && declaredType.IsPrimitive)
            {
                PushPrimitiveBoxed(L, value);
                return;
            }

            if (typeof(Delegate).IsAssignableFrom(declaredType))
            {
                DelegateMarshal.Push(L, (Delegate)value, declaredType);
                return;
            }

            // Declared-type facade for reference types.
            ObjectMarshal.Push(L, value, declaredType);
        }

        internal static object PopObject(IntPtr L, int index, Type declaredType)
        {
            if (declaredType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: declaredType is null");
            }

            if (declaredType == typeof(string))
            {
                LuaDataType luaType = LuaDll.lua_type(L, index);
                if (luaType == LuaDataType.UserData)
                {
                    object obj = ObjectRegistry.Pop(L, index);
                    if (obj == null)
                    {
                        return null;
                    }

                    if (obj is string s)
                    {
                        return s;
                    }

                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: expected string userdata, got: {obj.GetType().FullName}");
                }

                return PrimitiveMarshal.PopString(L, index);
            }

            Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
            if (nullableUnderlying != null)
            {
                if (LuaDll.lua_isnil(L, index))
                {
                    return null;
                }

                object inner = PopObject(L, index, nullableUnderlying);
                return Activator.CreateInstance(declaredType, inner);
            }

            if (declaredType.IsEnum)
            {
                Type underlying = Enum.GetUnderlyingType(declaredType);
                object raw = PopPrimitive(L, index, underlying);
                return Enum.ToObject(declaredType, raw);
            }

            if (PointerMarshal.IsPointerLikeType(declaredType))
            {
                IntPtr address = PointerMarshal.PopPointer(L, index);
                return address == IntPtr.Zero ? null : (object)address;
            }

            if (PointerMarshal.TryGetRejectMessage(declaredType, out string rejectMessage))
            {
                LuaCallbackBoundary.Throw(rejectMessage);
            }

            if (StructMarshal.IsStructType(declaredType))
            {
                return StructMarshal.PopValue(L, index, declaredType);
            }

            if (declaredType.IsValueType && declaredType.IsPrimitive)
            {
                return PopPrimitive(L, index, declaredType);
            }

            if (typeof(Delegate).IsAssignableFrom(declaredType))
            {
                return DelegateMarshal.Pop(L, index, declaredType);
            }

            if (declaredType.IsArray && declaredType.GetArrayRank() == 1)
            {
                return ArrayMarshal.PopSzArray(L, index, declaredType);
            }

            return ObjectMarshal.Pop(L, index, declaredType);
        }

        private static void PushPrimitiveBoxed(IntPtr L, object value)
        {
            switch (value)
            {
                case bool b:
                    PrimitiveMarshal.PushBool(L, b);
                    break;
                case sbyte sb:
                    PrimitiveMarshal.PushInt8(L, sb);
                    break;
                case byte ub:
                    PrimitiveMarshal.PushUInt8(L, ub);
                    break;
                case short s:
                    PrimitiveMarshal.PushInt16(L, s);
                    break;
                case ushort us:
                    PrimitiveMarshal.PushUInt16(L, us);
                    break;
                case int i:
                    PrimitiveMarshal.PushInt32(L, i);
                    break;
                case uint ui:
                    PrimitiveMarshal.PushUInt32(L, ui);
                    break;
                case long l:
                    PrimitiveMarshal.PushInt64(L, l);
                    break;
                case ulong ul:
                    PrimitiveMarshal.PushUInt64(L, ul);
                    break;
                case float f:
                    PrimitiveMarshal.PushFloat(L, f);
                    break;
                case double d:
                    PrimitiveMarshal.PushDouble(L, d);
                    break;
                case char c:
                    PrimitiveMarshal.PushUInt16(L, c);
                    break;
                case IntPtr p:
                    PrimitiveMarshal.PushIntPtr(L, p);
                    break;
                case UIntPtr up:
                    PrimitiveMarshal.PushUIntPtr(L, up);
                    break;
                default:
                    LuaCallbackBoundary.Throw($"zlua: unsupported primitive: {value.GetType().FullName}");
                    break;
            }
        }

        private static object PopPrimitive(IntPtr L, int index, Type type)
        {
            if (type == typeof(bool)) return PrimitiveMarshal.PopBool(L, index);
            if (type == typeof(sbyte)) return PrimitiveMarshal.PopInt8(L, index);
            if (type == typeof(byte)) return PrimitiveMarshal.PopUInt8(L, index);
            if (type == typeof(short)) return PrimitiveMarshal.PopInt16(L, index);
            if (type == typeof(ushort)) return PrimitiveMarshal.PopUInt16(L, index);
            if (type == typeof(int)) return PrimitiveMarshal.PopInt32(L, index);
            if (type == typeof(uint)) return PrimitiveMarshal.PopUInt32(L, index);
            if (type == typeof(long)) return PrimitiveMarshal.PopInt64(L, index);
            if (type == typeof(ulong)) return PrimitiveMarshal.PopUInt64(L, index);
            if (type == typeof(float)) return PrimitiveMarshal.PopFloat(L, index);
            if (type == typeof(double)) return PrimitiveMarshal.PopDouble(L, index);
            if (type == typeof(char)) return (char)PrimitiveMarshal.PopUInt16(L, index);
            if (type == typeof(IntPtr)) return PrimitiveMarshal.PopIntPtr(L, index);
            if (type == typeof(UIntPtr)) return PrimitiveMarshal.PopUIntPtr(L, index);
            LuaCallbackBoundary.Throw($"zlua: unsupported primitive: {type.FullName}");
            return null;
        }
    }
}