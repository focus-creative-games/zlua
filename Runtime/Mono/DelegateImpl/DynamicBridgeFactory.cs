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
using System.Collections.Concurrent;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using ZLua;
using ZLua.Lvm;
using ZLua.Utils;

namespace ZLua.DelegateImpl
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

        private static readonly MethodInfo FormatErrorObjectMethod = typeof(LuaDllExtension).GetMethod(
            nameof(LuaDllExtension.FormatErrorObject),
            new[] { typeof(IntPtr), typeof(int) });

        private static readonly MethodInfo PendingErrorSet = typeof(NestedLuaCallPendingError).GetMethod(
            nameof(NestedLuaCallPendingError.Set),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly PropertyInfo IsNestedManagedPcall = typeof(LuaPrintBuffer).GetProperty(
            nameof(LuaPrintBuffer.IsNestedManagedPcall),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly ConstructorInfo LuaScriptExceptionCtor =
            typeof(LuaScriptException).GetConstructor(new[] { typeof(string) });

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

        private static readonly MethodInfo EnterStandaloneOpaque = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.EnterStandaloneCSharpToLua),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static Func<LuaMethod, Delegate> CompileFactory(MethodInfo invokeMethod, Type delegateType)
        {
            ParameterInfo[] parameters = invokeMethod.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i].IsDefined(typeof(ParamArrayAttribute), inherit: false))
                {
                    throw new NotSupportedException(
                        $"zlua: params is not supported on delegate bridge / GetFunction ({delegateType.FullName}).");
                }
            }

            bool needsOpaqueScope = false;
            for (int i = 0; i < parameters.Length; i++)
            {
                LuaMarshalType mt = CSharpToLuaBridgeExpressionBuilder.ResolveParameterMarshalType(parameters[i]);
                if (mt == LuaMarshalType.OpaqueValue || parameters[i].ParameterType.IsByRef)
                {
                    needsOpaqueScope = true;
                    break;
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

            var locals = new System.Collections.Generic.List<ParameterExpression>
            {
                luaStateVar, oldTopVar, pcallResultVar, functionTypeVar
            };

            var invokeStatements = new System.Collections.Generic.List<Expression>();
            if (needsOpaqueScope)
            {
                invokeStatements.Add(Expression.Call(EnterStandaloneOpaque));
            }

            invokeStatements.Add(Expression.Call(targetParam, PushErrorHandlerToStack));
            invokeStatements.Add(Expression.Assign(
                functionTypeVar,
                Expression.Call(
                    LuaRawGetI,
                    luaStateVar,
                    Expression.Constant(LuaConsts.LuaRegistryIndex),
                    Expression.Convert(
                        Expression.Property(targetParam, RefIndexProperty),
                        typeof(long)))));
            invokeStatements.Add(Expression.IfThen(
                Expression.NotEqual(
                    functionTypeVar,
                    Expression.Constant(LuaDataType.Function)),
                Expression.Throw(
                    Expression.New(
                        typeof(InvalidOperationException).GetConstructor(new[] { typeof(string) }),
                        Expression.Constant("Lua function reference is invalid.")))));

            var writeBacks = new System.Collections.Generic.List<Expression>();
            for (int i = 0; i < parameters.Length; i++)
            {
                Expression push = CSharpToLuaBridgeExpressionBuilder.BuildPushArgument(
                    luaStateVar,
                    argExprs[i],
                    parameters[i],
                    out ParameterExpression opaqueHandleVar,
                    out Expression writeBack);
                if (opaqueHandleVar != null)
                {
                    locals.Add(opaqueHandleVar);
                }

                invokeStatements.Add(push);
                if (writeBack != null)
                {
                    writeBacks.Add(writeBack);
                }
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

            // Nested pcall: stash pending error (Unity Mono SIGSEGV if throw during outer lua_pcall).
            Expression errorMessage = Expression.Call(FormatErrorObjectMethod, luaStateVar, Expression.Constant(-1));
            Expression onPcallFail = Expression.IfThenElse(
                Expression.Property(null, IsNestedManagedPcall),
                Expression.Call(PendingErrorSet, errorMessage),
                Expression.Throw(Expression.New(LuaScriptExceptionCtor, errorMessage)));

            invokeStatements.Add(Expression.IfThen(
                Expression.NotEqual(pcallResultVar, Expression.Constant(0)),
                onPcallFail));

            foreach (Expression writeBack in writeBacks)
            {
                invokeStatements.Add(Expression.IfThen(
                    Expression.Equal(pcallResultVar, Expression.Constant(0)),
                    writeBack));
            }

            Expression returnExpression;
            if (returnType == typeof(void))
            {
                returnExpression = Expression.Empty();
            }
            else
            {
                // On nested failure, return default without throwing.
                ParameterExpression retVar = Expression.Variable(returnType, "_ret");
                locals.Add(retVar);
                Expression pop = CSharpToLuaBridgeExpressionBuilder.BuildPopReturn(
                    luaStateVar,
                    invokeMethod,
                    returnType);
                returnExpression = Expression.Block(
                    Expression.IfThen(
                        Expression.Equal(pcallResultVar, Expression.Constant(0)),
                        Expression.Assign(retVar, pop)),
                    retVar);
            }

            Expression tryBody = Expression.Block(
                invokeStatements.Concat(new[] { returnExpression }));

            // If getLuaState throws (env Reset/disposed), L stays IntPtr.Zero — never lua_settop(0).
            Expression restoreStack = Expression.IfThen(
                Expression.NotEqual(luaStateVar, Expression.Constant(IntPtr.Zero)),
                Expression.Call(LuaSetTop, luaStateVar, oldTopVar));

            Expression body = Expression.Block(
                locals,
                Expression.TryFinally(
                    Expression.Block(
                        getLuaState,
                        saveTop,
                        Expression.Call(EnterManagedPcall),
                        tryBody),
                    Expression.Block(
                        restoreStack,
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
