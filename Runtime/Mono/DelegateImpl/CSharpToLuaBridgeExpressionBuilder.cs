using System;
using System.Collections.Generic;
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

        private static readonly MethodInfo PushByteArrayMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PushByteArray),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PushStringMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PushString),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PushObjectMarshalMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PushObject),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PushByRefOpaqueMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PushByRefOpaque),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo WriteBackByRefOpaqueMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.WriteBackByRefOpaque),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PushStructMethodDefinition = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PushStruct),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopByteArrayMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PopByteArray),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopStringMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PopString),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopObjectMarshalMethod = typeof(CSharpToLuaMarshaling).GetMethod(
            nameof(CSharpToLuaMarshaling.PopObject),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static LuaMarshalType ResolveMarshalType(ICustomAttributeProvider provider, Type declaredType)
        {
            if (provider != null && provider.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                LuaMarshalAsAttribute attr = ((MemberInfo)provider as MemberInfo) != null
                    ? ((MemberInfo)provider).GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false)
                    : null;
                if (attr == null && provider is ParameterInfo pi)
                {
                    attr = pi.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
                }

                if (attr != null)
                {
                    return NormalizeMarshalType(attr.LuaMarshalType, declaredType);
                }
            }

            return LuaMarshalType.Default;
        }

        internal static LuaMarshalType ResolveParameterMarshalType(ParameterInfo parameter)
        {
            Type declared = parameter.ParameterType.IsByRef
                ? parameter.ParameterType.GetElementType()
                : parameter.ParameterType;
            if (parameter.ParameterType.IsByRef)
            {
                // C#→Lua byref defaults to OpaqueValue (spec §2.5 / 09-FUNCTION).
                return LuaMarshalType.OpaqueValue;
            }

            LuaMarshalAsAttribute attr = parameter.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (attr != null && attr.LuaMarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(attr.LuaMarshalType, declared);
            }

            if (LuaMarshalAsXmlRegistry.TryGetParameterRule(parameter, parameter.Member as MethodBase, out LuaMarshalAsXmlRule paramRule)
                && paramRule.MarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(paramRule.MarshalType, declared);
            }

            LuaMarshalAsAttribute typeAttr = GetTypeLevelAttribute(declared);
            if (typeAttr != null && typeAttr.LuaMarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(typeAttr.LuaMarshalType, declared);
            }

            if (LuaMarshalAsXmlRegistry.TryGetTypeRule(declared, out LuaMarshalAsXmlRule typeRule)
                && typeRule.MarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(typeRule.MarshalType, declared);
            }

            return LuaMarshalType.Default;
        }

        internal static LuaMarshalType ResolveReturnMarshalType(MethodInfo invokeMethod)
        {
            ParameterInfo ret = invokeMethod.ReturnParameter;
            LuaMarshalAsAttribute attr = ret?.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (attr != null && attr.LuaMarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(attr.LuaMarshalType, invokeMethod.ReturnType);
            }

            if (LuaMarshalAsXmlRegistry.TryGetReturnRule(invokeMethod, out LuaMarshalAsXmlRule returnRule)
                && returnRule.MarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(returnRule.MarshalType, invokeMethod.ReturnType);
            }

            LuaMarshalAsAttribute typeAttr = GetTypeLevelAttribute(invokeMethod.ReturnType);
            if (typeAttr != null && typeAttr.LuaMarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(typeAttr.LuaMarshalType, invokeMethod.ReturnType);
            }

            if (LuaMarshalAsXmlRegistry.TryGetTypeRule(invokeMethod.ReturnType, out LuaMarshalAsXmlRule typeRule)
                && typeRule.MarshalType != LuaMarshalType.Default)
            {
                return NormalizeMarshalType(typeRule.MarshalType, invokeMethod.ReturnType);
            }

            return LuaMarshalType.Default;
        }

        private static LuaMarshalAsAttribute GetTypeLevelAttribute(Type declared)
        {
            if (declared == null)
            {
                return null;
            }

            if (declared.IsByRef)
            {
                declared = declared.GetElementType();
            }

            Type underlying = Nullable.GetUnderlyingType(declared);
            if (underlying != null)
            {
                declared = underlying;
            }

            // Type-level rules never apply to open or closed generic types (spec §1.1).
            if (declared == null || declared.IsGenericType)
            {
                return null;
            }

            return declared.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
        }

        private static LuaMarshalType NormalizeMarshalType(LuaMarshalType marshalType, Type declaredType)
        {
            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (declaredType == typeof(byte[]) || declaredType == typeof(string))
                    {
                        return LuaMarshalType.Bytes;
                    }

                    return LuaMarshalType.Default;
                case LuaMarshalType.UserData:
                    if (!declaredType.IsValueType || declaredType.IsEnum)
                    {
                        // reference types + string ok; structs also ok for UserData
                    }

                    if (declaredType.IsPrimitive && declaredType != typeof(IntPtr) && declaredType != typeof(UIntPtr))
                    {
                        return LuaMarshalType.Default;
                    }

                    return LuaMarshalType.UserData;
                case LuaMarshalType.OpaqueValue:
                    return LuaMarshalType.OpaqueValue;
                default:
                    return LuaMarshalType.Default;
            }
        }

        /// <summary>
        /// Builds push expression. When byref opaque, returns handle expression for write-back (or null).
        /// </summary>
        internal static Expression BuildPushArgument(
            ParameterExpression luaStateParam,
            Expression valueExpression,
            ParameterInfo parameter,
            out ParameterExpression opaqueHandleVar,
            out Expression writeBackExpression)
        {
            opaqueHandleVar = null;
            writeBackExpression = null;

            Type parameterType = parameter.ParameterType;
            LuaMarshalType marshalType = ResolveParameterMarshalType(parameter);

            if (parameterType.IsByRef)
            {
                Type elementType = parameterType.GetElementType();
                if (elementType == null || !elementType.IsValueType)
                {
                    throw new NotSupportedException(
                        $"zlua: delegate byref parameter '{parameter.Name}' must be a value type.");
                }

                opaqueHandleVar = Expression.Variable(typeof(IntPtr), parameter.Name + "_opaque");
                MethodInfo push = PushByRefOpaqueMethod.MakeGenericMethod(elementType);
                MethodInfo writeBack = WriteBackByRefOpaqueMethod.MakeGenericMethod(elementType);
                Expression pushCall = Expression.Call(push, luaStateParam, valueExpression);
                writeBackExpression = Expression.Call(writeBack, opaqueHandleVar, valueExpression);
                return Expression.Assign(opaqueHandleVar, pushCall);
            }

            if (marshalType == LuaMarshalType.OpaqueValue)
            {
                if (!parameterType.IsValueType)
                {
                    throw new NotSupportedException(
                        $"zlua: OpaqueValue on non-struct parameter '{parameter.Name}' is not supported.");
                }

                MethodInfo pushStruct = PushStructMethodDefinition.MakeGenericMethod(parameterType);
                return Expression.Call(
                    pushStruct,
                    luaStateParam,
                    valueExpression,
                    Expression.Constant(LuaMarshalType.OpaqueValue));
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                if (parameterType == typeof(byte[]))
                {
                    return Expression.Call(
                        PushByteArrayMethod,
                        luaStateParam,
                        valueExpression,
                        Expression.Constant(LuaMarshalType.Bytes));
                }

                if (parameterType == typeof(string))
                {
                    return Expression.Call(
                        PushStringMethod,
                        luaStateParam,
                        valueExpression,
                        Expression.Constant(LuaMarshalType.Bytes));
                }
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                if (parameterType == typeof(string))
                {
                    return Expression.Call(
                        PushStringMethod,
                        luaStateParam,
                        valueExpression,
                        Expression.Constant(LuaMarshalType.UserData));
                }

                return Expression.Call(
                    PushObjectMarshalMethod,
                    luaStateParam,
                    Expression.Convert(valueExpression, typeof(object)),
                    Expression.Constant(parameterType),
                    Expression.Constant(LuaMarshalType.UserData));
            }

            return BuildDefaultPush(luaStateParam, valueExpression, parameterType);
        }

        private static Expression BuildDefaultPush(
            ParameterExpression luaStateParam,
            Expression valueExpression,
            Type parameterType)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;

            if (parameterType != targetType)
            {
                return Expression.IfThenElse(
                    Expression.Property(valueExpression, nameof(Nullable<int>.HasValue)),
                    BuildDefaultPush(
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
                || targetType == typeof(sbyte) || targetType == typeof(short) || targetType == typeof(ushort)
                || targetType == typeof(uint) || targetType == typeof(long) || targetType == typeof(ulong))
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

            LuaMarshalType marshalType = ResolveReturnMarshalType(invokeMethod);
            if (marshalType == LuaMarshalType.Bytes)
            {
                if (returnType == typeof(byte[]))
                {
                    return Expression.Call(
                        PopByteArrayMethod,
                        luaStateParam,
                        Expression.Constant(-1),
                        Expression.Constant(LuaMarshalType.Bytes));
                }

                if (returnType == typeof(string))
                {
                    return Expression.Call(
                        PopStringMethod,
                        luaStateParam,
                        Expression.Constant(-1),
                        Expression.Constant(LuaMarshalType.Bytes));
                }
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                if (returnType == typeof(string))
                {
                    return Expression.Call(
                        PopStringMethod,
                        luaStateParam,
                        Expression.Constant(-1),
                        Expression.Constant(LuaMarshalType.UserData));
                }

                return Expression.Convert(
                    Expression.Call(
                        PopObjectMarshalMethod,
                        luaStateParam,
                        Expression.Constant(-1),
                        Expression.Constant(returnType),
                        Expression.Constant(LuaMarshalType.UserData)),
                    returnType);
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
