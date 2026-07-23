using System;
using System.Linq.Expressions;
using System.Reflection;
using ZLua;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.DelegateImpl
{
    /// <summary>
    /// Expression trees for C# → Lua compiled delegate bridges (typed push/pop without object[]).
    /// </summary>
    internal static class CSharpToLuaBridgeExpressionBuilder
    {
        private static readonly MethodInfo PushObjectMethod = typeof(TypedMarshal).GetMethod(
            nameof(TypedMarshal.PushObject),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopObjectMethod = typeof(TypedMarshal).GetMethod(
            nameof(TypedMarshal.PopObject),
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

            if (parameterType != targetType)
            {
                return Expression.IfThenElse(
                    Expression.Property(valueExpression, nameof(Nullable<int>.HasValue)),
                    BuildPushArgument(
                        luaStateParam,
                        Expression.Property(valueExpression, nameof(Nullable<int>.Value)),
                        targetType),
                    Expression.Call(LuaPushNil, luaStateParam));
            }

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
                PushObjectMethod,
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

            Expression popResult;
            if (unwrapped == typeof(bool))
            {
                popResult = Expression.NotEqual(
                    Expression.Call(LuaToBoolean, luaStateParam, Expression.Constant(-1)),
                    Expression.Constant(0));
            }
            else if (unwrapped == typeof(char))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(char));
            }
            else if (unwrapped == typeof(byte))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(byte));
            }
            else if (unwrapped == typeof(sbyte))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(sbyte));
            }
            else if (unwrapped == typeof(short))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(short));
            }
            else if (unwrapped == typeof(ushort))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(ushort));
            }
            else if (unwrapped == typeof(int))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(int));
            }
            else if (unwrapped == typeof(uint))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(uint));
            }
            else if (unwrapped == typeof(long))
            {
                popResult = Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1));
            }
            else if (unwrapped == typeof(ulong))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                    typeof(ulong));
            }
            else if (unwrapped == typeof(float))
            {
                popResult = Expression.Convert(
                    Expression.Call(LuaToNumber, luaStateParam, Expression.Constant(-1)),
                    typeof(float));
            }
            else if (unwrapped == typeof(double))
            {
                popResult = Expression.Call(LuaToNumber, luaStateParam, Expression.Constant(-1));
            }
            else if (unwrapped == typeof(IntPtr))
            {
                popResult = Expression.New(
                    typeof(IntPtr).GetConstructor(new[] { typeof(long) }),
                    Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)));
            }
            else if (unwrapped == typeof(UIntPtr))
            {
                popResult = Expression.New(
                    typeof(UIntPtr).GetConstructor(new[] { typeof(ulong) }),
                    Expression.Convert(
                        Expression.Call(LuaToInteger, luaStateParam, Expression.Constant(-1)),
                        typeof(ulong)));
            }
            else if (unwrapped == typeof(string))
            {
                popResult = Expression.Call(ToStringMethod, luaStateParam, Expression.Constant(-1));
            }
            else
            {
                popResult = Expression.Call(
                    PopObjectMethod,
                    luaStateParam,
                    Expression.Constant(-1),
                    Expression.Constant(returnType));
            }

            if (popResult.Type == returnType)
            {
                return popResult;
            }

            return Expression.Convert(popResult, returnType);
        }
    }
}
