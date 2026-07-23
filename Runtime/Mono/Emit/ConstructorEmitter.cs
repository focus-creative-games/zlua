using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal static class ConstructorEmitter
    {
        internal static void BindCall(IntPtr L, int typeTableIndex, TypeBinding binding)
        {
            Type type = binding.Type;
            if (type.IsAbstract || type.IsInterface)
            {
                ClearCall(L, typeTableIndex);
                return;
            }

            // Arrays are constructed via zlua.new_szarray_*/new_mdarray_*. CLR array ctors
            // (especially jagged T[][](int,int)) assert inside Mono JIT when Expression.Compile'd.
            if (type.IsArray)
            {
                ClearCall(L, typeTableIndex);
                return;
            }

            List<ConstructorInfo> ctors = binding.Constructors;
            if (ctors == null || ctors.Count == 0)
            {
                // Value types always have an implicit default ctor.
                if (type.IsValueType && !type.IsEnum)
                {
                    ClosurePin.Push(L, CompileDefaultStructCtor(type));
                    SetCall(L, typeTableIndex);
                    return;
                }

                ClearCall(L, typeTableIndex);
                return;
            }

            // Soft-skip ctors we cannot Emit (e.g. String(ReadOnlySpan<char>)).
            // Throwing here aborts the whole type bind and, under nested lua_pcall, SIGSEGVs.
            var emitable = new List<ConstructorInfo>(ctors.Count);
            for (int i = 0; i < ctors.Count; i++)
            {
                try
                {
                    EnsureCanEmit(ctors[i]);
                    emitable.Add(ctors[i]);
                }
                catch (EmitException)
                {
                    // Soft-skip: e.g. String(char*) / ReadOnlySpan — keep other overloads.
                }
            }

            if (emitable.Count == 0)
            {
                if (type.IsValueType && !type.IsEnum)
                {
                    ClosurePin.Push(L, CompileDefaultStructCtor(type));
                    SetCall(L, typeTableIndex);
                    return;
                }

                ClearCall(L, typeTableIndex);
                return;
            }

            // Value types always expose arity-0 default construction even when
            // reflection only lists parameterized public constructors.
            try
            {
                LuaCSFunction closure;
                if (type.IsValueType && !type.IsEnum && !HasArity(emitable, 0))
                {
                    closure = CompileOverloadDispatchWithDefault(emitable, type);
                }
                else if (emitable.Count == 1)
                {
                    closure = CompileDirect(emitable[0], type);
                }
                else
                {
                    closure = CompileOverloadDispatch(emitable, type);
                }

                ClosurePin.Push(L, closure);
                SetCall(L, typeTableIndex);
            }
            catch (Exception)
            {
                if (type.IsValueType && !type.IsEnum)
                {
                    ClosurePin.Push(L, CompileDefaultStructCtor(type));
                    SetCall(L, typeTableIndex);
                    return;
                }

                ClearCall(L, typeTableIndex);
            }
        }

        private static bool HasArity(List<ConstructorInfo> ctors, int arity)
        {
            for (int i = 0; i < ctors.Count; i++)
            {
                if (ctors[i].GetParameters().Length == arity)
                {
                    return true;
                }
            }

            return false;
        }

        private static void SetCall(IntPtr L, int typeTableIndex)
        {
            // stack: ..., fn
            if (LuaDll.lua_getmetatable(L, typeTableIndex) == 0)
            {
                // getmetatable pushes nothing on failure
                LuaDll.lua_pop(L, 1); // drop fn
                return;
            }

            // stack: ..., fn, smt  →  ..., smt, fn
            LuaDll.lua_insert(L, -2);
            LuaDll.lua_setfield(L, -2, LuaConsts.MetaCall);
            LuaDll.lua_pop(L, 1);
        }

        private static void ClearCall(IntPtr L, int typeTableIndex)
        {
            if (LuaDll.lua_getmetatable(L, typeTableIndex) == 0)
            {
                return;
            }

            LuaDll.lua_pushnil(L);
            LuaDll.lua_setfield(L, -2, LuaConsts.MetaCall);
            LuaDll.lua_pop(L, 1);
        }

        private static void EnsureCanEmit(ConstructorInfo ctor)
        {
            ParameterInfo[] parameters = ctor.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                // Expression trees cannot Convert to pointer types (methods use DynamicMethod).
                if (PointerMarshal.IsPointerLikeType(paramType))
                {
                    throw EmitException.ForMember(
                        ctor.DeclaringType,
                        ".ctor",
                        $"unsupported parameter '{parameters[i].Name}' of type {paramType}");
                }

                if (!BridgeMarshaling.IsSupportedParameter(parameters[i]))
                {
                    throw EmitException.ForMember(
                        ctor.DeclaringType,
                        ".ctor",
                        $"unsupported parameter '{parameters[i].Name}' of type {paramType}");
                }
            }
        }

        private static LuaCSFunction CompileDefaultStructCtor(Type type)
        {
            return L =>
            {
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        BridgeMarshaling.ValidateExactArgCount(L, 0, 2);
                        object value = Activator.CreateInstance(type);
                        TypeRegistry.PushConstructorInstance(L, value, type);
                        return 1;
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

        private static LuaCSFunction CompileDirect(ConstructorInfo ctor, Type type)
        {
            return Wrap(BuildDirectCore(ctor, type));
        }

        private static Func<IntPtr, int> BuildDirectCore(ConstructorInfo ctor, Type type)
        {
            ParameterInfo[] parameters = ctor.GetParameters();
            const int argStart = 2; // 1 = type table
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            var exprs = new List<Expression>
            {
                Expression.Call(
                    EmitMethods.ValidateExactArgCount,
                    L,
                    Expression.Constant(parameters.Length),
                    Expression.Constant(argStart)),
            };

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

            Expression constructed = Expression.New(ctor, argExprs);
            Expression boxed = Expression.Convert(constructed, typeof(object));
            exprs.Add(Expression.Call(
                EmitMethods.PushConstructorInstance,
                L,
                boxed,
                Expression.Constant(type, typeof(Type))));
            exprs.Add(Expression.Constant(1));

            Expression body = Expression.Block(exprs);
            return Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
        }

        private static LuaCSFunction CompileOverloadDispatch(List<ConstructorInfo> ctors, Type type)
        {
            return CompileOverloadDispatchCore(ctors, type, includeDefaultStruct: false);
        }

        private static LuaCSFunction CompileOverloadDispatchWithDefault(List<ConstructorInfo> ctors, Type type)
        {
            return CompileOverloadDispatchCore(ctors, type, includeDefaultStruct: true);
        }

        private static LuaCSFunction CompileOverloadDispatchCore(
            List<ConstructorInfo> ctors,
            Type type,
            bool includeDefaultStruct)
        {
            var byArity = new Dictionary<int, List<ConstructorInfo>>();
            for (int i = 0; i < ctors.Count; i++)
            {
                ConstructorInfo ctor = ctors[i];
                int arity = ctor.GetParameters().Length;
                if (!byArity.TryGetValue(arity, out List<ConstructorInfo> list))
                {
                    list = new List<ConstructorInfo>();
                    byArity[arity] = list;
                }

                list.Add(ctor);
            }

            var cores = new Dictionary<int, Func<IntPtr, int>>();
            foreach (KeyValuePair<int, List<ConstructorInfo>> kv in byArity)
            {
                cores[kv.Key] = kv.Value.Count == 1
                    ? BuildDirectCore(kv.Value[0], type)
                    : BuildSameArityDispatchCore(kv.Value, type);
            }

            if (includeDefaultStruct && !cores.ContainsKey(0))
            {
                cores[0] = L =>
                {
                    BridgeMarshaling.ValidateExactArgCount(L, 0, 2);
                    object value = Activator.CreateInstance(type);
                    TypeRegistry.PushConstructorInstance(L, value, type);
                    return 1;
                };
            }

            return L =>
            {
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        int top = LuaDll.lua_gettop(L);
                        int actual = top - 2 + 1;
                        if (actual < 0)
                        {
                            actual = 0;
                        }

                        if (!cores.TryGetValue(actual, out Func<IntPtr, int> core))
                        {
                            LuaCallbackBoundary.Throw(
                                $"zlua: no constructor for {TypeRegistry.GetLuaFullName(type)} with {actual} argument(s)");
                        }

                        return core(L);
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

        /// <summary>
        /// Same-arity ctor overloads (e.g. Calculator(int) vs Calculator(string)): try each until Pop succeeds.
        /// </summary>
        private static Func<IntPtr, int> BuildSameArityDispatchCore(List<ConstructorInfo> ctors, Type type)
        {
            var cores = new Func<IntPtr, int>[ctors.Count];
            for (int i = 0; i < ctors.Count; i++)
            {
                cores[i] = BuildDirectCore(ctors[i], type);
            }

            return L =>
            {
                Exception lastError = null;
                for (int i = 0; i < cores.Length; i++)
                {
                    int top = LuaDll.lua_gettop(L);
                    try
                    {
                        return cores[i](L);
                    }
                    catch (Exception ex)
                    {
                        lastError = ex;
                        LuaDll.lua_settop(L, top);
                    }
                }

                if (lastError != null)
                {
                    throw lastError;
                }

                LuaCallbackBoundary.Throw(
                    $"zlua: no matching constructor for {TypeRegistry.GetLuaFullName(type)}");
                return 0;
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
                        return core(L);
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
