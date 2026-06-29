using System;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Expression trees for C# → Lua compiled delegate bridges (P1: typed push/pop without object[]).
    /// </summary>
    internal static class CSharpToLuaBridgeExpressionBuilder
    {
        private static readonly MethodInfo PushArgumentValueMethod = typeof(TypeMethodRegistration).GetMethod(
            nameof(TypeMethodRegistration.PushArgumentValue),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopReturnMethod = typeof(LuaInvokeMarshaling).GetMethod(
            nameof(LuaInvokeMarshaling.PopReturn),
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

        private static readonly MethodInfo LuaToBoolean = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_toboolean),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo LuaToInteger = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_tointeger),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo LuaToNumber = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_tonumber),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo ToStringMethod = typeof(LuaDllExtension).GetMethod(
            nameof(LuaDllExtension.tostring),
            new[] { typeof(IntPtr), typeof(int) });

        internal static Expression BuildPushArgument(
            ParameterExpression luaStateParam,
            Expression valueExpression,
            Type parameterType)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;

            if (targetType == typeof(bool))
            {
                return Expression.Call(
                    LuaPushBoolean,
                    luaStateParam,
                    Expression.Condition(valueExpression, Expression.Constant(1), Expression.Constant(0)));
            }

            if (targetType == typeof(int) || targetType == typeof(char) || targetType == typeof(byte)
                || targetType == typeof(sbyte) || targetType == typeof(short) || targetType == typeof(ushort))
            {
                return Expression.Call(
                    LuaPushInteger,
                    luaStateParam,
                    Expression.Convert(valueExpression, typeof(long)));
            }

            if (targetType == typeof(uint))
            {
                return Expression.Call(
                    LuaPushInteger,
                    luaStateParam,
                    Expression.Convert(valueExpression, typeof(long)));
            }

            if (targetType == typeof(long) || targetType == typeof(ulong))
            {
                return Expression.Call(
                    LuaPushInteger,
                    luaStateParam,
                    Expression.Convert(valueExpression, typeof(long)));
            }

            if (targetType == typeof(float) || targetType == typeof(double))
            {
                return Expression.Call(
                    LuaPushNumber,
                    luaStateParam,
                    Expression.Convert(valueExpression, typeof(double)));
            }

            if (targetType == typeof(string))
            {
                return Expression.Condition(
                    Expression.Equal(valueExpression, Expression.Constant(null, typeof(string))),
                    Expression.Call(LuaPushNil, luaStateParam),
                    Expression.Block(
                        Expression.Call(LuaPushString, luaStateParam, valueExpression),
                        Expression.Empty()));
            }

            if (targetType == typeof(IntPtr))
            {
                return Expression.Call(
                    LuaPushInteger,
                    luaStateParam,
                    Expression.Call(valueExpression, typeof(IntPtr).GetMethod(nameof(IntPtr.ToInt64), Type.EmptyTypes)));
            }

            if (targetType == typeof(UIntPtr))
            {
                return Expression.Call(
                    LuaPushInteger,
                    luaStateParam,
                    Expression.Convert(
                        Expression.Convert(valueExpression, typeof(ulong)),
                        typeof(long)));
            }

            return Expression.Call(
                PushArgumentValueMethod,
                luaStateParam,
                Expression.Convert(valueExpression, typeof(object)),
                Expression.Constant(parameterType));
        }

        internal static Expression BuildPopReturn(
            ParameterExpression luaStateParam,
            MethodInfo invokeMethod,
            Type returnType)
        {
            if (returnType == typeof(void))
            {
                return Expression.Empty();
            }

            Type unwrapped = Nullable.GetUnderlyingType(returnType) ?? returnType;

            if (unwrapped == typeof(bool))
            {
                return Expression.NotEqual(
                    Expression.Call(LuaToBoolean, luaStateParam, Expression.Constant(-1)),
                    Expression.Constant(0));
            }

            if (unwrapped == typeof(char))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(char));
            }

            if (unwrapped == typeof(byte))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(byte));
            }

            if (unwrapped == typeof(sbyte))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(sbyte));
            }

            if (unwrapped == typeof(short))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(short));
            }

            if (unwrapped == typeof(ushort))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(ushort));
            }

            if (unwrapped == typeof(int))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(int));
            }

            if (unwrapped == typeof(uint))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(uint));
            }

            if (unwrapped == typeof(long))
            {
                return Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1));
            }

            if (unwrapped == typeof(ulong))
            {
                return Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(ulong));
            }

            if (unwrapped == typeof(float))
            {
                return Expression.Convert(
                    Expression.Call(LuaToNumber, luaStateParam, Expression.Constant(-1)),
                    typeof(float));
            }

            if (unwrapped == typeof(double))
            {
                return Expression.Call(LuaToNumber, luaStateParam, Expression.Constant(-1));
            }

            if (unwrapped == typeof(IntPtr))
            {
                return Expression.New(
                    typeof(IntPtr).GetConstructor(new[] { typeof(long) }),
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)));
            }

            if (unwrapped == typeof(UIntPtr))
            {
                return Expression.New(
                    typeof(UIntPtr).GetConstructor(new[] { typeof(ulong) }),
                    Expression.Convert(
                        Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                        typeof(ulong)));
            }

            if (unwrapped == typeof(string))
            {
                return Expression.Call(ToStringMethod, luaStateParam, Expression.Constant(-1));
            }

            return Expression.Call(
                PopReturnMethod,
                luaStateParam,
                Expression.Constant(invokeMethod, typeof(MethodInfo)),
                Expression.Constant(returnType),
                Expression.Constant(-1));
        }
    }
}
