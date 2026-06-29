using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Compiles constructor overload dispatch with (argc, typeMask) fast table (P1 · 4.6).
    /// </summary>
    internal static class LuaToCSharpConstructorOverloadDispatchFactory
    {
        private readonly struct DispatchKey : IEquatable<DispatchKey>
        {
            internal readonly int DispatchId;
            internal readonly int ArgStartIndex;

            internal DispatchKey(int dispatchId, int argStartIndex)
            {
                DispatchId = dispatchId;
                ArgStartIndex = argStartIndex;
            }

            public bool Equals(DispatchKey other)
            {
                return DispatchId == other.DispatchId && ArgStartIndex == other.ArgStartIndex;
            }

            public override bool Equals(object obj) => obj is DispatchKey other && Equals(other);

            public override int GetHashCode()
            {
                unchecked
                {
                    return (DispatchId * 397) ^ ArgStartIndex;
                }
            }
        }

        private static readonly System.Collections.Concurrent.ConcurrentDictionary<DispatchKey, LuaCSFunction> Cache =
            new System.Collections.Concurrent.ConcurrentDictionary<DispatchKey, LuaCSFunction>();

        private static readonly MethodInfo TryCanConvertArgumentsMethod = typeof(TypeMethodRegistration).GetMethod(
            nameof(TypeMethodRegistration.TryCanConvertArguments),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo TryInvokeFastMethod = typeof(OverloadTypeMask).GetMethod(
            nameof(OverloadTypeMask.TryInvokeFast),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ThrowNoOverloadMethod = typeof(LuaToCSharpConstructorOverloadDispatchFactory).GetMethod(
            nameof(ThrowNoOverload),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ToLuaErrorMethod = typeof(LuaCallbackBoundary).GetMethod(
            nameof(LuaCallbackBoundary.ToLuaError),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo InvokeCompiledInvokerMethod = typeof(LuaToCSharpConstructorOverloadDispatchFactory).GetMethod(
            nameof(InvokeCompiledInvoker),
            BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(LuaCSFunction), typeof(IntPtr) },
            modifiers: null);

        internal static bool TryCreate(
            TypeMethodRegistration.DispatchGroup group,
            int dispatchId,
            int argStartIndex,
            Func<int, ConstructorInfo> resolveConstructor,
            Func<int, LuaCSFunction> resolveInvoker,
            out LuaCSFunction dispatch)
        {
            dispatch = null;
            if (group == null
                || !group.IsConstructor
                || group.ConstructorIds == null
                || group.ConstructorIds.Length == 0
                || TryCanConvertArgumentsMethod == null
                || ThrowNoOverloadMethod == null
                || ToLuaErrorMethod == null
                || InvokeCompiledInvokerMethod == null)
            {
                return false;
            }

            var entries = new List<OverloadEntry>(group.ConstructorIds.Length);
            for (int i = 0; i < group.ConstructorIds.Length; i++)
            {
                int ctorId = group.ConstructorIds[i];
                ConstructorInfo ctor = resolveConstructor(ctorId);
                LuaCSFunction invoker = resolveInvoker(ctorId);
                if (ctor == null
                    || invoker == null
                    || !LuaToCSharpConstructorBridgeFactory.CanFastInvoke(ctor))
                {
                    return false;
                }

                entries.Add(new OverloadEntry(ctor, invoker));
            }

            try
            {
                dispatch = Cache.GetOrAdd(
                    new DispatchKey(dispatchId, argStartIndex),
                    _ => Compile(group, entries, argStartIndex));
                return dispatch != null;
            }
            catch
            {
                return false;
            }
        }

        private static LuaCSFunction Compile(
            TypeMethodRegistration.DispatchGroup group,
            List<OverloadEntry> entries,
            int argStartIndex)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

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
                    Expression.Constant(entry.Constructor),
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

            Expression tryCatch = Expression.TryCatch(
                chain,
                Expression.Catch(exceptionParam, Expression.Call(ToLuaErrorMethod, luaStateParam, exceptionParam)));

            return Expression.Lambda<LuaCSFunction>(tryCatch, luaStateParam).Compile();
        }

        private static bool TryBuildFastMap(List<OverloadEntry> entries, out Dictionary<ulong, OverloadTypeMask.FastDispatchEntry> map)
        {
            var constructors = new ConstructorInfo[entries.Count];
            var invokers = new LuaCSFunction[entries.Count];
            for (int i = 0; i < entries.Count; i++)
            {
                constructors[i] = entries[i].Constructor;
                invokers[i] = entries[i].Invoker;
            }

            return OverloadTypeMask.TryBuildConstructorFastMap(constructors, invokers, out map);
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
            internal readonly ConstructorInfo Constructor;
            internal readonly LuaCSFunction Invoker;

            internal OverloadEntry(ConstructorInfo constructor, LuaCSFunction invoker)
            {
                Constructor = constructor;
                Invoker = invoker;
            }
        }
    }
}
