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
using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Marshaling;

namespace ZLua.Emit
{
    /// <summary>
    /// Expression factories that pop/push primitives, enums, string, and blittable structs
    /// without boxing through object. Other types fall back to object PopArg / PushReturn.
    /// </summary>
    internal static class TypedBridgeEmit
    {
        private const BindingFlags StaticAny =
            BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;

        private static readonly ConcurrentDictionary<Type, MethodInfo> s_pushStructCache =
            new ConcurrentDictionary<Type, MethodInfo>();
        private static readonly ConcurrentDictionary<Type, MethodInfo> s_popStructCache =
            new ConcurrentDictionary<Type, MethodInfo>();

        private static readonly MethodInfo s_pushStructOpen = FindGenericStructMarshal(
            nameof(StructMarshal.PushValue), parameterCount: 2);
        private static readonly MethodInfo s_popStructOpen = FindGenericStructMarshal(
            nameof(StructMarshal.PopValue), parameterCount: 2);

        private static readonly MethodInfo s_popBool = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopBool));
        private static readonly MethodInfo s_pushBool = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushBool));
        private static readonly MethodInfo s_popInt8 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopInt8));
        private static readonly MethodInfo s_pushInt8 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushInt8));
        private static readonly MethodInfo s_popUInt8 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopUInt8));
        private static readonly MethodInfo s_pushUInt8 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushUInt8));
        private static readonly MethodInfo s_popInt16 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopInt16));
        private static readonly MethodInfo s_pushInt16 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushInt16));
        private static readonly MethodInfo s_popUInt16 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopUInt16));
        private static readonly MethodInfo s_pushUInt16 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushUInt16));
        private static readonly MethodInfo s_popInt32 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopInt32));
        private static readonly MethodInfo s_pushInt32 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushInt32));
        private static readonly MethodInfo s_popUInt32 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopUInt32));
        private static readonly MethodInfo s_pushUInt32 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushUInt32));
        private static readonly MethodInfo s_popInt64 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopInt64));
        private static readonly MethodInfo s_pushInt64 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushInt64));
        private static readonly MethodInfo s_popUInt64 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopUInt64));
        private static readonly MethodInfo s_pushUInt64 = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushUInt64));
        private static readonly MethodInfo s_popFloat = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopFloat));
        private static readonly MethodInfo s_pushFloat = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushFloat));
        private static readonly MethodInfo s_popDouble = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopDouble));
        private static readonly MethodInfo s_pushDouble = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushDouble));
        private static readonly MethodInfo s_popIntPtr = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopIntPtr));
        private static readonly MethodInfo s_pushIntPtr = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushIntPtr));
        private static readonly MethodInfo s_popUIntPtr = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopUIntPtr));
        private static readonly MethodInfo s_pushUIntPtr = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushUIntPtr));
        private static readonly MethodInfo s_popString = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PopString));
        private static readonly MethodInfo s_pushString = Require(typeof(PrimitiveMarshal), nameof(PrimitiveMarshal.PushString));

        /// <summary>
        /// Expression of type <paramref name="declaredType"/> popped from Lua stack slot <paramref name="index"/>.
        /// </summary>
        internal static Expression PopArg(Expression L, int index, Type declaredType)
        {
            return PopArg(L, Expression.Constant(index), declaredType);
        }

        /// <summary>
        /// Pop from <paramref name="index"/> when <paramref name="top"/> &gt;= index; otherwise use typed default.
        /// </summary>
        internal static Expression PopArgOrDefault(
            Expression L,
            Expression top,
            int index,
            Type declaredType,
            object defaultValue)
        {
            Expression hasArg = Expression.GreaterThanOrEqual(top, Expression.Constant(index));
            Expression popped = PopArg(L, Expression.Constant(index), declaredType);
            Expression fallback = Expression.Constant(defaultValue, declaredType);
            return Expression.Condition(hasArg, popped, fallback);
        }

        private static Expression PopArg(Expression L, Expression indexExpr, Type declaredType)
        {
            if (TryGetPrimitivePop(declaredType, out MethodInfo pop, out Type resultType))
            {
                Expression typed = Expression.Call(pop, L, indexExpr);
                if (resultType != declaredType)
                {
                    typed = Expression.Convert(typed, declaredType);
                }

                return typed;
            }

            if (declaredType == typeof(string))
            {
                return Expression.Call(s_popString, L, indexExpr);
            }

            if (declaredType.IsEnum)
            {
                Type underlying = Enum.GetUnderlyingType(declaredType);
                if (!TryGetPrimitivePop(underlying, out MethodInfo popUnderlying, out _))
                {
                    return FallbackPop(L, indexExpr, declaredType);
                }

                Expression raw = Expression.Call(popUnderlying, L, indexExpr);
                return Expression.Convert(raw, declaredType);
            }

            if (TryGetBlittableStructPop(declaredType, out MethodInfo popStruct))
            {
                return Expression.Call(popStruct, L, indexExpr);
            }

            return FallbackPop(L, indexExpr, declaredType);
        }

        /// <summary>
        /// Push <paramref name="value"/> (typed) and return Expression of int (return count).
        /// </summary>
        internal static Expression PushReturn(Expression L, Type declaredType, Expression value)
        {
            if (declaredType == typeof(void))
            {
                return Expression.Constant(0);
            }

            if (declaredType == typeof(char))
            {
                Expression asChar = value.Type == typeof(char)
                    ? value
                    : Expression.Convert(value, typeof(char));
                Expression asUshort = Expression.Convert(asChar, typeof(ushort));
                return Expression.Block(
                    Expression.Call(s_pushUInt16, L, asUshort),
                    Expression.Constant(1));
            }

            if (TryGetPrimitivePush(declaredType, out MethodInfo push))
            {
                Expression coerced = value.Type == declaredType
                    ? value
                    : Expression.Convert(value, declaredType);
                return Expression.Block(
                    Expression.Call(push, L, coerced),
                    Expression.Constant(1));
            }

            if (declaredType == typeof(string))
            {
                Expression asString = value.Type == typeof(string)
                    ? value
                    : Expression.Convert(value, typeof(string));
                return Expression.Block(
                    Expression.Call(s_pushString, L, asString),
                    Expression.Constant(1));
            }

            if (declaredType.IsEnum)
            {
                Type underlying = Enum.GetUnderlyingType(declaredType);
                if (!TryGetPrimitivePush(underlying, out MethodInfo pushUnderlying))
                {
                    return FallbackPush(L, declaredType, value);
                }

                Expression asUnderlying = Expression.Convert(value, underlying);
                return Expression.Block(
                    Expression.Call(pushUnderlying, L, asUnderlying),
                    Expression.Constant(1));
            }

            if (TryGetBlittableStructPush(declaredType, out MethodInfo pushStruct))
            {
                Expression typed = value.Type == declaredType
                    ? value
                    : Expression.Convert(value, declaredType);
                return Expression.Block(
                    Expression.Call(pushStruct, L, typed),
                    Expression.Constant(1));
            }

            return FallbackPush(L, declaredType, value);
        }

        /// <summary>
        /// True when Pop/Push avoid the object boundary for this type.
        /// </summary>
        internal static bool IsZeroBoxMarshalType(Type type)
        {
            if (type == null || type == typeof(void))
            {
                return true;
            }

            if (type == typeof(string) || type.IsEnum)
            {
                return true;
            }

            if (TryGetPrimitivePop(type, out _, out _))
            {
                return true;
            }

            return TryGetBlittableStructPop(type, out _);
        }

        private static bool TryGetBlittableStructPush(Type type, out MethodInfo method)
        {
            method = null;
            if (!StructMarshal.IsStructType(type) || !StructMarshal.IsBlittable(type))
            {
                return false;
            }

            if (s_pushStructCache.TryGetValue(type, out method))
            {
                return true;
            }

            try
            {
                method = s_pushStructOpen.MakeGenericMethod(type);
            }
            catch (ArgumentException)
            {
                method = null;
                return false;
            }

            s_pushStructCache[type] = method;
            return true;
        }

        private static bool TryGetBlittableStructPop(Type type, out MethodInfo method)
        {
            method = null;
            if (!StructMarshal.IsStructType(type) || !StructMarshal.IsBlittable(type))
            {
                return false;
            }

            if (s_popStructCache.TryGetValue(type, out method))
            {
                return true;
            }

            try
            {
                method = s_popStructOpen.MakeGenericMethod(type);
            }
            catch (ArgumentException)
            {
                method = null;
                return false;
            }

            s_popStructCache[type] = method;
            return true;
        }

        private static MethodInfo FindGenericStructMarshal(string name, int parameterCount)
        {
            MethodInfo[] methods = typeof(StructMarshal).GetMethods(StaticAny);
            for (int i = 0; i < methods.Length; i++)
            {
                MethodInfo m = methods[i];
                if (m.Name == name
                    && m.IsGenericMethodDefinition
                    && m.GetParameters().Length == parameterCount)
                {
                    return m;
                }
            }

            throw new InvalidOperationException(
                $"zlua emit: missing generic StructMarshal.{name}");
        }

        private static Expression FallbackPop(Expression L, Expression indexExpr, Type declaredType)
        {
            Expression popped = Expression.Call(
                EmitMethods.PopArg,
                L,
                indexExpr,
                Expression.Constant(declaredType, typeof(Type)));
            return Expression.Convert(popped, declaredType);
        }

        private static Expression FallbackPush(Expression L, Type declaredType, Expression value)
        {
            Expression boxed = value.Type == typeof(object)
                ? value
                : Expression.Convert(value, typeof(object));
            return Expression.Call(
                EmitMethods.PushReturn,
                L,
                Expression.Constant(declaredType, typeof(Type)),
                boxed);
        }

        private static bool TryGetPrimitivePop(Type type, out MethodInfo method, out Type resultType)
        {
            method = null;
            resultType = type;
            if (type == typeof(bool)) { method = s_popBool; return true; }
            if (type == typeof(sbyte)) { method = s_popInt8; return true; }
            if (type == typeof(byte)) { method = s_popUInt8; return true; }
            if (type == typeof(short)) { method = s_popInt16; return true; }
            if (type == typeof(ushort)) { method = s_popUInt16; return true; }
            if (type == typeof(int)) { method = s_popInt32; return true; }
            if (type == typeof(uint)) { method = s_popUInt32; return true; }
            if (type == typeof(long)) { method = s_popInt64; return true; }
            if (type == typeof(ulong)) { method = s_popUInt64; return true; }
            if (type == typeof(float)) { method = s_popFloat; return true; }
            if (type == typeof(double)) { method = s_popDouble; return true; }
            if (type == typeof(char))
            {
                method = s_popUInt16;
                resultType = typeof(ushort);
                return true;
            }

            if (type == typeof(IntPtr)) { method = s_popIntPtr; return true; }
            if (type == typeof(UIntPtr)) { method = s_popUIntPtr; return true; }
            return false;
        }

        private static bool TryGetPrimitivePush(Type type, out MethodInfo method)
        {
            method = null;
            if (type == typeof(bool)) { method = s_pushBool; return true; }
            if (type == typeof(sbyte)) { method = s_pushInt8; return true; }
            if (type == typeof(byte)) { method = s_pushUInt8; return true; }
            if (type == typeof(short)) { method = s_pushInt16; return true; }
            if (type == typeof(ushort)) { method = s_pushUInt16; return true; }
            if (type == typeof(int)) { method = s_pushInt32; return true; }
            if (type == typeof(uint)) { method = s_pushUInt32; return true; }
            if (type == typeof(long)) { method = s_pushInt64; return true; }
            if (type == typeof(ulong)) { method = s_pushUInt64; return true; }
            if (type == typeof(float)) { method = s_pushFloat; return true; }
            if (type == typeof(double)) { method = s_pushDouble; return true; }
            if (type == typeof(char))
            {
                // Push as ushort; caller must convert char → ushort first when using this method directly.
                method = s_pushUInt16;
                return true;
            }

            if (type == typeof(IntPtr)) { method = s_pushIntPtr; return true; }
            if (type == typeof(UIntPtr)) { method = s_pushUIntPtr; return true; }
            return false;
        }

        private static MethodInfo Require(Type type, string name)
        {
            MethodInfo method = type.GetMethod(name, StaticAny);
            if (method == null)
            {
                throw new InvalidOperationException($"zlua emit: missing method {type.FullName}.{name}");
            }

            return method;
        }
    }
}
