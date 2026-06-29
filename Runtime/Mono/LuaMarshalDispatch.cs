using System;
using System.Reflection;

namespace ZLua
{
    internal static class LuaMarshalDispatch
    {
        internal static bool CanConvert(IntPtr luaState, int luaIndex, Type parameterType, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return false;
            }

            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;
            if (targetType.IsByRef)
            {
                targetType = targetType.GetElementType();
            }

            LuaDataType luaType = LuaDll.lua_type(luaState, luaIndex);
            if (luaType == LuaDataType.Nil)
            {
                return !targetType.IsValueType
                    || Nullable.GetUnderlyingType(parameterType) != null
                    || PointerMarshaling.IsPointerLikeType(targetType);
            }

            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (targetType == typeof(byte[]))
                    {
                        return luaType == LuaDataType.String;
                    }

                    if (targetType == typeof(string))
                    {
                        return luaType == LuaDataType.String;
                    }

                    return false;

                case LuaMarshalType.UserData:
                    return CanConvertUserData(luaState, luaIndex, targetType, luaType);

                default:
                    return false;
            }
        }

        internal static object Read(IntPtr luaState, int luaIndex, Type type, LuaMarshalType marshalType)
        {
            Type targetType = Nullable.GetUnderlyingType(type) ?? type;
            if (targetType.IsByRef)
            {
                targetType = targetType.GetElementType();
            }

            if (LuaDll.lua_type(luaState, luaIndex) == LuaDataType.Nil)
            {
                if (PointerMarshaling.IsPointerLikeType(targetType))
                {
                    return PointerMarshaling.BoxPointerForInvoke(IntPtr.Zero, targetType);
                }

                return null;
            }

            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (targetType == typeof(byte[]))
                    {
                        return LuaMarshalBytes.ReadByteArray(luaState, luaIndex);
                    }

                    if (targetType == typeof(string))
                    {
                        return LuaMarshalBytes.ReadStringAsOctets(luaState, luaIndex);
                    }

                    break;

                case LuaMarshalType.UserData:
                    return ReadUserData(luaState, luaIndex, targetType);
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs read for {targetType.Name} ({marshalType})");
        }

        internal static int PushReturn(IntPtr luaState, Type returnType, object ret, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return -1;
            }

            Type unwrappedReturnType = Nullable.GetUnderlyingType(returnType) ?? returnType;
            if (IsNullNullableReturn(returnType, ret))
            {
                LuaDll.lua_pushnil(luaState);
                return 1;
            }

            object value = UnwrapNullableValue(ret);
            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (unwrappedReturnType == typeof(byte[]))
                    {
                        LuaMarshalBytes.PushByteArray(luaState, value as byte[]);
                        return 1;
                    }

                    if (unwrappedReturnType == typeof(string))
                    {
                        LuaMarshalBytes.PushStringAsOctets(luaState, value as string);
                        return 1;
                    }

                    break;

                case LuaMarshalType.OpaqueLightUserData:
                    if (ValueTypeMarshaling.IsStructType(unwrappedReturnType))
                    {
                        PushOpaqueStruct(luaState, value, unwrappedReturnType);
                        return 1;
                    }

                    break;

                case LuaMarshalType.UserData:
                    PushUserData(luaState, value, unwrappedReturnType);
                    return 1;
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push for {unwrappedReturnType.Name} ({marshalType})");
        }

        internal static void PushArgument(IntPtr luaState, object value, Type type, LuaMarshalType marshalType)
        {
            Type targetType = Nullable.GetUnderlyingType(type) ?? type;
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            switch (marshalType)
            {
                case LuaMarshalType.Bytes:
                    if (targetType == typeof(byte[]))
                    {
                        LuaMarshalBytes.PushByteArray(luaState, value as byte[]);
                        return;
                    }

                    if (targetType == typeof(string))
                    {
                        LuaMarshalBytes.PushStringAsOctets(luaState, value as string);
                        return;
                    }

                    break;

                case LuaMarshalType.OpaqueLightUserData:
                    PushOpaqueStruct(luaState, value, targetType);
                    return;

                case LuaMarshalType.UserData:
                    PushUserData(luaState, value, targetType);
                    return;
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push arg for {targetType.Name} ({marshalType})");
        }

        private static void PushOpaqueStruct(IntPtr luaState, object value, Type structType)
        {
            if (!ValueTypeMarshaling.IsStructType(structType))
            {
                throw new NotSupportedException($"unsupported opaque push for {structType.Name}");
            }

            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            if (value.GetType() != structType)
            {
                throw new NotSupportedException($"unsupported struct value for {structType.Name}");
            }

            IntPtr handle = StructOpaqueScope.RegisterStruct(value, structType);
            PointerMarshaling.PushPointer(luaState, handle);
        }

        private static bool CanConvertUserData(IntPtr luaState, int luaIndex, Type targetType, LuaDataType luaType)
        {
            if (luaType != LuaDataType.UserData)
            {
                return false;
            }

            if (targetType.IsEnum)
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed != null
                    && boxed.GetType() == targetType;
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed != null
                    && boxed.GetType() == targetType;
            }

            if (targetType.IsArray)
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Array array
                    && targetType.IsAssignableFrom(array.GetType());
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Delegate del
                    && targetType.IsAssignableFrom(del.GetType());
            }

            if (targetType.IsClass || targetType.IsInterface)
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed != null
                    && targetType.IsAssignableFrom(boxed.GetType());
            }

            if (targetType == typeof(object))
            {
                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out _);
            }

            return ValueTypeMarshaling.TryReadBoxedValueUserData(luaState, luaIndex, targetType, out _);
        }

        private static object ReadUserData(IntPtr luaState, int luaIndex, Type targetType)
        {
            if (targetType.IsEnum)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed != null
                    && boxed.GetType() == targetType)
                {
                    return boxed;
                }

                throw new NotSupportedException($"unsupported enum userdata for {targetType.Name}");
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object structValue)
                    && structValue != null
                    && structValue.GetType() == targetType)
                {
                    return structValue;
                }

                throw new NotSupportedException($"unsupported struct userdata for {targetType.Name}");
            }

            if (targetType.IsArray)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object arrayValue)
                    && arrayValue is Array array
                    && targetType.IsAssignableFrom(array.GetType()))
                {
                    return array;
                }

                throw new NotSupportedException($"unsupported array userdata for {targetType.Name}");
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Delegate del
                    && targetType.IsAssignableFrom(del.GetType()))
                {
                    return del;
                }

                throw new NotSupportedException($"unsupported delegate userdata for {targetType.Name}");
            }

            if (targetType.IsClass || targetType.IsInterface)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object referenceValue)
                    && referenceValue != null
                    && targetType.IsAssignableFrom(referenceValue.GetType()))
                {
                    return referenceValue;
                }

                throw new NotSupportedException($"unsupported reference userdata for {targetType.Name}");
            }

            if (targetType == typeof(object))
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed))
                {
                    return boxed;
                }

                throw new NotSupportedException("unsupported userdata for object");
            }

            if (ValueTypeMarshaling.TryReadBoxedValueUserData(luaState, luaIndex, targetType, out object primitive))
            {
                return primitive;
            }

            throw new NotSupportedException($"unsupported userdata for {targetType.Name}");
        }

        private static void PushUserData(IntPtr luaState, object value, Type targetType)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            Type runtimeType = value.GetType();
            if (targetType.IsEnum)
            {
                if (runtimeType != targetType)
                {
                    throw new NotSupportedException($"unsupported enum value for {targetType.Name}");
                }

                LuaManagerObject.PushConstructorInstance(luaState, value, targetType);
                return;
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                if (runtimeType != targetType)
                {
                    throw new NotSupportedException($"unsupported struct value for {targetType.Name}");
                }

                LuaManagerObject.PushConstructorInstance(luaState, value, targetType);
                return;
            }

            if (targetType.IsArray || typeof(Delegate).IsAssignableFrom(targetType)
                || targetType.IsClass || targetType.IsInterface)
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, ResolveInstancePushType(targetType, value));
                return;
            }

            if (targetType == typeof(object))
            {
                Type actualType = runtimeType;
                if (actualType.IsEnum || ValueTypeMarshaling.IsStructType(actualType)
                    || actualType.IsArray || typeof(Delegate).IsAssignableFrom(actualType)
                    || actualType.IsClass || actualType.IsInterface)
                {
                    LuaManagerObject.PushConstructorInstance(luaState, value, ResolveInstancePushType(actualType, value));
                    return;
                }

                ValueTypeMarshaling.PushBoxedValueUserData(luaState, value);
                return;
            }

            ValueTypeMarshaling.PushBoxedValueUserData(luaState, value);
        }

        private static Type ResolveInstancePushType(Type declaredType, object instance)
        {
            return instance?.GetType() ?? declaredType;
        }

        private static bool IsNullNullableReturn(Type returnType, object ret)
        {
            if (Nullable.GetUnderlyingType(returnType) == null)
            {
                return ret == null;
            }

            if (ret == null)
            {
                return true;
            }

            Type runtimeType = ret.GetType();
            if (runtimeType.IsGenericType && runtimeType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                PropertyInfo hasValueProperty = runtimeType.GetProperty("HasValue", BindingFlags.Public | BindingFlags.Instance);
                if (hasValueProperty?.GetValue(ret) is bool hasValue)
                {
                    return !hasValue;
                }
            }

            return false;
        }

        private static object UnwrapNullableValue(object ret)
        {
            if (ret == null)
            {
                return null;
            }

            Type runtimeType = ret.GetType();
            if (runtimeType.IsGenericType && runtimeType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                return runtimeType.GetProperty("Value", BindingFlags.Public | BindingFlags.Instance)?.GetValue(ret);
            }

            return ret;
        }
    }
}
