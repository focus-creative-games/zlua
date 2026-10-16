// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal static class MethodEmitter
    {
        private sealed class ClosedMethodEntry
        {
            public int RegistryRef;
            public LuaCSFunction Invoker;
        }

        private readonly struct ClosedMethodKey : IEquatable<ClosedMethodKey>
        {
            public readonly MethodInfo Method;
            public readonly bool IsStatic;
            public readonly bool IsByVal;

            public ClosedMethodKey(MethodInfo method, bool isStatic, bool isByVal)
            {
                Method = method;
                IsStatic = isStatic;
                IsByVal = isByVal;
            }

            public bool Equals(ClosedMethodKey other)
            {
                return Method == other.Method
                    && IsStatic == other.IsStatic
                    && IsByVal == other.IsByVal;
            }

            public override bool Equals(object obj)
            {
                return obj is ClosedMethodKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    int hash = Method != null ? Method.GetHashCode() : 0;
                    hash = (hash * 397) ^ (IsStatic ? 1 : 0);
                    hash = (hash * 397) ^ (IsByVal ? 1 : 0);
                    return hash;
                }
            }
        }

        private static readonly Dictionary<ClosedMethodKey, ClosedMethodEntry> s_closedMethods =
            new Dictionary<ClosedMethodKey, ClosedMethodEntry>();

        /// <summary>
        /// Drop closed-method closure registry refs (state-local). Call on Reset / <c>lua_close</c>.
        /// </summary>
        internal static void Shutdown()
        {
            s_closedMethods.Clear();
        }

        internal static void Write(IntPtr L, int methodTableRef, MetaInfo info, Type ownerType, bool isStatic, bool isByVal)
        {
            if (info == null || info.Kind != MetaKind.Method)
            {
                return;
            }

            List<MethodInfo> methods = info.MethodOverloads;
            if (methods == null || methods.Count == 0)
            {
                if (info.Method == null)
                {
                    return;
                }

                methods = new List<MethodInfo> { info.Method };
            }

            var emitable = new List<MethodInfo>(methods.Count);
            var openGenerics = new List<MethodInfo>();
            string rejectMessage = null;
            for (int i = 0; i < methods.Count; i++)
            {
                if (IsOpenGeneric(methods[i]))
                {
                    openGenerics.Add(methods[i]);
                    continue;
                }

                if (PointerMarshal.TryGetMethodRejectMessage(methods[i], out string methodReject))
                {
                    rejectMessage = methodReject;
                    continue;
                }

                // Do not throw EmitException under reverse P/Invoke (Tuanjie Mono SIGSEGV).
                // CanEmitClosed is cheap (no Expression.Compile).
                if (CanEmitClosed(methods[i]))
                {
                    emitable.Add(methods[i]);
                }
            }

            MethodInfo tagMethod = ResolveTagMethod(methods, emitable, info.Method);
            MethodClosureTag tag = BuildTag(tagMethod, ownerType, isStatic, isByVal);

            LuaCSFunction closure;
            if (emitable.Count == 0)
            {
                if (rejectMessage != null)
                {
                    closure = CompileRejectStub(rejectMessage);
                }
                else if (openGenerics.Count > 0)
                {
                    closure = CompileGtapClosure(openGenerics, tag, isStatic, isByVal, info.Name);
                }
                else
                {
                    // Soft-skip: e.g. Vector2.SmoothDamp(ref …) / array Address — cannot Emit byref.
                    // Aborting the whole type bind blocks unrelated members (Il2Cpp simply omits them).
                    return;
                }
            }
            else
            {
                // Defer Expression.Compile / overload dispatch build until first Lua call.
                closure = CreateLazyMethodClosure(new LazyMethodBind
                {
                    MethodTableRef = methodTableRef,
                    Name = info.Name,
                    Tag = tag,
                    Emitable = emitable,
                    OpenGenerics = openGenerics,
                    IsStatic = isStatic,
                    IsByVal = isByVal,
                });
            }

            ClosurePin.WriteToTableWithTag(L, methodTableRef, info.Name, closure, tag);
        }

        private sealed class LazyMethodBind
        {
            public int MethodTableRef;
            public string Name;
            public MethodClosureTag Tag;
            public List<MethodInfo> Emitable;
            public List<MethodInfo> OpenGenerics;
            public bool IsStatic;
            public bool IsByVal;
            public readonly object Gate = new object();
            public volatile LuaCSFunction Compiled;
        }

        private static LuaCSFunction CreateLazyMethodClosure(LazyMethodBind bind)
        {
            return L =>
            {
                LuaCSFunction compiled = bind.Compiled;
                if (compiled == null)
                {
                    lock (bind.Gate)
                    {
                        compiled = bind.Compiled;
                        if (compiled == null)
                        {
                            try
                            {
                                LuaCSFunction closedPath = bind.Emitable.Count == 1
                                    ? CompileDirect(bind.Emitable[0], bind.IsStatic, bind.IsByVal)
                                    : CompileOverloadDispatch(
                                        bind.Emitable, bind.IsStatic, bind.IsByVal, bind.Name);

                                compiled = bind.OpenGenerics != null && bind.OpenGenerics.Count > 0
                                    ? WrapGtapThenFallback(
                                        bind.OpenGenerics,
                                        bind.Tag,
                                        bind.IsStatic,
                                        bind.IsByVal,
                                        bind.Name,
                                        closedPath)
                                    : closedPath;

                                ClosurePin.WriteToTableWithTag(L, bind.MethodTableRef, bind.Name, compiled, bind.Tag);
                                bind.Compiled = compiled;
                            }
                            catch (Exception ex)
                            {
                                LuaCallbackBoundary.Enter();
                                try
                                {
                                    return LuaCallbackBoundary.ToLuaError(L, ex);
                                }
                                finally
                                {
                                    LuaCallbackBoundary.Leave();
                                }
                            }
                        }
                    }
                }

                return compiled(L);
            };
        }

        internal static int GetOrCreateClosedMethodClosureRef(
            IntPtr L,
            MethodInfo closedMethod,
            MethodClosureTag sourceTag)
        {
            return GetOrCreateClosedMethodEntry(L, closedMethod, sourceTag).RegistryRef;
        }

        private static ClosedMethodEntry GetOrCreateClosedMethodEntry(
            IntPtr L,
            MethodInfo closedMethod,
            MethodClosureTag sourceTag)
        {
            if (closedMethod == null)
            {
                throw new ArgumentNullException(nameof(closedMethod));
            }

            var key = new ClosedMethodKey(closedMethod, sourceTag.IsStatic, sourceTag.IsByVal);
            if (s_closedMethods.TryGetValue(key, out ClosedMethodEntry existing))
            {
                return existing;
            }

            LuaCSFunction invoker = CompileDirect(closedMethod, sourceTag.IsStatic, sourceTag.IsByVal);
            MethodClosureTag tag = BuildTag(closedMethod, sourceTag.OwnerType, sourceTag.IsStatic, sourceTag.IsByVal);
            ClosurePin.PushWithTag(L, invoker, tag);
            int closureRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            var entry = new ClosedMethodEntry
            {
                RegistryRef = closureRef,
                Invoker = invoker,
            };
            s_closedMethods[key] = entry;
            return entry;
        }

        private static MethodClosureTag BuildTag(MethodInfo method, Type ownerType, bool isStatic, bool isByVal)
        {
            return new MethodClosureTag
            {
                Method = method,
                OwnerType = ownerType,
                IsStatic = isStatic,
                IsByVal = isByVal,
            };
        }

        private static MethodInfo ResolveTagMethod(List<MethodInfo> methods, List<MethodInfo> emitable, MethodInfo primary)
        {
            for (int i = 0; i < methods.Count; i++)
            {
                if (IsOpenGeneric(methods[i]))
                {
                    return methods[i];
                }
            }

            if (emitable.Count > 0)
            {
                return emitable[0];
            }

            return primary ?? methods[0];
        }

        private static bool IsOpenGeneric(MethodInfo method)
        {
            return method.ContainsGenericParameters || method.IsGenericMethodDefinition;
        }

        private static bool CanEmitClosed(MethodInfo method)
        {
            return InterpretedMethodInvoker.CanBind(method);
        }

        private static LuaCSFunction CompileGtapClosure(
            List<MethodInfo> openGenerics,
            MethodClosureTag tag,
            bool isStatic,
            bool isByVal,
            string name)
        {
            return WrapGtapThenFallback(
                openGenerics,
                tag,
                isStatic,
                isByVal,
                name,
                fallback: L =>
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua: cannot invoke open generic method {name} without System.Type GTAP prefix "
                        + "(use zlua.inflate, or pass zlua.typeof(...) prefixes)");
                    return 0;
                });
        }

        private static LuaCSFunction WrapGtapThenFallback(
            List<MethodInfo> openGenerics,
            MethodClosureTag tag,
            bool isStatic,
            bool isByVal,
            string name,
            LuaCSFunction fallback)
        {
            MethodInfo[] openArray = openGenerics.ToArray();
            return L =>
            {
                LuaCallbackBoundary.Enter();
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        if (TryInvokeGtap(L, openArray, tag, isStatic, isByVal, out int nrets))
                        {
                            if (NestedLuaCallPendingError.TryTake(out string pendingGtap))
                            {
                                return LuaDllExtension.error(L, pendingGtap);
                            }

                            return nrets;
                        }

                        return fallback(L);
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    StructOpaqueScope.LeaveLuaToCSharp();
                    LuaCallbackBoundary.Leave();
                }
            };
        }

        /// <summary>
        /// GTAP: peel N consecutive <see cref="Type"/> ByObj userdata, inflate, invoke closed method.
        /// Prefers open-generic path over ordinary overloads when applicable (spec §3.8).
        /// </summary>
        private static bool TryInvokeGtap(
            IntPtr L,
            MethodInfo[] openGenerics,
            MethodClosureTag tag,
            bool isStatic,
            bool isByVal,
            out int nrets)
        {
            nrets = 0;
            if (openGenerics == null || openGenerics.Length == 0)
            {
                return false;
            }

            int argStart = isStatic ? 1 : 2;
            int top = LuaDll.lua_gettop(L);
            MethodInfo matchedClosed = null;
            int matchedPrefixCount = 0;
            int matchCount = 0;

            for (int i = 0; i < openGenerics.Length; i++)
            {
                MethodInfo open = openGenerics[i];
                int n = open.GetGenericArguments().Length;
                if (n < 1)
                {
                    continue;
                }

                if (!TryReadSystemTypePrefix(L, argStart, n, out Type[] typeArgs))
                {
                    continue;
                }

                int valueArgStart = argStart + n;
                int valueArgCount = top - valueArgStart + 1;
                if (valueArgCount < 0)
                {
                    valueArgCount = 0;
                }

                MethodInfo closed;
                try
                {
                    closed = open.MakeGenericMethod(typeArgs);
                }
                catch
                {
                    continue;
                }

                int minArity = DefaultParameterUtil.GetMinLuaArity(closed);
                int maxArity = InterpretedMethodInvoker.GetMaxLuaArity(closed);
                if (valueArgCount < minArity || valueArgCount > maxArity)
                {
                    continue;
                }

                // Remaining args must be applicable to the closed signature.
                // Zero-arg closed methods (e.g. GetComponent<T>()) skip SelectMethod.
                if (maxArity > 0 || valueArgCount > 0)
                {
                    MethodInfo[] one = { closed };
                    if (LuaArgMatcher.SelectMethod(L, valueArgStart, one, out _)
                        != LuaOverloadMatch.BestMatch)
                    {
                        continue;
                    }
                }

                matchCount++;
                if (matchCount == 1)
                {
                    matchedClosed = closed;
                    matchedPrefixCount = n;
                }
            }

            if (matchCount != 1 || matchedClosed == null)
            {
                return false;
            }

            for (int i = 0; i < matchedPrefixCount; i++)
            {
                LuaDll.lua_remove(L, argStart);
            }

            MethodClosureTag invokeTag = BuildTag(matchedClosed, tag.OwnerType, isStatic, isByVal);
            ClosedMethodEntry entry = GetOrCreateClosedMethodEntry(L, matchedClosed, invokeTag);
            nrets = entry.Invoker(L);
            return true;
        }

        private static bool TryReadSystemTypePrefix(IntPtr L, int argStart, int count, out Type[] typeArgs)
        {
            typeArgs = null;
            int top = LuaDll.lua_gettop(L);
            if (argStart + count - 1 > top)
            {
                return false;
            }

            typeArgs = new Type[count];
            for (int i = 0; i < count; i++)
            {
                int idx = argStart + i;
                // GTAP prefix: System.Type ByObj userdata only (not type table / string).
                if (LuaDll.lua_type(L, idx) != LuaDataType.UserData
                    || !ObjectRegistry.TryGetObject(L, idx, out object obj)
                    || !(obj is Type clrType))
                {
                    typeArgs = null;
                    return false;
                }

                typeArgs[i] = clrType;
            }

            return true;
        }

        private static LuaCSFunction CompileRejectStub(string message)
        {
            return Wrap(_ =>
            {
                LuaCallbackBoundary.Throw(message);
                return 0;
            });
        }

        private static LuaCSFunction CompileDirect(MethodInfo method, bool isStatic, bool isByVal)
        {
            Func<IntPtr, int> core = InterpretedMethodInvoker.NeedsInterpreted(method)
                || DefaultParameterUtil.HasOptionalDefaults(method)
                || ParamsArrayUtil.HasParamsArray(method)
                ? InterpretedMethodInvoker.CompileMethod(method, isStatic, isByVal)
                : BuildDirectCore(method, isStatic, isByVal);
            return Wrap(core);
        }

        private static Func<IntPtr, int> BuildDirectCore(MethodInfo method, bool isStatic, bool isByVal)
        {
            if (ExtensionMethodUtil.IsExtensionMethod(method))
            {
                return BuildExtensionCore(method);
            }

            if (!isStatic && isByVal && method.DeclaringType != null && method.DeclaringType.IsValueType)
            {
                return BuildByValCore(method);
            }

            if (PointerMarshal.MethodRequiresPointerInvoke(method))
            {
                return BuildPointerCore(method, isStatic);
            }

            Type declaringType = method.DeclaringType;
            ParameterInfo[] parameters = method.GetParameters();
            int argStart = isStatic ? 1 : 2;
            int expectedArgs = parameters.Length;

            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");
            var locals = new List<ParameterExpression>();
            var exprs = new List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(expectedArgs),
                    Expression.Constant(argStart)),
            };

            Expression targetExpr = null;
            if (!isStatic)
            {
                ParameterExpression targetLocal = Expression.Variable(typeof(object), "target");
                locals.Add(targetLocal);
                exprs.Add(Expression.Assign(
                    targetLocal,
                    Expression.Call(
                        EmitMethods.PopTarget,
                        L,
                        Expression.Constant(1),
                        Expression.Constant(declaringType, typeof(Type)),
                        Expression.Constant(false))));
                targetExpr = Expression.Convert(targetLocal, declaringType);
            }

            var argExprs = new Expression[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                Expression popped = Expression.Call(
                    EmitMethods.PopArg,
                    L,
                    Expression.Constant(argStart + i),
                    Expression.Constant(paramType, typeof(Type)));
                argExprs[i] = Expression.Convert(popped, paramType);
            }

            Expression call = isStatic
                ? Expression.Call(method, argExprs)
                : Expression.Call(targetExpr, method, argExprs);

            if (method.ReturnType == typeof(void))
            {
                exprs.Add(call);
                exprs.Add(Expression.Constant(0));
            }
            else
            {
                ParameterExpression resultLocal = Expression.Variable(method.ReturnType, "result");
                locals.Add(resultLocal);
                exprs.Add(Expression.Assign(resultLocal, call));
                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(method.ReturnType, typeof(Type)),
                    Expression.Convert(resultLocal, typeof(object))));
            }

            Expression body = Expression.Block(locals, exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        /// <summary>
        /// Extension static-as-instance: slot 1 → CLR param 0; remaining from slot 2 (spec 13 §4).
        /// </summary>
        private static Func<IntPtr, int> BuildExtensionCore(MethodInfo method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            const int argStart = 2;
            int expectedArgs = parameters.Length - 1;

            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");
            var locals = new List<ParameterExpression>();
            var exprs = new List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(expectedArgs),
                    Expression.Constant(argStart)),
            };

            var argExprs = new Expression[parameters.Length];
            Type p0Type = parameters[0].ParameterType;
            Expression receiverPopped = Expression.Call(
                EmitMethods.PopArg,
                L,
                Expression.Constant(1),
                Expression.Constant(p0Type, typeof(Type)));
            argExprs[0] = Expression.Convert(receiverPopped, p0Type);

            for (int i = 1; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                Expression popped = Expression.Call(
                    EmitMethods.PopArg,
                    L,
                    Expression.Constant(argStart + i - 1),
                    Expression.Constant(paramType, typeof(Type)));
                argExprs[i] = Expression.Convert(popped, paramType);
            }

            Expression call = Expression.Call(method, argExprs);

            if (method.ReturnType == typeof(void))
            {
                exprs.Add(call);
                exprs.Add(Expression.Constant(0));
            }
            else
            {
                ParameterExpression resultLocal = Expression.Variable(method.ReturnType, "result");
                locals.Add(resultLocal);
                exprs.Add(Expression.Assign(resultLocal, call));
                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(method.ReturnType, typeof(Type)),
                    Expression.Convert(resultLocal, typeof(object))));
            }

            Expression body = Expression.Block(locals, exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        /// <summary>
        /// ByVal instance: in-place via payload / companion (no copy + WriteBack).
        /// </summary>
        private static Func<IntPtr, int> BuildByValCore(MethodInfo method)
        {
            Type declaringType = method.DeclaringType;
            ParameterInfo[] parameters = method.GetParameters();
            const int argStart = 2;
            int expectedArgs = parameters.Length;

            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");
            var locals = new List<ParameterExpression>();
            var exprs = new List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(expectedArgs),
                    Expression.Constant(argStart)),
            };

            ParameterExpression argsLocal = Expression.Variable(typeof(object[]), "args");
            locals.Add(argsLocal);
            exprs.Add(Expression.Assign(
                argsLocal,
                Expression.NewArrayBounds(typeof(object), Expression.Constant(parameters.Length))));

            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                Expression popped = Expression.Call(
                    EmitMethods.PopArg,
                    L,
                    Expression.Constant(argStart + i),
                    Expression.Constant(paramType, typeof(Type)));
                exprs.Add(Expression.Assign(
                    Expression.ArrayAccess(argsLocal, Expression.Constant(i)),
                    popped));
            }

            ParameterExpression resultLocal = Expression.Variable(typeof(object), "result");
            locals.Add(resultLocal);
            exprs.Add(Expression.Assign(
                resultLocal,
                Expression.Call(
                    EmitMethods.ByValInvokeInstance,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(method, typeof(MethodInfo)),
                    argsLocal)));

            if (method.ReturnType == typeof(void))
            {
                exprs.Add(Expression.Constant(0));
            }
            else
            {
                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(method.ReturnType, typeof(Type)),
                    resultLocal));
            }

            Expression body = Expression.Block(locals, exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        private static Func<IntPtr, int> BuildPointerCore(MethodInfo method, bool isStatic)
        {
            Type declaringType = method.DeclaringType;
            ParameterInfo[] parameters = method.GetParameters();
            int argStart = isStatic ? 1 : 2;
            int expectedArgs = parameters.Length;

            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");
            var locals = new List<ParameterExpression>();
            var exprs = new List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(expectedArgs),
                    Expression.Constant(argStart)),
            };

            ParameterExpression targetLocal = Expression.Variable(typeof(object), "target");
            locals.Add(targetLocal);
            if (isStatic)
            {
                exprs.Add(Expression.Assign(targetLocal, Expression.Constant(null, typeof(object))));
            }
            else
            {
                exprs.Add(Expression.Assign(
                    targetLocal,
                    Expression.Call(
                        EmitMethods.PopTarget,
                        L,
                        Expression.Constant(1),
                        Expression.Constant(declaringType, typeof(Type)),
                        Expression.Constant(false))));
            }

            ParameterExpression argsLocal = Expression.Variable(typeof(object[]), "args");
            locals.Add(argsLocal);
            exprs.Add(Expression.Assign(
                argsLocal,
                Expression.NewArrayBounds(typeof(object), Expression.Constant(parameters.Length))));

            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                Expression popped = Expression.Call(
                    EmitMethods.PopArg,
                    L,
                    Expression.Constant(argStart + i),
                    Expression.Constant(paramType, typeof(Type)));
                exprs.Add(Expression.Assign(
                    Expression.ArrayAccess(argsLocal, Expression.Constant(i)),
                    popped));
            }

            ParameterExpression resultLocal = Expression.Variable(typeof(object), "result");
            locals.Add(resultLocal);
            exprs.Add(Expression.Assign(
                resultLocal,
                Expression.Call(
                    EmitMethods.PointerInvoke,
                    Expression.Constant(method, typeof(MethodInfo)),
                    targetLocal,
                    argsLocal)));

            if (method.ReturnType == typeof(void))
            {
                exprs.Add(Expression.Constant(0));
            }
            else
            {
                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(method.ReturnType, typeof(Type)),
                    resultLocal));
            }

            Expression body = Expression.Block(locals, exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        private static LuaCSFunction CompileOverloadDispatch(List<MethodInfo> methods, bool isStatic, bool isByVal, string name)
        {
            var byArity = new Dictionary<int, List<MethodInfo>>();
            var paramsMethods = new List<MethodInfo>();
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                bool hasParams = ParamsArrayUtil.HasParamsArray(method);
                if (hasParams)
                {
                    paramsMethods.Add(method);
                }

                int minArity = DefaultParameterUtil.GetMinLuaArity(method);
                int maxArity = hasParams
                    ? ParamsArrayUtil.MaxExpandArityBucket
                    : InterpretedMethodInvoker.GetLuaArity(method);
                for (int arity = minArity; arity <= maxArity; arity++)
                {
                    if (!byArity.TryGetValue(arity, out List<MethodInfo> list))
                    {
                        list = new List<MethodInfo>();
                        byArity[arity] = list;
                    }

                    list.Add(method);
                }
            }

            var cores = new Dictionary<int, Func<IntPtr, int>>();
            foreach (KeyValuePair<int, List<MethodInfo>> kv in byArity)
            {
                List<MethodInfo> group = kv.Value;
                MethodInfo first = group[0];
                if (group.Count == 1)
                {
                    cores[kv.Key] = InterpretedMethodInvoker.NeedsInterpreted(first)
                        || DefaultParameterUtil.HasOptionalDefaults(first)
                        || ParamsArrayUtil.HasParamsArray(first)
                        ? InterpretedMethodInvoker.CompileMethod(first, isStatic, isByVal)
                        : BuildDirectCore(first, isStatic, isByVal);
                }
                else
                {
                    cores[kv.Key] = BuildScoredOverloadCore(group, isStatic, isByVal, name);
                }
            }

            Func<IntPtr, int> paramsOverflowCore = null;
            if (paramsMethods.Count == 1)
            {
                paramsOverflowCore = InterpretedMethodInvoker.CompileMethod(
                    paramsMethods[0], isStatic, isByVal);
            }
            else if (paramsMethods.Count > 1)
            {
                paramsOverflowCore = BuildScoredOverloadCore(paramsMethods, isStatic, isByVal, name);
            }

            int argStart = isStatic ? 1 : 2;
            return L =>
            {
                LuaCallbackBoundary.Enter();
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        int top = LuaDll.lua_gettop(L);
                        int actual = top - argStart + 1;
                        if (actual < 0)
                        {
                            actual = 0;
                        }

                        if (!cores.TryGetValue(actual, out Func<IntPtr, int> core))
                        {
                            if (paramsOverflowCore != null && actual > ParamsArrayUtil.MaxExpandArityBucket)
                            {
                                core = paramsOverflowCore;
                            }
                            else
                            {
                                LuaCallbackBoundary.Throw($"zlua: no overload of {name} with {actual} argument(s)");
                            }
                        }

                        int nrets = core(L);
                        if (NestedLuaCallPendingError.TryTake(out string pending))
                        {
                            return LuaDllExtension.error(L, pending);
                        }

                        return nrets;
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    StructOpaqueScope.LeaveLuaToCSharp();
                    LuaCallbackBoundary.Leave();
                }
            };
        }

        private static Func<IntPtr, int> BuildScoredOverloadCore(
            List<MethodInfo> methods,
            bool isStatic,
            bool isByVal,
            string name)
        {
            // Always interpret multi-overload bodies. BuildDirectCore uses Expression.Compile →
            // DynamicMethod; Unity Mono throws if that runs while another DynamicMethod is active
            // (e.g. C#→Lua UserData string push → bind System.String while bridge lambda is live).
            var compiled = new Func<IntPtr, int>[methods.Count];
            for (int i = 0; i < methods.Count; i++)
            {
                compiled[i] = InterpretedMethodInvoker.CompileMethod(methods[i], isStatic, isByVal);
            }

            MethodInfo[] methodArray = methods.ToArray();
            int argStart = isStatic ? 1 : 2;
            string typeName = TypeRegistry.GetLuaFullName(methods[0].DeclaringType);
            return L =>
            {
                LuaOverloadMatch match = LuaArgMatcher.SelectMethod(L, argStart, methodArray, out int selected);
                if (match == LuaOverloadMatch.Ambiguous)
                {
                    LuaCallbackBoundary.Throw($"zlua: ambiguous method found: {typeName}.{name}");
                }

                if (match != LuaOverloadMatch.BestMatch)
                {
                    LuaCallbackBoundary.Throw($"zlua: no matching overload of {typeName}.{name}");
                }

                return compiled[selected](L);
            };
        }

        private static LuaCSFunction Wrap(Func<IntPtr, int> core)
        {
            return L =>
            {
                LuaCallbackBoundary.Enter();
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        int nrets = core(L);
                        if (NestedLuaCallPendingError.TryTake(out string pending))
                        {
                            return LuaDllExtension.error(L, pending);
                        }

                        return nrets;
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    StructOpaqueScope.LeaveLuaToCSharp();
                    LuaCallbackBoundary.Leave();
                }
            };
        }
    }
}
