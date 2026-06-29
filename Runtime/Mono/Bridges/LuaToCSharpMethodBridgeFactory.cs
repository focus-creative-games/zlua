using System;
using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Compiles Lua → C# bridges for non-overloaded default-marshal methods at registration time.
    /// </summary>
    internal static class LuaToCSharpMethodBridgeFactory
    {
        private readonly struct FactoryKey : IEquatable<FactoryKey>
        {
            internal readonly MethodInfo Method;
            internal readonly bool IsStatic;

            internal FactoryKey(MethodInfo method, bool isStatic)
            {
                Method = method;
                IsStatic = isStatic;
            }

            public bool Equals(FactoryKey other)
            {
                return Method == other.Method && IsStatic == other.IsStatic;
            }

            public override bool Equals(object obj)
            {
                return obj is FactoryKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Method?.MetadataToken ?? 0) * 397) ^ (IsStatic ? 1 : 0);
                }
            }
        }

        private static readonly ConcurrentDictionary<FactoryKey, LuaCSFunction> Cache =
            new ConcurrentDictionary<FactoryKey, LuaCSFunction>();

        private static readonly MethodInfo ValidateExactArgCountMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.ValidateExactArgCount),
            BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(IntPtr), typeof(int), typeof(int), typeof(MethodInfo) },
            modifiers: null);

        private static readonly MethodInfo PushReturnMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PushReturn),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ToLuaErrorMethod = typeof(LuaCallbackBoundary).GetMethod(
            nameof(LuaCallbackBoundary.ToLuaError),
            BindingFlags.NonPublic | BindingFlags.Static);

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

        private static readonly MethodInfo EnterLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.EnterLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo LeaveLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.LeaveLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool TryCreate(MethodInfo method, bool isStatic, out LuaCSFunction invoker)
        {
            invoker = null;
            if (!CanFastInvoke(method, isStatic) || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                invoker = Cache.GetOrAdd(new FactoryKey(method, isStatic), key => Compile(key.Method, key.IsStatic));
                return invoker != null;
            }
            catch
            {
                return false;
            }
        }

        private static bool EnsureHelperMethods()
        {
            return ValidateExactArgCountMethod != null
                && PushReturnMethod != null
                && ToLuaErrorMethod != null
                && EnterLuaToCSharpMethod != null
                && LeaveLuaToCSharpMethod != null
                && PopTargetMethod != null;
        }

        internal static bool CanFastInvoke(MethodInfo method, bool isStatic)
        {
            if (method == null || method.IsAbstract || method.ContainsGenericParameters)
            {
                return false;
            }

            if (!isStatic && method.DeclaringType == typeof(Array))
            {
                return false;
            }

            if (HasNonDefaultMarshal(method))
            {
                return false;
            }

            if (PointerMethodInvoker.RequiresPointerInvoke(method)
                || PointerMethodInvoker.HasUnsafeReflectionSignature(method))
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                ParameterInfo parameter = parameters[i];
                if (parameter.ParameterType.IsByRef
                    || parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false)
                    || parameter.HasDefaultValue)
                {
                    return false;
                }

                if (!IsFastType(parameter.ParameterType))
                {
                    return false;
                }
            }

            Type returnType = method.ReturnType;
            return returnType == typeof(void) || IsFastType(returnType);
        }

        private static bool HasNonDefaultMarshal(MethodInfo method)
        {
            if (LuaMarshalAsResolver.ResolveReturn(method, LuaMarshalDirection.CSharpToLua) != LuaMarshalType.Default)
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (LuaMarshalAsResolver.ResolveParameter(
                        parameters[i],
                        method,
                        LuaMarshalDirection.LuaToCSharp) != LuaMarshalType.Default)
                {
                    return true;
                }
            }

            return false;
        }

        internal static bool CanFastType(Type type)
        {
            return IsFastType(type);
        }

        private static bool IsFastType(Type type)
        {
            if (Nullable.GetUnderlyingType(type) != null)
            {
                return false;
            }

            Type targetType = type;
            if (targetType.IsByRef || targetType == typeof(object) || targetType == typeof(TypedReference))
            {
                return false;
            }

            if (targetType.IsPrimitive
                || targetType == typeof(string)
                || targetType == typeof(decimal)
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr))
            {
                return true;
            }

            if (targetType.IsEnum || ValueTypeMarshaling.IsStructType(targetType))
            {
                return true;
            }

            if (targetType.IsArray)
            {
                return true;
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                return true;
            }

            return targetType.IsClass || targetType.IsInterface;
        }

        private static LuaCSFunction Compile(MethodInfo method, bool isStatic)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

            int argStartIndex = isStatic ? 1 : 2;
            ParameterInfo[] parameters = method.GetParameters();

            Expression validateArgs = Expression.Call(
                ValidateExactArgCountMethod,
                luaStateParam,
                Expression.Constant(argStartIndex),
                Expression.Constant(parameters.Length),
                Expression.Constant(method));

            Expression targetExpression = null;
            if (!isStatic)
            {
                targetExpression = Expression.Convert(
                    Expression.Call(
                        PopTargetMethod,
                        luaStateParam,
                        Expression.Constant(1),
                        Expression.Constant(method.DeclaringType)),
                    method.DeclaringType);
            }

            Expression[] callArguments = new Expression[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                callArguments[i] = LuaToCSharpBridgeExpressionBuilder.BuildPopArgument(
                    luaStateParam,
                    argStartIndex + i,
                    parameters[i].ParameterType);
            }

            Expression callExpression = isStatic
                ? Expression.Call(method, callArguments)
                : Expression.Call(targetExpression, method, callArguments);

            Expression setReturnCount;
            Type returnType = method.ReturnType;
            if (returnType == typeof(void))
            {
                setReturnCount = Expression.Block(callExpression, Expression.Constant(0));
            }
            else
            {
                setReturnCount = LuaToCSharpBridgeExpressionBuilder.BuildPushReturn(
                    luaStateParam,
                    returnType,
                    callExpression);
            }

            Expression enterScope = Expression.Call(EnterLuaToCSharpMethod);
            Expression leaveScope = Expression.Call(LeaveLuaToCSharpMethod);

            Expression tryBody = Expression.Block(
                validateArgs,
                Expression.TryFinally(
                    Expression.Block(enterScope, setReturnCount),
                    leaveScope));

            Expression tryCatch = Expression.TryCatch(
                tryBody,
                Expression.Catch(exceptionParam, Expression.Call(ToLuaErrorMethod, luaStateParam, exceptionParam)));

            return Expression.Lambda<LuaCSFunction>(tryCatch, luaStateParam).Compile();
        }
    }
}
