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
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal static class FieldEmitter
    {
        internal static void Write(IntPtr L, int getterTableRef, int setterTableRef, FieldInfo field, bool isStatic, bool isByVal)
        {
            if (field == null)
            {
                return;
            }

            if (!BridgeMarshaling.IsSupportedType(field.FieldType))
            {
                // Soft-skip: cannot throw under reverse P/Invoke (Tuanjie Mono SIGSEGV).
                return;
            }

            ClosurePin.WriteToTable(
                L,
                getterTableRef,
                field.Name,
                CreateLazyAccessor(getterTableRef, field.Name, () => CompileGetter(field, isStatic, isByVal)));

            if (!field.IsInitOnly && !field.IsLiteral)
            {
                ClosurePin.WriteToTable(
                    L,
                    setterTableRef,
                    field.Name,
                    CreateLazyAccessor(setterTableRef, field.Name, () => CompileSetter(field, isStatic, isByVal)));
            }
        }

        private static LuaMarshalBinding ResolveFieldBinding(FieldInfo field, LuaMarshalDirection direction)
        {
            return LuaMarshalAsValidation.Current.ResolveMemberBinding(field, direction);
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

        private static LuaCSFunction CompileGetter(FieldInfo field, bool isStatic, bool isByVal)
        {
            LuaMarshalBinding binding = ResolveFieldBinding(field, LuaMarshalDirection.CSharpToLua);
            if (binding.IsComposite || binding.MarshalType == LuaMarshalType.ActualTypeUserData)
            {
                return CompileGetterInterpreted(field, isStatic, isByVal, binding);
            }

            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression value;
            if (isStatic)
            {
                value = Expression.Field(null, field);
            }
            else if (isByVal && declaringType.IsValueType)
            {
                value = Expression.Call(
                    EmitMethods.ByValGetField,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(field, typeof(FieldInfo)));
            }
            else
            {
                Expression targetObj = Expression.Call(
                    EmitMethods.PopTarget,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(false));
                value = Expression.Field(Expression.Convert(targetObj, declaringType), field);
            }

            Expression body = TypedBridgeEmit.PushMember(L, fieldType, value, binding);

            Func<IntPtr, int> core = Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
            return Wrap(core);
        }

        private static LuaCSFunction CompileGetterInterpreted(
            FieldInfo field,
            bool isStatic,
            bool isByVal,
            LuaMarshalBinding binding)
        {
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            return Wrap(L =>
            {
                object value;
                if (isStatic)
                {
                    value = field.GetValue(null);
                }
                else if (isByVal && declaringType != null && declaringType.IsValueType)
                {
                    value = ByValInstanceOps.GetField(L, 1, declaringType, field);
                }
                else
                {
                    object target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal: false);
                    value = field.GetValue(target);
                }

                return CompositeMarshal.Push(L, value, fieldType, binding);
            });
        }

        private static LuaCSFunction CompileSetter(FieldInfo field, bool isStatic, bool isByVal)
        {
            LuaMarshalBinding binding = ResolveFieldBinding(field, LuaMarshalDirection.LuaToCSharp);
            if (binding.IsComposite || binding.MarshalType == LuaMarshalType.ActualTypeUserData)
            {
                return CompileSetterInterpreted(field, isStatic, isByVal, binding);
            }

            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression typedValue = TypedBridgeEmit.PopMember(L, 2, fieldType, binding);

            Expression body;
            if (isStatic)
            {
                body = Expression.Block(
                    Expression.Assign(Expression.Field(null, field), typedValue),
                    Expression.Constant(0));
            }
            else if (isByVal && declaringType.IsValueType)
            {
                Expression valueObj = Expression.Convert(typedValue, typeof(object));
                body = Expression.Block(
                    Expression.Call(
                        EmitMethods.ByValSetField,
                        L,
                        Expression.Constant(1),
                        Expression.Constant(declaringType, typeof(Type)),
                        Expression.Constant(field, typeof(FieldInfo)),
                        valueObj),
                    Expression.Constant(0));
            }
            else
            {
                Expression targetObj = Expression.Call(
                    EmitMethods.PopTarget,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(false));
                body = Expression.Block(
                    Expression.Assign(
                        Expression.Field(Expression.Convert(targetObj, declaringType), field),
                        typedValue),
                    Expression.Constant(0));
            }

            Func<IntPtr, int> core = Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
            return Wrap(core);
        }

        private static LuaCSFunction CompileSetterInterpreted(
            FieldInfo field,
            bool isStatic,
            bool isByVal,
            LuaMarshalBinding binding)
        {
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            return Wrap(L =>
            {
                object value = CompositeMarshal.Pop(L, 2, fieldType, binding);
                if (isStatic)
                {
                    field.SetValue(null, value);
                }
                else if (isByVal && declaringType != null && declaringType.IsValueType)
                {
                    ByValInstanceOps.SetField(L, 1, declaringType, field, value);
                }
                else
                {
                    object target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal: false);
                    field.SetValue(target, value);
                }

                return 0;
            });
        }

        private static LuaCSFunction Wrap(Func<IntPtr, int> core)
        {
            return L =>
            {
                LuaCallbackBoundary.Enter();
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
                    LuaCallbackBoundary.Leave();
                }
            };
        }
    }
}
