using System;
using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;
using ZLua;
using ZLua.Marshal;
using ZLua.Mt;
using ZLua.DelegateImpl;

namespace ZLua.MethodBridge
{
    /// <summary>
    /// Compiles Lua → C# property get/set bridges for default-marshal parameterless public properties.
    /// </summary>
    internal static class PropertyBridgeFactory
    {
        private readonly struct GetterKey : IEquatable<GetterKey>
        {
            internal readonly PropertyInfo Property;
            internal readonly bool IsStatic;

            internal GetterKey(PropertyInfo property, bool isStatic)
            {
                Property = property;
                IsStatic = isStatic;
            }

            public bool Equals(GetterKey other)
            {
                return Property == other.Property && IsStatic == other.IsStatic;
            }

            public override bool Equals(object obj)
            {
                return obj is GetterKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Property?.MetadataToken ?? 0) * 397) ^ (IsStatic ? 1 : 0);
                }
            }
        }

        private readonly struct SetterKey : IEquatable<SetterKey>
        {
            internal readonly PropertyInfo Property;
            internal readonly bool IsStatic;

            internal SetterKey(PropertyInfo property, bool isStatic)
            {
                Property = property;
                IsStatic = isStatic;
            }

            public bool Equals(SetterKey other)
            {
                return Property == other.Property && IsStatic == other.IsStatic;
            }

            public override bool Equals(object obj)
            {
                return obj is SetterKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Property?.MetadataToken ?? 0) * 397) ^ (IsStatic ? 1 : 0);
                }
            }
        }

        private static readonly ConcurrentDictionary<GetterKey, LuaCSFunction> GetterCache =
            new ConcurrentDictionary<GetterKey, LuaCSFunction>();

        private static readonly ConcurrentDictionary<SetterKey, LuaCSFunction> SetterCache =
            new ConcurrentDictionary<SetterKey, LuaCSFunction>();

        private static readonly MethodInfo PushReturnMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PushReturn),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopTargetMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopTarget),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo SetBoxedInstancePropertyMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.SetBoxedInstanceProperty),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo ToLuaErrorMethod = typeof(LuaCallbackBoundary).GetMethod(
            nameof(LuaCallbackBoundary.ToLuaError),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo EnterLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.EnterLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo LeaveLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.LeaveLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopBoolMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopBool),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopCharMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopChar),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopByteMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopByte),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopSByteMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopSByte),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt16Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopInt16),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt16Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopUInt16),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt32Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopInt32),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt32Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopUInt32),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopInt64Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopInt64),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUInt64Method = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopUInt64),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopSingleMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopSingle),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopDoubleMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopDouble),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopIntPtrMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopIntPtr),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopUIntPtrMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopUIntPtr),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo PopStringMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.PopString),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool TryCreateGetter(PropertyInfo property, bool isStatic, out LuaCSFunction getter)
        {
            getter = null;
            MethodInfo getterMethod = property?.GetGetMethod(nonPublic: false);
            if (getterMethod == null || !CanFastAccess(property, isStatic, getterMethod) || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                getter = GetterCache.GetOrAdd(
                    new GetterKey(property, isStatic),
                    key => CompileGetter(key.Property, getterMethod, key.IsStatic));
                return getter != null;
            }
            catch
            {
                return false;
            }
        }

        internal static bool TryCreateSetter(PropertyInfo property, bool isStatic, out LuaCSFunction setter)
        {
            setter = null;
            MethodInfo setterMethod = property?.GetSetMethod(nonPublic: false);
            if (setterMethod == null || !CanFastAccess(property, isStatic, setterMethod) || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                setter = SetterCache.GetOrAdd(
                    new SetterKey(property, isStatic),
                    key => CompileSetter(key.Property, setterMethod, key.IsStatic));
                return setter != null;
            }
            catch
            {
                return false;
            }
        }

        private static bool EnsureHelperMethods()
        {
            return PushReturnMethod != null
                && PopTargetMethod != null
                && ToLuaErrorMethod != null
                && EnterLuaToCSharpMethod != null
                && LeaveLuaToCSharpMethod != null;
        }

        private static bool CanFastAccess(PropertyInfo property, bool isStatic, MethodInfo accessor)
        {
            if (property == null || accessor == null || property.GetIndexParameters().Length != 0)
            {
                return false;
            }

            if (accessor.IsStatic != isStatic)
            {
                return false;
            }

            if (property.DeclaringType == null || property.DeclaringType.ContainsGenericParameters)
            {
                return false;
            }

            if (property.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                return false;
            }

            if (!IsFastType(property.PropertyType))
            {
                return false;
            }

            return MethodBridgeFactory.CanFastInvoke(accessor, isStatic);
        }

        private static bool IsFastType(Type type)
        {
            if (Nullable.GetUnderlyingType(type) != null)
            {
                return false;
            }

            Type targetType = type;
            if (targetType.IsByRef || targetType == typeof(object) || targetType == typeof(TypedReference))
            {
                return false;
            }

            if (targetType.IsPrimitive
                || targetType == typeof(string)
                || targetType == typeof(decimal)
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr))
            {
                return true;
            }

            if (targetType.IsEnum || StructMarshaling.IsStructType(targetType))
            {
                return true;
            }

            if (targetType.IsArray)
            {
                return true;
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                return true;
            }

            return targetType.IsClass || targetType.IsInterface;
        }

        private static LuaCSFunction CompileGetter(PropertyInfo property, MethodInfo getterMethod, bool isStatic)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

            Expression callExpression;
            if (isStatic)
            {
                callExpression = Expression.Call(getterMethod);
            }
            else
            {
                Expression targetExpression = Expression.Convert(
                    Expression.Call(
                        PopTargetMethod,
                        luaStateParam,
                        Expression.Constant(1),
                        Expression.Constant(property.DeclaringType)),
                    property.DeclaringType);
                callExpression = Expression.Call(targetExpression, getterMethod);
            }

            Expression pushReturn = BridgeExpressionBuilder.BuildPushReturn(
                luaStateParam,
                property.PropertyType,
                callExpression);

            return CompileWrapped(luaStateParam, exceptionParam, pushReturn);
        }

        private static LuaCSFunction CompileSetter(PropertyInfo property, MethodInfo setterMethod, bool isStatic)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

            Expression body;
            if (!isStatic && StructMarshaling.IsStructType(property.DeclaringType))
            {
                if (SetBoxedInstancePropertyMethod == null)
                {
                    throw new InvalidOperationException("SetBoxedInstanceProperty helper missing");
                }

                body = Expression.Call(
                    SetBoxedInstancePropertyMethod,
                    luaStateParam,
                    Expression.Constant(property),
                    Expression.Constant(1),
                    Expression.Constant(3));
            }
            else
            {
                Expression valueExpression = BridgeExpressionBuilder.BuildPopArgument(luaStateParam, 3, property.PropertyType);
                if (isStatic)
                {
                    body = Expression.Block(
                        Expression.Call(setterMethod, valueExpression),
                        Expression.Constant(0));
                }
                else
                {
                    Expression targetExpression = Expression.Convert(
                        Expression.Call(
                            PopTargetMethod,
                            luaStateParam,
                            Expression.Constant(1),
                            Expression.Constant(property.DeclaringType)),
                        property.DeclaringType);
                    body = Expression.Block(
                        Expression.Call(targetExpression, setterMethod, valueExpression),
                        Expression.Constant(0));
                }
            }

            return CompileWrapped(luaStateParam, exceptionParam, body);
        }

        private static LuaCSFunction CompileWrapped(
            ParameterExpression luaStateParam,
            ParameterExpression exceptionParam,
            Expression body)
        {
            Expression enterScope = Expression.Call(EnterLuaToCSharpMethod);
            Expression leaveScope = Expression.Call(LeaveLuaToCSharpMethod);

            Expression tryBody = Expression.TryFinally(
                Expression.Block(enterScope, body),
                leaveScope);

            Expression tryCatch = Expression.TryCatch(
                tryBody,
                Expression.Catch(exceptionParam, Expression.Call(ToLuaErrorMethod, luaStateParam, exceptionParam)));

            return Expression.Lambda<LuaCSFunction>(tryCatch, luaStateParam).Compile();
        }

        private static Expression BuildPopArgument(ParameterExpression luaStateParam, int index, Type propertyType)
        {
            Type targetType = Nullable.GetUnderlyingType(propertyType) ?? propertyType;
            MethodInfo popMethod = GetPopMethod(targetType);
            if (popMethod == null)
            {
                Expression popObject = Expression.Call(
                    PopTargetMethod,
                    luaStateParam,
                    Expression.Constant(index),
                    Expression.Constant(targetType));
                return Expression.Convert(popObject, propertyType);
            }

            Expression popValue = Expression.Call(popMethod, luaStateParam, Expression.Constant(index));
            return propertyType == targetType
                ? popValue
                : Expression.Convert(popValue, propertyType);
        }

        private static MethodInfo GetPopMethod(Type targetType)
        {
            if (targetType == typeof(bool))
            {
                return PopBoolMethod;
            }

            if (targetType == typeof(char))
            {
                return PopCharMethod;
            }

            if (targetType == typeof(byte))
            {
                return PopByteMethod;
            }

            if (targetType == typeof(sbyte))
            {
                return PopSByteMethod;
            }

            if (targetType == typeof(short))
            {
                return PopInt16Method;
            }

            if (targetType == typeof(ushort))
            {
                return PopUInt16Method;
            }

            if (targetType == typeof(int))
            {
                return PopInt32Method;
            }

            if (targetType == typeof(uint))
            {
                return PopUInt32Method;
            }

            if (targetType == typeof(long))
            {
                return PopInt64Method;
            }

            if (targetType == typeof(ulong))
            {
                return PopUInt64Method;
            }

            if (targetType == typeof(float))
            {
                return PopSingleMethod;
            }

            if (targetType == typeof(double))
            {
                return PopDoubleMethod;
            }

            if (targetType == typeof(IntPtr))
            {
                return PopIntPtrMethod;
            }

            if (targetType == typeof(UIntPtr))
            {
                return PopUIntPtrMethod;
            }

            if (targetType == typeof(string))
            {
                return PopStringMethod;
            }

            return null;
        }
    }
}
