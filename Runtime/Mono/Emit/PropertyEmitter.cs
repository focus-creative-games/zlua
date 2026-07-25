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
                throw EmitException.ForMember(property.DeclaringType, property.Name, "indexed properties not emitted yet");
            }

            if (!BridgeMarshaling.IsSupportedType(property.PropertyType))
            {
                throw EmitException.ForMember(property.DeclaringType, property.Name, "unsupported property type");
            }

            MethodInfo getter = property.GetGetMethod(nonPublic: false);
            MethodInfo setter = property.GetSetMethod(nonPublic: false);

            if (getter != null)
            {
                ClosurePin.WriteToTable(L, getterTableRef, property.Name, CompileGetter(property, getter, isStatic, isByVal));
            }

            if (setter != null)
            {
                ClosurePin.WriteToTable(L, setterTableRef, property.Name, CompileSetter(property, setter, isStatic, isByVal));
            }
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
