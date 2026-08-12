using System;
using System.Linq.Expressions;
using System.Reflection;
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
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression value;
            if (isStatic)
            {
                value = Expression.Field(null, field);
                value = Expression.Convert(value, typeof(object));
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
                value = Expression.Convert(
                    Expression.Field(Expression.Convert(targetObj, declaringType), field),
                    typeof(object));
            }

            Expression body = Expression.Call(
                EmitMethods.PushReturn,
                L,
                Expression.Constant(fieldType, typeof(Type)),
                value);

            Func<IntPtr, int> core = Expression.Lambda<Func<IntPtr, int>>(body, L).Compile();
            return Wrap(core);
        }

        private static LuaCSFunction CompileSetter(FieldInfo field, bool isStatic, bool isByVal)
        {
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;
            ParameterExpression L = Expression.Parameter(typeof(IntPtr), "L");

            Expression valueObj = Expression.Call(
                EmitMethods.PopArg,
                L,
                Expression.Constant(2),
                Expression.Constant(fieldType, typeof(Type)));

            Expression body;
            if (isStatic)
            {
                Expression typedValue = Expression.Convert(valueObj, fieldType);
                body = Expression.Block(
                    Expression.Assign(Expression.Field(null, field), typedValue),
                    Expression.Constant(0));
            }
            else if (isByVal && declaringType.IsValueType)
            {
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
                Expression typedValue = Expression.Convert(valueObj, fieldType);
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
