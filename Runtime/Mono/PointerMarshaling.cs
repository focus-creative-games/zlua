using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Non-managed pointers, function pointers, and related opaque address tokens (lightuserdata).
    /// </summary>
    internal static partial class PointerMarshaling
    {
        internal static bool IsFunctionPointerType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            PropertyInfo property = typeof(Type).GetProperty("IsFunctionPointer", BindingFlags.Public | BindingFlags.Instance);
            if (property?.PropertyType == typeof(bool) && (bool)property.GetValue(type))
            {
                return true;
            }

            // Unity Mono may lack IsFunctionPointer; delegate* still reports as MONO_TYPE_FNPTR (0x1b).
            string text = type.ToString();
            return text != null && text.IndexOf("delegate*", StringComparison.Ordinal) >= 0;
        }

        internal static bool IsTypedReference(Type type)
        {
            return type == typeof(TypedReference);
        }

        internal static bool IsPointerLikeType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            if (type.IsPointer || IsFunctionPointerType(type))
            {
                return true;
            }

            // Unity Mono may not set IsPointer for int* / void* in method signatures.
            string name = type.Name;
            return !string.IsNullOrEmpty(name)
                && name.EndsWith("*", StringComparison.Ordinal)
                && name.IndexOf("delegate*", StringComparison.Ordinal) < 0;
        }

        internal static bool IsUnsupportedMarshalType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            if (IsTypedReference(type))
            {
                return true;
            }

            if (type == typeof(decimal))
            {
                return true;
            }

            return IsByRefLikeType(type);
        }

        internal static bool IsByRefLikeType(Type type)
        {
            if (type == null || !type.IsValueType)
            {
                return false;
            }

            PropertyInfo property = typeof(Type).GetProperty("IsByRefLike", BindingFlags.Public | BindingFlags.Instance);
            if (property?.PropertyType == typeof(bool))
            {
                return (bool)property.GetValue(type);
            }

            string name = type.FullName ?? type.ToString();
            if (string.IsNullOrEmpty(name))
            {
                return false;
            }

            return name.StartsWith("System.Span`", StringComparison.Ordinal)
                || name.StartsWith("System.ReadOnlySpan`", StringComparison.Ordinal);
        }

        /// <summary>
        /// Mono <see cref="MethodInfo.Invoke"/> cannot marshal these signatures (FNPTR=0x1b, etc.).
        /// </summary>
        internal static bool IsReflectionInvokeUnsafeType(Type type)
        {
            return IsPointerLikeType(type) || IsUnsupportedMarshalType(type);
        }

        internal static void PushPointer(IntPtr luaState, IntPtr address)
        {
            LuaDll.lua_pushlightuserdata(luaState, address);
        }

        internal static IntPtr ReadPointer(IntPtr luaState, int index)
        {
            if (LuaDll.lua_type(luaState, index) != LuaDataType.LightUserData)
            {
                throw new NotSupportedException("pointer value must be lightuserdata");
            }

            return LuaDll.lua_touserdata(luaState, index);
        }

        internal static bool CanConvertPointerValue(IntPtr luaState, int index)
        {
            LuaDataType luaType = LuaDll.lua_type(luaState, index);
            return luaType == LuaDataType.LightUserData || luaType == LuaDataType.Nil;
        }

        internal static IntPtr CoerceToAddress(object value)
        {
            if (value == null)
            {
                return IntPtr.Zero;
            }

            if (value is IntPtr intPtr)
            {
                return intPtr;
            }

            if (value is UIntPtr uintPtr)
            {
                return (IntPtr)(long)(ulong)uintPtr;
            }

            throw new NotSupportedException($"unsupported pointer value type {value.GetType().FullName}");
        }

        internal static object BoxPointerForInvoke(IntPtr address, Type pointerType)
        {
            if (address == IntPtr.Zero)
            {
                return null;
            }

            return address;
        }
    }
}
