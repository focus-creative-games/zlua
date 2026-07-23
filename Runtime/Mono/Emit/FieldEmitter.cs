using System;
using System.Collections.Generic;
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
                throw EmitException.ForMember(field.DeclaringType, field.Name, "unsupported field type");
            }

            ClosurePin.WriteToTable(L, getterTableRef, field.Name, CompileGetter(field, isStatic, isByVal));

            if (!field.IsInitOnly && !field.IsLiteral)
            {
                ClosurePin.WriteToTable(L, setterTableRef, field.Name, CompileSetter(field, isStatic, isByVal));
            }
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
            }
            else
            {
                Expression targetObj = Expression.Call(
                    EmitMethods.PopTarget,
                    L,
                    Expression.Constant(1),
                    Expression.Constant(declaringType, typeof(Type)),
                    Expression.Constant(isByVal));
                value = Expression.Field(Expression.Convert(targetObj, declaringType), field);
            }

            Expression body = Expression.Call(
                EmitMethods.PushReturn,
                L,
                Expression.Constant(fieldType, typeof(Type)),
                Expression.Convert(value, typeof(object)));

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
            Expression typedValue = Expression.Convert(valueObj, fieldType);

            Expression body;
            if (isStatic)
            {
                body = Expression.Block(
                    Expression.Assign(Expression.Field(null, field), typedValue),
                    Expression.Constant(0));
            }
            else if (isByVal && declaringType.IsValueType)
            {
                ParameterExpression targetLocal = Expression.Variable(typeof(object), "target");
                ParameterExpression typedTarget = Expression.Variable(declaringType, "typedTarget");
                var exprs = new List<Expression>
                {
                    Expression.Assign(
                        targetLocal,
                        Expression.Call(
                            EmitMethods.PopTarget,
                            L,
                            Expression.Constant(1),
                            Expression.Constant(declaringType, typeof(Type)),
                            Expression.Constant(true))),
                    Expression.Assign(typedTarget, Expression.Convert(targetLocal, declaringType)),
                    Expression.Assign(Expression.Field(typedTarget, field), typedValue),
                    Expression.Call(
                        EmitMethods.StructWriteBack,
                        L,
                        Expression.Constant(1),
                        Expression.Convert(typedTarget, typeof(object)),
                        Expression.Constant(declaringType, typeof(Type))),
                    Expression.Constant(0),
                };
                body = Expression.Block(new[] { targetLocal, typedTarget }, exprs);
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
