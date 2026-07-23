using System;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Unmanaged pointers / function pointers as lightuserdata; TypedReference / ByRefLike / decimal rejection.
    /// </summary>
    internal static class PointerMarshal
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

        internal static bool IsUnsupportedMarshalType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            if (IsTypedReference(type) || type == typeof(decimal) || IsByRefLikeType(type))
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Call-time reject message for methods whose signatures cannot be marshaled (Il2Cpp-aligned).
        /// </summary>
        internal static bool TryGetRejectMessage(Type type, out string message)
        {
            if (IsTypedReference(type))
            {
                message = "zlua: can't marshal typed reference type";
                return true;
            }

            if (type == typeof(decimal) || IsByRefLikeType(type))
            {
                message = "zlua argument mismatch";
                return true;
            }

            message = null;
            return false;
        }

        internal static bool TryGetMethodRejectMessage(MethodInfo method, out string message)
        {
            if (method == null)
            {
                message = null;
                return false;
            }

            if (method.ReturnType != typeof(void) && TryGetRejectMessage(method.ReturnType, out message))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (TryGetRejectMessage(parameters[i].ParameterType, out message))
                {
                    return true;
                }
            }

            message = null;
            return false;
        }

        internal static bool MethodRequiresPointerInvoke(MethodInfo method)
        {
            if (method == null)
            {
                return false;
            }

            if (IsPointerLikeType(method.ReturnType))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (IsPointerLikeType(parameters[i].ParameterType))
                {
                    return true;
                }
            }

            return false;
        }

        internal static void PushPointer(IntPtr L, IntPtr address)
        {
            if (address == IntPtr.Zero)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            LuaDll.lua_pushlightuserdata(L, address);
        }

        internal static IntPtr PopPointer(IntPtr L, int index)
        {
            LuaDataType luaType = LuaDll.lua_type(L, index);
            if (luaType == LuaDataType.LightUserData)
            {
                return LuaDll.lua_touserdata(L, index);
            }

            if (luaType == LuaDataType.Nil)
            {
                return IntPtr.Zero;
            }

            LuaCallbackBoundary.Throw($"zlua: expected userdata or nil at index {index}");
            return IntPtr.Zero;
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
    }
}
