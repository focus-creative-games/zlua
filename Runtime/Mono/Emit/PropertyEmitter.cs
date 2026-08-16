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
using System.Linq.Expressions;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal static class PropertyEmitter
    {
        internal static void Write(IntPtr L, int getterTableRef, int setterTableRef, PropertyInfo property, bool isStatic, bool isByVal)
        {
            if (property == null)
            {
                return;
            }

            ParameterInfo[] indexParams = property.GetIndexParameters();
            if (indexParams != null && indexParams.Length > 0)
            {
                // Soft-skip: cannot throw under reverse P/Invoke (Tuanjie Mono SIGSEGV).
                return;
            }

            if (!BridgeMarshaling.IsSupportedType(property.PropertyType))
            {
                // Soft-skip: cannot throw under reverse P/Invoke (Tuanjie Mono SIGSEGV).
                return;
            }

            MethodInfo getter = property.GetGetMethod(nonPublic: false);
            MethodInfo setter = property.GetSetMethod(nonPublic: false);

            if (getter != null)
            {
                ClosurePin.WriteToTable(
                    L,
                    getterTableRef,
                    property.Name,
                    CreateLazyAccessor(
                        getterTableRef,
                        property.Name,
                        () => CompileGetter(property, getter, isStatic, isByVal)));
            }

            if (setter != null)
            {
                ClosurePin.WriteToTable(
                    L,
                    setterTableRef,
                    property.Name,
                    CreateLazyAccessor(
                        setterTableRef,
                        property.Name,
                        () => CompileSetter(property, setter, isStatic, isByVal)));
            }
        }

        private sealed class LazyAccessorBind
        {
            public int TableRef;
            public string Name;
            public Func<LuaCSFunction> Factory;
            public readonly object Gate = new object();
            public volatile LuaCSFunction Compiled;
        }

        private static LuaCSFunction CreateLazyAccessor(int tableRef, string name, Func<LuaCSFunction> factory)
        {
            var bind = new LazyAccessorBind
            {
                TableRef = tableRef,
                Name = name,
                Factory = factory,
            };

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
                                compiled = bind.Factory();
                                ClosurePin.WriteToTable(L, bind.TableRef, bind.Name, compiled);
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

        private static LuaCSFunction CompileGetter(PropertyInfo property, MethodInfo getter, bool isStatic, bool isByVal)
        {
            Type declaringType = property.DeclaringType;
            Type propertyType = property.PropertyType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression body;
            if (isStatic)
            {
                Expression value = Expression.Call(getter);
                body = Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(propertyType, typeof(Type)),
                    Expression.Convert(value, typeof(object)));
            }
            else if (isByVal && declaringType.IsValueType)
            {
                Expression value = Expression.Call(
                    EmitMethods.ByValInvokePropertyGetter,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(getter, typeof(MethodInfo)));
                body = Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(propertyType, typeof(Type)),
                    value);
            }
            else
            {
                Expression targetObj = Expression.Call(
                    EmitMethods.PopTarget,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(false));
                Expression typedTarget = Expression.Convert(targetObj, declaringType);
                Expression value = Expression.Call(typedTarget, getter);
                body = Expression.Call(
                    EmitMethods.PushReturn,
                    L,
                    Expression.Constant(propertyType, typeof(Type)),
                    Expression.Convert(value, typeof(object)));
            }

            Func<IntPtr, int> core = Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
            return Wrap(core);
        }

        private static LuaCSFunction CompileSetter(PropertyInfo property, MethodInfo setter, bool isStatic, bool isByVal)
        {
            Type declaringType = property.DeclaringType;
            Type propertyType = property.PropertyType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression valueObj = Expression.Call(
                EmitMethods.PopArg,
                L,
                Expression.Constant(2),
                Expression.Constant(propertyType, typeof(Type)));

            Expression body;
            if (isStatic)
            {
                Expression typedValue = Expression.Convert(valueObj, propertyType);
                body = Expression.Block(
                    Expression.Call(setter, typedValue),
                    Expression.Constant(0));
            }
            else if (isByVal && declaringType.IsValueType)
            {
                body = Expression.Block(
                    Expression.Call(
                        EmitMethods.ByValInvokePropertySetter,
                        L,
                        Expression.Constant(1),
                        Expression.Constant(declaringType, typeof(Type)),
                        Expression.Constant(setter, typeof(MethodInfo)),
                        valueObj),
                    Expression.Constant(0));
            }
            else
            {
                Expression typedValue = Expression.Convert(valueObj, propertyType);
                Expression targetObj = Expression.Call(
                    EmitMethods.PopTarget,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(false));
                body = Expression.Block(
                    Expression.Call(Expression.Convert(targetObj, declaringType), setter, typedValue),
                    Expression.Constant(0));
            }

            Func<IntPtr, int> core = Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
            return Wrap(core);
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
                    LuaCallbackBoundary.Leave();
                }
            };
        }
    }
}
