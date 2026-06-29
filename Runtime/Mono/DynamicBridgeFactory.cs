using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    internal static class DynamicBridgeFactory
    {
        private readonly struct FactoryKey : IEquatable<FactoryKey>
        {
            internal readonly MethodInfo InvokeMethod;
            internal readonly Type DelegateType;

            internal FactoryKey(MethodInfo invokeMethod, Type delegateType)
            {
                InvokeMethod = invokeMethod;
                DelegateType = delegateType;
            }

            public bool Equals(FactoryKey other)
            {
                return InvokeMethod == other.InvokeMethod && DelegateType == other.DelegateType;
            }

            public override bool Equals(object obj)
            {
                return obj is FactoryKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((InvokeMethod?.MetadataToken ?? 0) * 397)
                        ^ (DelegateType?.MetadataToken ?? 0);
                }
            }
        }

        private static readonly ConcurrentDictionary<FactoryKey, Func<LuaMethod, Delegate>> FactoryCache =
            new ConcurrentDictionary<FactoryKey, Func<LuaMethod, Delegate>>();

        private static readonly MethodInfo LuaGetTop = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_gettop),
            new[] { typeof(IntPtr) });

        private static readonly MethodInfo LuaSetTop = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_settop),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo LuaRawGetI = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_rawgeti),
            new[] { typeof(IntPtr), typeof(int), typeof(long) });

        private static readonly MethodInfo LuaPCall = typeof(LuaDll).GetMethod(
            nameof(LuaDll.lua_pcall),
            new[] { typeof(IntPtr), typeof(int), typeof(int), typeof(int) });

        private static readonly MethodInfo EnterManagedPcall = typeof(LuaPrintBuffer).GetMethod(
            nameof(LuaPrintBuffer.EnterManagedPcall),
            BindingFlags.Public | BindingFlags.Static);

        private static readonly MethodInfo LeaveManagedPcall = typeof(LuaPrintBuffer).GetMethod(
            nameof(LuaPrintBuffer.LeaveManagedPcall),
            BindingFlags.Public | BindingFlags.Static);

        private static readonly MethodInfo PushErrorHandlerToStack = typeof(LuaMethod).GetMethod(
            nameof(LuaMethod.PushErrorHandlerToStack),
            BindingFlags.NonPublic | BindingFlags.Instance);

        private static readonly MethodInfo ToStringMethod = typeof(LuaDllExtension).GetMethod(
            nameof(LuaDllExtension.tostring),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly PropertyInfo LuaStateProperty = typeof(LuaMethod).GetProperty(nameof(LuaMethod.LuaState));
        private static readonly PropertyInfo RefIndexProperty = typeof(LuaMethod).GetProperty(nameof(LuaMethod.RefIndex));

        internal static void Warmup(Type delegateType)
        {
            if (delegateType == null || !typeof(Delegate).IsAssignableFrom(delegateType))
            {
                return;
            }

            MethodInfo invokeMethod = delegateType.GetMethod("Invoke");
            if (invokeMethod == null)
            {
                return;
            }

            EnsureFactory(invokeMethod, delegateType);
        }

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

            Func<LuaMethod, Delegate> factory = EnsureFactory(invokeMethod, delegateType);
            return factory(target);
        }

        private static Func<LuaMethod, Delegate> EnsureFactory(MethodInfo invokeMethod, Type delegateType)
        {
            return FactoryCache.GetOrAdd(
                new FactoryKey(invokeMethod, delegateType),
                _ => CompileFactory(invokeMethod, delegateType));
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
            Type returnType = invokeMethod.ReturnType;

            ParameterExpression luaStateVar = Expression.Parameter(typeof(IntPtr), "L");
            ParameterExpression oldTopVar = Expression.Parameter(typeof(int), "oldTop");
            ParameterExpression pcallResultVar = Expression.Parameter(typeof(int), "pcallResult");
            ParameterExpression functionTypeVar = Expression.Parameter(typeof(LuaDataType), "functionType");

            Expression getLuaState = Expression.Assign(
                luaStateVar,
                Expression.Property(targetParam, LuaStateProperty));
            Expression saveTop = Expression.Assign(oldTopVar, Expression.Call(LuaGetTop, luaStateVar));

            var invokeStatements = new System.Collections.Generic.List<Expression>
            {
                Expression.Call(targetParam, PushErrorHandlerToStack),
                Expression.Assign(
                    functionTypeVar,
                    Expression.Call(
                        LuaRawGetI,
                        luaStateVar,
                        Expression.Constant(LuaConsts.LuaRegistryIndex),
                        Expression.Convert(
                            Expression.Property(targetParam, RefIndexProperty),
                            typeof(long)))),
                Expression.IfThen(
                    Expression.NotEqual(
                        functionTypeVar,
                        Expression.Constant(LuaDataType.Function)),
                    Expression.Throw(
                        Expression.New(
                            typeof(InvalidOperationException).GetConstructor(new[] { typeof(string) }),
                            Expression.Constant("Lua function reference is invalid.")))),
            };

            for (int i = 0; i < parameters.Length; i++)
            {
                invokeStatements.Add(CSharpToLuaBridgeExpressionBuilder.BuildPushArgument(
                    luaStateVar,
                    argExprs[i],
                    parameters[i].ParameterType));
            }

            int nArgs = parameters.Length;
            int nRet = returnType == typeof(void) ? 0 : 1;
            invokeStatements.Add(Expression.Assign(
                pcallResultVar,
                Expression.Call(
                    LuaPCall,
                    luaStateVar,
                    Expression.Constant(nArgs),
                    Expression.Constant(nRet),
                    Expression.Add(oldTopVar, Expression.Constant(1)))));

            invokeStatements.Add(Expression.IfThen(
                Expression.NotEqual(pcallResultVar, Expression.Constant(0)),
                Expression.Throw(
                    Expression.New(
                        typeof(Exception).GetConstructor(new[] { typeof(string) }),
                        Expression.Call(ToStringMethod, luaStateVar, Expression.Constant(-1))))));

            Expression returnExpression;
            if (returnType == typeof(void))
            {
                returnExpression = Expression.Empty();
            }
            else
            {
                returnExpression = CSharpToLuaBridgeExpressionBuilder.BuildPopReturn(
                    luaStateVar,
                    invokeMethod,
                    returnType);
            }

            Expression tryBody = Expression.Block(
                invokeStatements.Concat(new[] { returnExpression }));

            Expression body = Expression.Block(
                new[] { luaStateVar, oldTopVar, pcallResultVar, functionTypeVar },
                Expression.TryFinally(
                    Expression.Block(
                        getLuaState,
                        saveTop,
                        Expression.Call(EnterManagedPcall),
                        tryBody),
                    Expression.Block(
                        Expression.Call(LuaSetTop, luaStateVar, oldTopVar),
                        Expression.Call(LeaveManagedPcall))));

            LambdaExpression innerDelegate = Expression.Lambda(
                delegateType,
                body,
                argExprs);
            LambdaExpression factory = Expression.Lambda<Func<LuaMethod, Delegate>>(
                Expression.Convert(innerDelegate, typeof(Delegate)),
                targetParam);

            return (Func<LuaMethod, Delegate>)factory.Compile();
        }
    }
}
