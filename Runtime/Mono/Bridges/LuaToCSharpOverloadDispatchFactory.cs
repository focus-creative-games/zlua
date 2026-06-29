using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Compiles overload dispatch: (argc, typeMask) fast table + CanConvert fallback chain (P1 · 4.6).
    /// Instance overloads pop <c>this</c> once then inline-invoke; static overloads delegate to per-overload fast invokers.
    /// </summary>
    internal static class LuaToCSharpOverloadDispatchFactory
    {
        private readonly struct DispatchKey : IEquatable<DispatchKey>
        {
            internal readonly int DispatchId;

            internal DispatchKey(int dispatchId)
            {
                DispatchId = dispatchId;
            }

            public bool Equals(DispatchKey other) => DispatchId == other.DispatchId;

            public override bool Equals(object obj) => obj is DispatchKey other && Equals(other);

            public override int GetHashCode() => DispatchId;
        }

        private static readonly System.Collections.Concurrent.ConcurrentDictionary<DispatchKey, LuaCSFunction> Cache =
            new System.Collections.Concurrent.ConcurrentDictionary<DispatchKey, LuaCSFunction>();

        private static readonly MethodInfo TryCanConvertArgumentsMethod = typeof(TypeMethodRegistration).GetMethod(
            nameof(TypeMethodRegistration.TryCanConvertArguments),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo TryInvokeFastMethod = typeof(OverloadTypeMask).GetMethod(
            nameof(OverloadTypeMask.TryInvokeFast),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ThrowNoOverloadMethod = typeof(LuaToCSharpOverloadDispatchFactory).GetMethod(
            nameof(ThrowNoOverload),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ToLuaErrorMethod = typeof(LuaCallbackBoundary).GetMethod(
            nameof(LuaCallbackBoundary.ToLuaError),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo InvokeCompiledInvokerMethod = typeof(LuaToCSharpOverloadDispatchFactory).GetMethod(
            nameof(InvokeCompiledInvoker),
            BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(LuaCSFunction), typeof(IntPtr) },
            modifiers: null);

        private static readonly MethodInfo ValidateExactArgCountMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.ValidateExactArgCount),
                        BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(IntPtr), typeof(int), typeof(int), typeof(MethodInfo) },
            modifiers: null);

        private static readonly MethodInfo PopTargetMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.PopTarget),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo EnterLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.EnterLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo LeaveLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.LeaveLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool TryCreate(
            TypeMethodRegistration.DispatchGroup group,
            int dispatchId,
            Func<int, MethodInfo> resolveMethod,
            Func<int, LuaCSFunction> resolveInvoker,
            out LuaCSFunction dispatch)
        {
            dispatch = null;
            if (group == null
                || group.IsConstructor
                || group.MethodIds == null
                || group.MethodIds.Length == 0
                || TryCanConvertArgumentsMethod == null
                || ThrowNoOverloadMethod == null
                || ToLuaErrorMethod == null
                || InvokeCompiledInvokerMethod == null
                || !EnsureHelperMethods(group.IsStatic))
            {
                return false;
            }

            var entries = new List<OverloadEntry>(group.MethodIds.Length);
            for (int i = 0; i < group.MethodIds.Length; i++)
            {
                int methodId = group.MethodIds[i];
                MethodInfo method = resolveMethod(methodId);
                if (method == null
                    || !LuaToCSharpMethodBridgeFactory.CanFastInvoke(method, group.IsStatic))
                {
                    return false;
                }

                LuaCSFunction invoker = null;
                if (group.IsStatic)
                {
                    invoker = resolveInvoker(methodId);
                    if (invoker == null)
                    {
                        return false;
                    }
                }

                entries.Add(new OverloadEntry(method, invoker));
            }

            try
            {
                dispatch = Cache.GetOrAdd(
                    new DispatchKey(dispatchId),
                    _ => Compile(group, entries));
                return dispatch != null;
            }
            catch
            {
                return false;
            }
        }

        private static bool EnsureHelperMethods(bool isStatic)
        {
            if (ValidateExactArgCountMethod == null
                || EnterLuaToCSharpMethod == null
                || LeaveLuaToCSharpMethod == null)
            {
                return false;
            }

            return isStatic
                ? TryInvokeFastMethod != null
                : PopTargetMethod != null;
        }

        private static LuaCSFunction Compile(TypeMethodRegistration.DispatchGroup group, List<OverloadEntry> entries)
        {
            return group.IsStatic
                ? CompileStaticDispatch(group, entries)
                : CompileInstanceDispatch(group, entries);
        }

        private static LuaCSFunction CompileStaticDispatch(
            TypeMethodRegistration.DispatchGroup group,
            List<OverloadEntry> entries)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");
            const int argStartIndex = 1;

            Expression chain = Expression.Call(
                ThrowNoOverloadMethod,
                luaStateParam,
                Expression.Constant(group),
                Expression.Constant(argStartIndex));

            for (int i = entries.Count - 1; i >= 0; i--)
            {
                OverloadEntry entry = entries[i];
                Expression canConvert = Expression.Call(
                    TryCanConvertArgumentsMethod,
                    luaStateParam,
                    Expression.Constant(entry.Method),
                    Expression.Constant(argStartIndex));
                Expression invoke = Expression.Call(
                    InvokeCompiledInvokerMethod,
                    Expression.Constant(entry.Invoker),
                    luaStateParam);
                chain = Expression.Condition(canConvert, invoke, chain);
            }

            if (TryBuildFastMap(entries, out Dictionary<ulong, OverloadTypeMask.FastDispatchEntry> fastMap))
            {
                ParameterExpression fastResultVar = Expression.Variable(typeof(int), "fastResult");
                chain = Expression.Block(
                    new[] { fastResultVar },
                    Expression.Assign(
                        fastResultVar,
                        Expression.Call(
                            TryInvokeFastMethod,
                            luaStateParam,
                            Expression.Constant(fastMap),
                            Expression.Constant(argStartIndex))),
                    Expression.Condition(
                        Expression.NotEqual(fastResultVar, Expression.Constant(-1)),
                        fastResultVar,
                        chain));
            }

            return WrapTryCatch(luaStateParam, exceptionParam, chain);
        }

        private static LuaCSFunction CompileInstanceDispatch(
            TypeMethodRegistration.DispatchGroup group,
            List<OverloadEntry> entries)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");
            ParameterExpression targetParam = Expression.Variable(group.OwnerType, "target");
            const int argStartIndex = 2;

            Expression assignTarget = Expression.Assign(
                targetParam,
                Expression.Convert(
                    Expression.Call(
                        PopTargetMethod,
                        luaStateParam,
                        Expression.Constant(1),
                        Expression.Constant(group.OwnerType, typeof(Type))),
                    group.OwnerType));

            Expression chain = Expression.Call(
                ThrowNoOverloadMethod,
                luaStateParam,
                Expression.Constant(group),
                Expression.Constant(argStartIndex));

            for (int i = entries.Count - 1; i >= 0; i--)
            {
                OverloadEntry entry = entries[i];
                Expression canConvert = Expression.Call(
                    TryCanConvertArgumentsMethod,
                    luaStateParam,
                    Expression.Constant(entry.Method),
                    Expression.Constant(argStartIndex));
                Expression invoke = BuildInstanceInvoke(luaStateParam, targetParam, entry.Method, argStartIndex);
                chain = Expression.Condition(canConvert, invoke, chain);
            }

            Expression body = Expression.Block(
                new[] { targetParam },
                assignTarget,
                chain);

            return WrapTryCatch(luaStateParam, exceptionParam, body);
        }

        private static Expression BuildInstanceInvoke(
            ParameterExpression luaStateParam,
            ParameterExpression targetParam,
            MethodInfo method,
            int argStartIndex)
        {
            ParameterInfo[] parameters = method.GetParameters();
            Expression validateArgs = Expression.Call(
                ValidateExactArgCountMethod,
                luaStateParam,
                Expression.Constant(argStartIndex),
                Expression.Constant(parameters.Length),
                Expression.Constant(method));

            Expression[] callArguments = new Expression[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                callArguments[i] = LuaToCSharpBridgeExpressionBuilder.BuildPopArgument(
                    luaStateParam,
                    argStartIndex + i,
                    parameters[i].ParameterType);
            }

            Expression callExpression = Expression.Call(targetParam, method, callArguments);
            Expression setReturnCount = method.ReturnType == typeof(void)
                ? Expression.Block(callExpression, Expression.Constant(0))
                : LuaToCSharpBridgeExpressionBuilder.BuildPushReturn(
                    luaStateParam,
                    method.ReturnType,
                    callExpression);

            Expression enterScope = Expression.Call(EnterLuaToCSharpMethod);
            Expression leaveScope = Expression.Call(LeaveLuaToCSharpMethod);

            return Expression.Block(
                validateArgs,
                Expression.TryFinally(
                    Expression.Block(enterScope, setReturnCount),
                    leaveScope));
        }

        private static LuaCSFunction WrapTryCatch(
            ParameterExpression luaStateParam,
            ParameterExpression exceptionParam,
            Expression body)
        {
            Expression tryCatch = Expression.TryCatch(
                body,
                Expression.Catch(exceptionParam, Expression.Call(ToLuaErrorMethod, luaStateParam, exceptionParam)));

            return Expression.Lambda<LuaCSFunction>(tryCatch, luaStateParam).Compile();
        }

        private static bool TryBuildFastMap(List<OverloadEntry> entries, out Dictionary<ulong, OverloadTypeMask.FastDispatchEntry> map)
        {
            var methods = new MethodInfo[entries.Count];
            var invokers = new LuaCSFunction[entries.Count];
            for (int i = 0; i < entries.Count; i++)
            {
                methods[i] = entries[i].Method;
                invokers[i] = entries[i].Invoker;
            }

            return OverloadTypeMask.TryBuildFastMap(methods, invokers, out map);
        }

        private static int InvokeCompiledInvoker(LuaCSFunction invoker, IntPtr luaState)
        {
            return invoker(luaState);
        }

        private static int ThrowNoOverload(IntPtr luaState, TypeMethodRegistration.DispatchGroup group, int argStartIndex)
        {
            return TypeMethodRegistration.FailNoOverloadForDispatch(luaState, group, argStartIndex);
        }

        private readonly struct OverloadEntry
        {
            internal readonly MethodInfo Method;
            internal readonly LuaCSFunction Invoker;

            internal OverloadEntry(MethodInfo method, LuaCSFunction invoker)
            {
                Method = method;
                Invoker = invoker;
            }
        }
    }
}
