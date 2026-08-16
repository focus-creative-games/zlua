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
using System.Collections.Concurrent;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Unmanaged pointers / function pointers as lightuserdata; TypedReference / ByRefLike / decimal rejection.
    /// </summary>
    internal static class PointerMarshal
    {
        private static readonly PropertyInfo s_isFunctionPointerProp =
            typeof(Type).GetProperty("IsFunctionPointer", BindingFlags.Public | BindingFlags.Instance);

        private static readonly PropertyInfo s_isByRefLikeProp =
            typeof(Type).GetProperty("IsByRefLike", BindingFlags.Public | BindingFlags.Instance);

        private static readonly ConcurrentDictionary<Type, bool> s_isFunctionPointerByType =
            new ConcurrentDictionary<Type, bool>();

        private static readonly ConcurrentDictionary<Type, bool> s_isByRefLikeByType =
            new ConcurrentDictionary<Type, bool>();

        internal static bool IsFunctionPointerType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            return s_isFunctionPointerByType.GetOrAdd(type, ComputeIsFunctionPointer);
        }

        private static bool ComputeIsFunctionPointer(Type type)
        {
            if (s_isFunctionPointerProp?.PropertyType == typeof(bool)
                && (bool)s_isFunctionPointerProp.GetValue(type))
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

            return s_isByRefLikeByType.GetOrAdd(type, ComputeIsByRefLike);
        }

        private static bool ComputeIsByRefLike(Type type)
        {
            if (s_isByRefLikeProp?.PropertyType == typeof(bool))
            {
                return (bool)s_isByRefLikeProp.GetValue(type);
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
