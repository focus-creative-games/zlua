using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace ZLua
{
    /// <summary>
    /// Mono 侧 struct / enum 实例 userdata：GCHandle 持有 box 后的对象，与 class 规则一致。
    /// </summary>
    internal static class ValueTypeMarshaling
    {
        private static int _boxedValueMetatableRef;

        internal static bool IsStructType(Type type)
        {
            return type.IsValueType
                && !type.IsEnum
                && !type.IsPrimitive
                && !type.IsPointer
                && !PointerMarshaling.IsFunctionPointerType(type)
                && !PointerMarshaling.IsTypedReference(type)
                && !PointerMarshaling.IsByRefLikeType(type);
        }

        internal static int PushBoxedInstance(IntPtr luaState, object instance, int typeTableIndex)
        {
            GCHandle handle = GCHandle.Alloc(instance);
            IntPtr handlePtr = GCHandle.ToIntPtr(handle);
            IntPtr userData = LuaDll.lua_newuserdatauv(luaState, (UIntPtr)IntPtr.Size, 0);
            Marshal.WriteIntPtr(userData, handlePtr);

            int absTypeTableIndex = LuaDll.lua_absindex(luaState, typeTableIndex);
            LuaDll.lua_pushstring(luaState, "__instance_mt");
            LuaDll.lua_rawget(luaState, absTypeTableIndex);
            if (LuaDll.lua_type(luaState, -1) != LuaDataType.Table)
            {
                LuaDll.lua_pop(luaState, 1);
                handle.Free();
                Marshal.WriteIntPtr(userData, IntPtr.Zero);
                LuaCallbackBoundary.Throw("zlua: instance metatable missing");
            }

            LuaDll.lua_setmetatable(luaState, -2);
            return 1;
        }

        internal static int PushBoxedValueUserData(IntPtr luaState, object boxedValue)
        {
            EnsureBoxedValueMetatable(luaState);
            GCHandle handle = GCHandle.Alloc(boxedValue);
            IntPtr handlePtr = GCHandle.ToIntPtr(handle);
            IntPtr userData = LuaDll.lua_newuserdatauv(luaState, (UIntPtr)IntPtr.Size, 0);
            Marshal.WriteIntPtr(userData, handlePtr);
            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, _boxedValueMetatableRef);
            LuaDll.lua_setmetatable(luaState, -2);
            return 1;
        }

        internal static object CopyBoxedStruct(object boxed, Type structType)
        {
            object copy = Activator.CreateInstance(structType);
            FieldInfo[] fields = structType.GetFields(BindingFlags.Public | BindingFlags.Instance);
            for (int i = 0; i < fields.Length; i++)
            {
                fields[i].SetValue(copy, fields[i].GetValue(boxed));
            }

            return copy;
        }

        internal static bool TryReadBoxedValueUserData(IntPtr luaState, int index, Type targetType, out object value)
        {
            value = null;
            if (LuaDll.lua_type(luaState, index) != LuaDataType.UserData)
            {
                return false;
            }

            if (!TryGetBoxedTarget(luaState, index, out object boxed) || boxed == null)
            {
                return false;
            }

            Type runtimeType = boxed.GetType();
            if (targetType.IsAssignableFrom(runtimeType))
            {
                value = boxed;
                return true;
            }

            if (targetType.IsEnum && runtimeType == targetType)
            {
                value = boxed;
                return true;
            }

            if (targetType.IsPrimitive || targetType == typeof(decimal))
            {
                try
                {
                    value = Convert.ChangeType(boxed, targetType);
                    return true;
                }
                catch
                {
                    return false;
                }
            }

            if (targetType == typeof(string) && boxed is string)
            {
                value = boxed;
                return true;
            }

            if (targetType == typeof(IntPtr) && boxed is IntPtr intPtr)
            {
                value = intPtr;
                return true;
            }

            if (targetType == typeof(UIntPtr) && boxed is UIntPtr uintPtr)
            {
                value = uintPtr;
                return true;
            }

            return false;
        }

        private static void EnsureBoxedValueMetatable(IntPtr luaState)
        {
            if (_boxedValueMetatableRef != 0)
            {
                return;
            }

            LuaDll.lua_createtable(luaState, 0, 0);
            _boxedValueMetatableRef = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
        }

        internal static bool TryGetBoxedTarget(IntPtr luaState, int index, out object target)
        {
            target = null;
            IntPtr userData = LuaDll.lua_touserdata(luaState, index);
            if (userData == IntPtr.Zero)
            {
                return false;
            }

            IntPtr handlePtr = Marshal.ReadIntPtr(userData);
            if (handlePtr == IntPtr.Zero)
            {
                return false;
            }

            GCHandle handle = GCHandle.FromIntPtr(handlePtr);
            if (!handle.IsAllocated)
            {
                return false;
            }

            target = handle.Target;
            return target != null;
        }

        internal static int ReleaseBoxedInstance(IntPtr luaState)
        {
            try
            {
                IntPtr userData = LuaDll.lua_touserdata(luaState, 1);
                if (userData == IntPtr.Zero)
                {
                    return 0;
                }

                IntPtr handlePtr = Marshal.ReadIntPtr(userData);
                if (handlePtr != IntPtr.Zero)
                {
                    GCHandle handle = GCHandle.FromIntPtr(handlePtr);
                    if (handle.IsAllocated)
                    {
                        handle.Free();
                    }

                    Marshal.WriteIntPtr(userData, IntPtr.Zero);
                }

                return 0;
            }
            catch
            {
                return 0;
            }
        }

        internal static bool TryReadEnumValue(IntPtr luaState, int index, Type enumType, out object value)
        {
            value = null;
            Type underlying = Enum.GetUnderlyingType(enumType);
            if (TryReadUnderlyingInteger(luaState, index, underlying, out object rawValue))
            {
                value = Enum.ToObject(enumType, rawValue);
                return true;
            }

            if (TryGetBoxedTarget(luaState, index, out object boxed) && boxed != null && boxed.GetType() == enumType)
            {
                value = boxed;
                return true;
            }

            return false;
        }

        internal static void PushUnderlyingInteger(IntPtr luaState, Type underlyingType, object rawValue)
        {
            switch (Type.GetTypeCode(underlyingType))
            {
                case TypeCode.SByte:
                case TypeCode.Byte:
                case TypeCode.Int16:
                case TypeCode.UInt16:
                case TypeCode.Int32:
                case TypeCode.UInt32:
                case TypeCode.Int64:
                case TypeCode.UInt64:
                    LuaDll.lua_pushinteger(luaState, Convert.ToInt64(rawValue));
                    break;
                default:
                    LuaDll.lua_pushnumber(luaState, Convert.ToDouble(rawValue));
                    break;
            }
        }

        internal static bool TryReadUnderlyingInteger(IntPtr luaState, int index, Type underlyingType, out object value)
        {
            value = null;
            if (LuaDll.lua_type(luaState, index) != LuaDataType.Number)
            {
                return false;
            }

            if (LuaDll.lua_isinteger(luaState, index) != 0)
            {
                return TryConvertIntegral(LuaDll.lua_tointeger(luaState, index), underlyingType, out value);
            }

            double number = LuaDll.lua_tonumber(luaState, index);
            if (Math.Truncate(number) != number)
            {
                return false;
            }

            long truncated = (long)number;
            if ((double)truncated != number)
            {
                return false;
            }

            return TryConvertIntegral(truncated, underlyingType, out value);
        }

        private static bool TryConvertIntegral(long integral, Type underlyingType, out object value)
        {
            value = null;
            switch (Type.GetTypeCode(underlyingType))
            {
                case TypeCode.SByte:
                    if (integral < sbyte.MinValue || integral > sbyte.MaxValue)
                    {
                        return false;
                    }

                    value = (sbyte)integral;
                    return true;
                case TypeCode.Byte:
                    if (integral < byte.MinValue || integral > byte.MaxValue)
                    {
                        return false;
                    }

                    value = (byte)integral;
                    return true;
                case TypeCode.Int16:
                    if (integral < short.MinValue || integral > short.MaxValue)
                    {
                        return false;
                    }

                    value = (short)integral;
                    return true;
                case TypeCode.UInt16:
                    if (integral < ushort.MinValue || integral > ushort.MaxValue)
                    {
                        return false;
                    }

                    value = (ushort)integral;
                    return true;
                case TypeCode.Int32:
                    if (integral < int.MinValue || integral > int.MaxValue)
                    {
                        return false;
                    }

                    value = (int)integral;
                    return true;
                case TypeCode.UInt32:
                    if (integral < 0 || integral > uint.MaxValue)
                    {
                        return false;
                    }

                    value = (uint)integral;
                    return true;
                case TypeCode.Int64:
                    value = integral;
                    return true;
                case TypeCode.UInt64:
                    if (integral < 0)
                    {
                        return false;
                    }

                    value = (ulong)integral;
                    return true;
                default:
                    return false;
            }
        }
    }
}
