using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    internal static class DynamicBridgeFactory
    {
        private static readonly ConcurrentDictionary<MethodInfo, Func<LuaMethod, Delegate>> FactoryCache =
            new ConcurrentDictionary<MethodInfo, Func<LuaMethod, Delegate>>();

        private static readonly MethodInfo InvokeVoidWithArgsMethod = typeof(LuaCallInvoker).GetMethod(
            nameof(LuaCallInvoker.InvokeVoidWithArgs),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo InvokeWithArgsMethod = typeof(LuaCallInvoker).GetMethod(
            nameof(LuaCallInvoker.InvokeWithArgs),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static Delegate CreateDelegate(Type delegateType, LuaMethod target)
        {
            if (delegateType == null)
            {
                throw new ArgumentNullException(nameof(delegateType));
            }

            if (target == null)
            {
                throw new ArgumentNullException(nameof(target));
            }

            MethodInfo invokeMethod = delegateType.GetMethod("Invoke");
            if (invokeMethod == null)
            {
                throw new InvalidOperationException($"Delegate type '{delegateType.FullName}' has no Invoke method.");
            }

            Func<LuaMethod, Delegate> factory = FactoryCache.GetOrAdd(invokeMethod, _ => CompileFactory(invokeMethod, delegateType));
            return factory(target);
        }

        private static Func<LuaMethod, Delegate> CompileFactory(MethodInfo invokeMethod, Type delegateType)
        {
            ParameterInfo[] parameters = invokeMethod.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i].ParameterType.IsByRef)
                {
                    throw new NotSupportedException(
                        $"zlua: delegate invoke parameter '{parameters[i].Name}' with ref/out is not supported.");
                }
            }

            ParameterExpression targetParam = Expression.Parameter(typeof(LuaMethod), "target");
            ParameterExpression[] argExprs = parameters
                .Select(p => Expression.Parameter(p.ParameterType, p.Name))
                .ToArray();
            Type[] paramTypes = Array.ConvertAll(parameters, p => p.ParameterType);
            Type returnType = invokeMethod.ReturnType;

            ParameterExpression argsVar = Expression.Variable(typeof(object[]), "args");
            var statements = new List<Expression>
            {
                Expression.Assign(argsVar, Expression.NewArrayBounds(typeof(object), Expression.Constant(parameters.Length))),
            };

            for (int i = 0; i < parameters.Length; i++)
            {
                Expression boxedValue = argExprs[i];
                if (parameters[i].ParameterType.IsValueType)
                {
                    boxedValue = Expression.Convert(boxedValue, typeof(object));
                }
                else
                {
                    boxedValue = Expression.TypeAs(boxedValue, typeof(object));
                }

                statements.Add(Expression.Assign(Expression.ArrayAccess(argsVar, Expression.Constant(i)), boxedValue));
            }

            Expression bodyExpression;
            if (returnType == typeof(void))
            {
                statements.Add(Expression.Call(
                    InvokeVoidWithArgsMethod,
                    targetParam,
                    Expression.Constant(paramTypes),
                    argsVar));
                bodyExpression = Expression.Block(new[] { argsVar }, statements);
            }
            else
            {
                MethodCallExpression call = Expression.Call(
                    InvokeWithArgsMethod,
                    targetParam,
                    Expression.Constant(returnType),
                    Expression.Constant(paramTypes),
                    argsVar);
                Expression result = returnType.IsValueType
                    ? Expression.Unbox(call, returnType)
                    : Expression.Convert(call, returnType);
                statements.Add(result);
                bodyExpression = Expression.Block(new[] { argsVar }, statements);
            }

            LambdaExpression innerDelegate = Expression.Lambda(delegateType, bodyExpression, argExprs);
            LambdaExpression factory = Expression.Lambda<Func<LuaMethod, Delegate>>(
                Expression.Convert(innerDelegate, typeof(Delegate)),
                targetParam);

            return (Func<LuaMethod, Delegate>)factory.Compile();
        }
    }
}
