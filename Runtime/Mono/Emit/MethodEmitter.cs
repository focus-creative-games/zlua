using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Bridge;
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

                try
                {
                    EnsureCanEmitClosed(methods[i]);
                    emitable.Add(methods[i]);
                }
                catch (EmitException)
                {
                    // Soft-skip unemittable overload; keep other overloads if any.
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

        private static void EnsureCanEmitClosed(MethodInfo method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!BridgeMarshaling.IsSupportedParameter(parameters[i]))
                {
                    throw EmitException.ForMember(
                        method.DeclaringType,
                        method.Name,
                        $"unsupported parameter '{parameters[i].Name}' of type {parameters[i].ParameterType}");
                }
            }

            if (method.ReturnType != typeof(void) && !BridgeMarshaling.IsSupportedType(method.ReturnType))
            {
                throw EmitException.ForMember(method.DeclaringType, method.Name, "unsupported return type");
            }
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
            return Wrap(BuildDirectCore(method, isStatic, isByVal));
        }

        private static Func<IntPtr, int> BuildDirectCore(MethodInfo method, bool isStatic, bool isByVal)
        {
            if (PointerMarshal.MethodRequiresPointerInvoke(method))
            {
                return BuildPointerCore(method, isStatic, isByVal);
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
            ParameterExpression typedTargetLocal = null;
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
                        Expression.Constant(isByVal))));

                if (isByVal && declaringType.IsValueType)
                {
                    typedTargetLocal = Expression.Variable(declaringType, "typedTarget");
                    locals.Add(typedTargetLocal);
                    exprs.Add(Expression.Assign(typedTargetLocal, Expression.Convert(targetLocal, declaringType)));
                    targetExpr = typedTargetLocal;
                }
                else
                {
                    targetExpr = Expression.Convert(targetLocal, declaringType);
                }
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

            Expression writeBack = null;
            if (typedTargetLocal != null)
            {
                writeBack = Expression.Call(
                    EmitMethods.StructWriteBack,
                    L,
                    Expression.Constant(1),
                    Expression.Convert(typedTargetLocal, typeof(object)),
                    Expression.Constant(declaringType, typeof(Type)));
            }

            if (method.ReturnType == typeof(void))
            {
                exprs.Add(call);
                if (writeBack != null)
                {
                    exprs.Add(writeBack);
                }

                exprs.Add(Expression.Constant(0));
            }
            else
            {
                ParameterExpression resultLocal = Expression.Variable(method.ReturnType, "result");
                locals.Add(resultLocal);
                exprs.Add(Expression.Assign(resultLocal, call));
                if (writeBack != null)
                {
                    exprs.Add(writeBack);
                }

                exprs.Add(Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(method.ReturnType, typeof(Type)),
                    Expression.Convert(resultLocal, typeof(object))));
            }

            Expression body = Expression.Block(locals, exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        private static Func<IntPtr, int> BuildPointerCore(MethodInfo method, bool isStatic, bool isByVal)
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
                        Expression.Constant(isByVal))));
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

            if (isByVal && !isStatic)
            {
                exprs.Add(Expression.Call(
                    EmitMethods.StructWriteBack,
                    L,
                    Expression.Constant(1),
                    targetLocal,
                    Expression.Constant(declaringType, typeof(Type))));
            }

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
            var byArity = new Dictionary<int, MethodInfo>();
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                int arity = method.GetParameters().Length;
                if (byArity.ContainsKey(arity))
                {
                    continue;
                }

                byArity[arity] = method;
            }

            var cores = new Dictionary<int, Func<IntPtr, int>>();
            foreach (KeyValuePair<int, MethodInfo> kv in byArity)
            {
                cores[kv.Key] = BuildDirectCore(kv.Value, isStatic, isByVal);
            }

            int argStart = isStatic ? 1 : 2;
            return L =>
            {
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
                        if (LuaInvokePendingError.TryTake(out string pending))
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
                }
            };
        }

        private static LuaCSFunction Wrap(Func<IntPtr, int> core)
        {
            return L =>
            {
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        int nrets = core(L);
                        if (LuaInvokePendingError.TryTake(out string pending))
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
                }
            };
        }
    }
}
