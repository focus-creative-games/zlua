using System;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Shared Expression trees for Lua → C# compiled bridges (P0: typed pop/push without boxing).
    /// </summary>
    internal static class LuaToCSharpBridgeExpressionBuilder
    {
        private static readonly MethodInfo PopTargetMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopTarget),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopBoolMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopBool),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopCharMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopChar),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopByteMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopByte),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopSByteMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopSByte),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt16Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopInt16),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt16Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopUInt16),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt32Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopInt32),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt32Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopUInt32),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt64Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopInt64),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt64Method = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopUInt64),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopSingleMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopSingle),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopDoubleMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopDouble),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopIntPtrMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopIntPtr),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUIntPtrMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopUIntPtr),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopStringMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopString),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PushReturnMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PushReturn),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo LuaPushBoolean = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pushboolean),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo LuaPushInteger = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pushinteger),
            new[] { typeof(IntPtr), typeof(long) });

        private static readonly MethodInfo LuaPushNumber = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pushnumber),
            new[] { typeof(IntPtr), typeof(double) });

        private static readonly MethodInfo LuaPushNil = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pushnil),
            new[] { typeof(IntPtr) });

        private static readonly MethodInfo LuaPushString = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pushstring),
            new[] { typeof(IntPtr), typeof(string) });

        internal static Expression BuildPopArgument(ParameterExpression luaStateParam, int index, Type parameterType)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;
            MethodInfo popMethod = GetPopMethod(targetType);
            if (popMethod == null)
            {
                Expression popObject = Expression.Call(
                    PopTargetMethod,
                    luaStateParam,
                    Expression.Constant(index),
                    Expression.Constant(targetType));
                return Expression.Convert(popObject, parameterType);
            }

            Expression popValue = Expression.Call(popMethod, luaStateParam, Expression.Constant(index));
            return parameterType == targetType
                ? popValue
                : Expression.Convert(popValue, parameterType);
        }

        internal static Expression BuildPushReturn(ParameterExpression luaStateParam, Type returnType, Expression valueExpression)
        {
            if (returnType == typeof(void))
            {
                return Expression.Constant(0);
            }

            Type unwrapped = Nullable.GetUnderlyingType(returnType) ?? returnType;

            if (unwrapped == typeof(bool))
            {
                return Expression.Block(
                    Expression.Call(
                        LuaPushBoolean,
                        luaStateParam,
                        Expression.Condition(valueExpression, Expression.Constant(1), Expression.Constant(0))),
                    Expression.Constant(1));
            }

            if (unwrapped == typeof(int) || unwrapped == typeof(char) || unwrapped == typeof(byte)
                || unwrapped == typeof(sbyte) || unwrapped == typeof(short) || unwrapped == typeof(ushort))
            {
                return Expression.Block(
                    Expression.Call(
                        LuaPushInteger,
                        luaStateParam,
                        Expression.Convert(valueExpression, typeof(long))),
                    Expression.Constant(1));
            }

            if (unwrapped == typeof(uint))
            {
                return Expression.Block(
                    Expression.Call(
                        LuaPushInteger,
                        luaStateParam,
                        Expression.Convert(valueExpression, typeof(long))),
                    Expression.Constant(1));
            }

            if (unwrapped == typeof(long) || unwrapped == typeof(ulong))
            {
                return Expression.Block(
                    Expression.Call(
                        LuaPushInteger,
                        luaStateParam,
                        Expression.Convert(valueExpression, typeof(long))),
                    Expression.Constant(1));
            }

            if (unwrapped == typeof(float) || unwrapped == typeof(double))
            {
                return Expression.Block(
                    Expression.Call(
                        LuaPushNumber,
                        luaStateParam,
                        Expression.Convert(valueExpression, typeof(double))),
                    Expression.Constant(1));
            }

            if (unwrapped == typeof(string))
            {
                return Expression.Condition(
                    Expression.Equal(valueExpression, Expression.Constant(null, typeof(string))),
                    Expression.Block(
                        Expression.Call(LuaPushNil, luaStateParam),
                        Expression.Constant(1)),
                    Expression.Block(
                        Expression.Call(LuaPushString, luaStateParam, valueExpression),
                        Expression.Constant(1)));
            }

            return Expression.Call(
                PushReturnMethod,
                luaStateParam,
                Expression.Constant(returnType),
                Expression.Convert(valueExpression, typeof(object)));
        }

        private static MethodInfo GetPopMethod(Type targetType)
        {
            if (targetType == typeof(bool))
            {
                return PopBoolMethod;
            }

            if (targetType == typeof(char))
            {
                return PopCharMethod;
            }

            if (targetType == typeof(byte))
            {
                return PopByteMethod;
            }

            if (targetType == typeof(sbyte))
            {
                return PopSByteMethod;
            }

            if (targetType == typeof(short))
            {
                return PopInt16Method;
            }

            if (targetType == typeof(ushort))
            {
                return PopUInt16Method;
            }

            if (targetType == typeof(int))
            {
                return PopInt32Method;
            }

            if (targetType == typeof(uint))
            {
                return PopUInt32Method;
            }

            if (targetType == typeof(long))
            {
                return PopInt64Method;
            }

            if (targetType == typeof(ulong))
            {
                return PopUInt64Method;
            }

            if (targetType == typeof(float))
            {
                return PopSingleMethod;
            }

            if (targetType == typeof(double))
            {
                return PopDoubleMethod;
            }

            if (targetType == typeof(IntPtr))
            {
                return PopIntPtrMethod;
            }

            if (targetType == typeof(UIntPtr))
            {
                return PopUIntPtrMethod;
            }

            if (targetType == typeof(string))
            {
                return PopStringMethod;
            }

            return null;
        }
    }
}
