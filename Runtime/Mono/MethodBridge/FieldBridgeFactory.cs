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
    /// Compiles Lua → C# field get/set bridges for default-marshal public fields at registration time.
    /// </summary>
    internal static class FieldBridgeFactory
    {
        private readonly struct GetterKey : IEquatable<GetterKey>
        {
            internal readonly FieldInfo Field;
            internal readonly bool IsStatic;

            internal GetterKey(FieldInfo field, bool isStatic)
            {
                Field = field;
                IsStatic = isStatic;
            }

            public bool Equals(GetterKey other)
            {
                return Field == other.Field && IsStatic == other.IsStatic;
            }

            public override bool Equals(object obj)
            {
                return obj is GetterKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Field?.MetadataToken ?? 0) * 397) ^ (IsStatic ? 1 : 0);
                }
            }
        }

        private readonly struct SetterKey : IEquatable<SetterKey>
        {
            internal readonly FieldInfo Field;
            internal readonly bool IsStatic;

            internal SetterKey(FieldInfo field, bool isStatic)
            {
                Field = field;
                IsStatic = isStatic;
            }

            public bool Equals(SetterKey other)
            {
                return Field == other.Field && IsStatic == other.IsStatic;
            }

            public override bool Equals(object obj)
            {
                return obj is SetterKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Field?.MetadataToken ?? 0) * 397) ^ (IsStatic ? 1 : 0);
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

        private static readonly MethodInfo SetBoxedInstanceFieldMethod = typeof(MethodBridgeMarshaling).GetMethod(
            nameof(MethodBridgeMarshaling.SetBoxedInstanceField),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool TryCreateGetter(FieldInfo field, bool isStatic, out LuaCSFunction getter)
        {
            getter = null;
            if (!CanFastAccess(field, isStatic) || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                getter = GetterCache.GetOrAdd(new GetterKey(field, isStatic), key => CompileGetter(key.Field, key.IsStatic));
                return getter != null;
            }
            catch
            {
                return false;
            }
        }

        internal static bool TryCreateSetter(FieldInfo field, bool isStatic, out LuaCSFunction setter)
        {
            setter = null;
            if (!CanFastAccess(field, isStatic) || field.IsInitOnly || field.IsLiteral || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                setter = SetterCache.GetOrAdd(new SetterKey(field, isStatic), key => CompileSetter(key.Field, key.IsStatic));
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

        private static bool CanFastAccess(FieldInfo field, bool isStatic)
        {
            if (field == null || !field.IsPublic || field.IsStatic != isStatic)
            {
                return false;
            }

            if (field.DeclaringType == null || field.DeclaringType.ContainsGenericParameters)
            {
                return false;
            }

            if (field.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                return false;
            }

            return IsFastType(field.FieldType);
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

        private static LuaCSFunction CompileGetter(FieldInfo field, bool isStatic)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

            Expression readField;
            if (isStatic)
            {
                readField = Expression.Field(null, field);
            }
            else
            {
                Expression targetExpression = Expression.Convert(
                    Expression.Call(
                        PopTargetMethod,
                        luaStateParam,
                        Expression.Constant(1),
                        Expression.Constant(field.DeclaringType)),
                    field.DeclaringType);
                readField = Expression.Field(targetExpression, field);
            }

            Expression pushReturn = BridgeExpressionBuilder.BuildPushReturn(
                luaStateParam,
                field.FieldType,
                readField);

            return CompileWrapped(luaStateParam, exceptionParam, pushReturn);
        }

        private static LuaCSFunction CompileSetter(FieldInfo field, bool isStatic)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");

            Expression body;
            if (!isStatic && StructMarshaling.IsStructType(field.DeclaringType))
            {
                if (SetBoxedInstanceFieldMethod == null)
                {
                    throw new InvalidOperationException("SetBoxedInstanceField helper missing");
                }

                body = Expression.Call(
                    SetBoxedInstanceFieldMethod,
                    luaStateParam,
                    Expression.Constant(field),
                    Expression.Constant(1),
                    Expression.Constant(3));
            }
            else
            {
                Expression valueExpression = BridgeExpressionBuilder.BuildPopArgument(luaStateParam, 3, field.FieldType);
                Expression assignField;
                if (isStatic)
                {
                    assignField = Expression.Assign(Expression.Field(null, field), valueExpression);
                }
                else
                {
                    Expression targetExpression = Expression.Convert(
                        Expression.Call(
                            PopTargetMethod,
                            luaStateParam,
                            Expression.Constant(1),
                            Expression.Constant(field.DeclaringType)),
                        field.DeclaringType);
                    assignField = Expression.Assign(Expression.Field(targetExpression, field), valueExpression);
                }

                body = Expression.Block(assignField, Expression.Constant(0));
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

        private static Expression BuildPopArgument(ParameterExpression luaStateParam, int index, Type fieldType)
        {
            Type targetType = Nullable.GetUnderlyingType(fieldType) ?? fieldType;
            MethodInfo popMethod = GetPopMethod(targetType);
            if (popMethod == null)
            {
                Expression popObject = Expression.Call(
                    PopTargetMethod,
                    luaStateParam,
                    Expression.Constant(index),
                    Expression.Constant(targetType));
                return Expression.Convert(popObject, fieldType);
            }

            Expression popValue = Expression.Call(popMethod, luaStateParam, Expression.Constant(index));
            return fieldType == targetType
                ? popValue
                : Expression.Convert(popValue, fieldType);
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
