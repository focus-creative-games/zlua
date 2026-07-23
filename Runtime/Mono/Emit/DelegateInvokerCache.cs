using System;
using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// Typed Expression-compiled invokers for Lua userdata <c>__call</c> on delegates.
    /// Avoids <see cref="Delegate.DynamicInvoke"/> and per-call <c>GetMethod("Invoke")</c>.
    /// </summary>
    internal static class DelegateInvokerCache
    {
        private static readonly ConcurrentDictionary<Type, Func<Delegate, IntPtr, int>> Cache =
            new ConcurrentDictionary<Type, Func<Delegate, IntPtr, int>>();

        internal static int Invoke(Delegate del, IntPtr L)
        {
            if (del == null)
            {
                LuaCallbackBoundary.Throw("zlua: __call expects delegate userdata");
                return 0;
            }

            Type delegateType = del.GetType();
            Func<Delegate, IntPtr, int> invoker = Cache.GetOrAdd(delegateType, BuildInvoker);
            return invoker(del, L);
        }

        private static Func<Delegate, IntPtr, int> BuildInvoker(Type delegateType)
        {
            MethodInfo invoke = delegateType.GetMethod("Invoke");
            if (invoke == null)
            {
                return (_, __) =>
                {
                    LuaCallbackBoundary.Throw("zlua: delegate Invoke method not found");
                    return 0;
                };
            }

            ParameterInfo[] parameters = invoke.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!BridgeMarshaling.IsSupportedParameter(parameters[i]))
                {
                    return BuildReflectiveInvoker(invoke, parameters);
                }
            }

            if (invoke.ReturnType != typeof(void) && !BridgeMarshaling.IsSupportedType(invoke.ReturnType))
            {
                return BuildReflectiveInvoker(invoke, parameters);
            }

            return BuildTypedInvoker(delegateType, invoke, parameters);
        }

        private static Func<Delegate, IntPtr, int> BuildTypedInvoker(
            Type delegateType,
            MethodInfo invoke,
            ParameterInfo[] parameters)
        {
            ParameterExpression delParam = Expression.Parameter(typeof(Delegate), "del");
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");
            var exprs = new System.Collections.Generic.List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(parameters.Length),
                    Expression.Constant(2)),
            };

            Expression typedDel = Expression.Convert(delParam, delegateType);
            var argExprs = new Expression[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                Expression popped = Expression.Call(
                    EmitMethods.PopArg,
                    L,
                    Expression.Constant(i + 2),
                    Expression.Constant(paramType, typeof(Type)));
                argExprs[i] = Expression.Convert(popped, paramType);
            }

            Expression call = Expression.Call(typedDel, invoke, argExprs);
            if (invoke.ReturnType == typeof(void))
            {
                exprs.Add(call);
                exprs.Add(Expression.Constant(0));
            }
            else
            {
                ParameterExpression result = Expression.Variable(invoke.ReturnType, "result");
                exprs.Add(Expression.Assign(result, call));
                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(invoke.ReturnType, typeof(Type)),
                    Expression.Convert(result, typeof(object))));
                return Expression.Lambda<Func<Delegate, IntPtr, int>>(
                    Expression.Block(new[] { result }, exprs),
                    delParam,
                    L).Compile();
            }

            return Expression.Lambda<Func<Delegate, IntPtr, int>>(
                Expression.Block(exprs),
                delParam,
                L).Compile();
        }

        /// <summary>
        /// Fallback for byref / unsupported signatures: cached MethodInfo.Invoke (still faster than DynamicInvoke).
        /// </summary>
        private static Func<Delegate, IntPtr, int> BuildReflectiveInvoker(
            MethodInfo invoke,
            ParameterInfo[] parameters)
        {
            Type[] paramTypes = new Type[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                paramTypes[i] = parameters[i].ParameterType;
            }

            Type returnType = invoke.ReturnType;
            return (del, L) =>
            {
                BridgeMarshaling.ValidateExactArgCount(L, parameters.Length, 2);
                var args = new object[parameters.Length];
                for (int i = 0; i < parameters.Length; i++)
                {
                    args[i] = TypedMarshal.PopObject(L, i + 2, paramTypes[i]);
                }

                object result = invoke.Invoke(del, args);
                if (returnType == typeof(void))
                {
                    return 0;
                }

                TypedMarshal.PushObject(L, result, returnType);
                return 1;
            };
        }
    }
}
