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
        private static readonly Dictionary<MethodInfo, int> s_closedMethodClosureRefs =
            new Dictionary<MethodInfo, int>();

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
            bool hadOpenGeneric = false;
            string rejectMessage = null;
            for (int i = 0; i < methods.Count; i++)
            {
                if (IsOpenGeneric(methods[i]))
                {
                    hadOpenGeneric = true;
                    continue;
                }

                if (PointerMarshal.TryGetMethodRejectMessage(methods[i], out string methodReject))
                {
                    rejectMessage = methodReject;
                    continue;
                }

                // Do not throw EmitException under reverse P/Invoke (Tuanjie Mono SIGSEGV).
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
                else if (hadOpenGeneric)
                {
                    closure = CompileOpenGenericStub();
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
                closure = emitable.Count == 1
                    ? CompileDirect(emitable[0], isStatic, isByVal)
                    : CompileOverloadDispatch(emitable, isStatic, isByVal, info.Name);
            }

            ClosurePin.WriteToTableWithTag(L, methodTableRef, info.Name, closure, tag);
        }

        internal static int GetOrCreateClosedMethodClosureRef(
            IntPtr L,
            MethodInfo closedMethod,
            MethodClosureTag sourceTag)
        {
            if (closedMethod == null)
            {
                throw new ArgumentNullException(nameof(closedMethod));
            }

            if (s_closedMethodClosureRefs.TryGetValue(closedMethod, out int existingRef))
            {
                return existingRef;
            }

            LuaCSFunction closure = CompileDirect(closedMethod, sourceTag.IsStatic, sourceTag.IsByVal);
            MethodClosureTag tag = BuildTag(closedMethod, sourceTag.OwnerType, sourceTag.IsStatic, sourceTag.IsByVal);
            ClosurePin.PushWithTag(L, closure, tag);
            int closureRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            s_closedMethodClosureRefs[closedMethod] = closureRef;
            return closureRef;
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

        private static LuaCSFunction CompileOpenGenericStub()
        {
            return Wrap(_ =>
            {
                LuaCallbackBoundary.Throw("zlua: cannot invoke open generic method");
                return 0;
            });
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
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                int arity = InterpretedMethodInvoker.GetLuaArity(method);
                if (!byArity.TryGetValue(arity, out List<MethodInfo> list))
                {
                    list = new List<MethodInfo>();
                    byArity[arity] = list;
                }

                list.Add(method);
            }

            var cores = new Dictionary<int, Func<IntPtr, int>>();
            foreach (KeyValuePair<int, List<MethodInfo>> kv in byArity)
            {
                List<MethodInfo> group = kv.Value;
                MethodInfo first = group[0];
                if (group.Count == 1)
                {
                    cores[kv.Key] = InterpretedMethodInvoker.NeedsInterpreted(first)
                        ? InterpretedMethodInvoker.CompileMethod(first, isStatic, isByVal)
                        : BuildDirectCore(first, isStatic, isByVal);
                }
                else
                {
                    cores[kv.Key] = BuildScoredOverloadCore(group, isStatic, isByVal, name);
                }
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
                            LuaCallbackBoundary.Throw($"zlua: no overload of {name} with {actual} argument(s)");
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
            return L =>
            {
                if (!LuaArgMatcher.TrySelectMethod(L, argStart, methodArray, out int selected))
                {
                    LuaCallbackBoundary.Throw($"zlua: no matching overload of {name}");
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
