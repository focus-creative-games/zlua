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
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// Typed pop/push helpers used by Expression-compiled bridges (no MethodMarshalCtx upvalues).
    /// </summary>
    internal static class BridgeMarshaling
    {
        internal static bool IsSupportedType(Type type)
        {
            if (type == null)
            {
                return false;
            }

            if (type.IsByRef || type == typeof(TypedReference))
            {
                return false;
            }

            if (PointerMarshal.IsUnsupportedMarshalType(type))
            {
                return false;
            }

            if (type.IsGenericTypeDefinition || type.ContainsGenericParameters)
            {
                return false;
            }

            // Type-level Table/Unpacked is supported via InterpretedMethodInvoker, not Expression PopArg.
            if (type.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                LuaMarshalAsAttribute attr = type.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
                if (attr != null
                    && attr.LuaMarshalType != LuaMarshalType.Default
                    && attr.LuaMarshalType != LuaMarshalType.Table
                    && attr.LuaMarshalType != LuaMarshalType.UnpackedValues)
                {
                    return false;
                }
            }

            return true;
        }

        internal static bool IsSupportedParameter(ParameterInfo parameter)
        {
            if (parameter == null)
            {
                return false;
            }

            Type type = parameter.ParameterType;
            if (type.IsByRef)
            {
                return false;
            }

            // params T[]: treat as a single array argument (table or array userdata).
            if (parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false))
            {
                return type.IsArray && type.GetArrayRank() == 1 && IsSupportedType(type.GetElementType());
            }

            if (parameter.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                LuaMarshalAsAttribute attr = parameter.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
                if (attr != null && attr.LuaMarshalType != LuaMarshalType.Default)
                {
                    // Table/Unpacked handled by interpreted path; other non-Default still block Expression.
                    if (attr.LuaMarshalType == LuaMarshalType.Table
                        || attr.LuaMarshalType == LuaMarshalType.UnpackedValues)
                    {
                        return IsSupportedType(type);
                    }

                    return false;
                }
            }

            return IsSupportedType(type);
        }

        /// <summary>
        /// True when the closed method can use the Expression fast path (no composite marshal).
        /// </summary>
        internal static bool CanExpressionEmit(MethodBase method)
        {
            if (method == null || InterpretedMethodInvoker.NeedsInterpreted(method))
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!IsSupportedParameter(parameters[i]))
                {
                    return false;
                }
            }

            if (method is MethodInfo methodInfo
                && methodInfo.ReturnType != typeof(void)
                && !IsSupportedType(methodInfo.ReturnType))
            {
                return false;
            }

            return true;
        }

        /// <summary>
        /// ByObj instance target only. ByVal field/method/property use <see cref="ByValInstanceOps"/>.
        /// </summary>
        internal static object PopTarget(IntPtr L, int index, Type declaringType, bool isByVal)
        {
            if (isByVal)
            {
                LuaCallbackBoundary.Throw("zlua internal error: ByVal target must use ByValInstanceOps");
            }

            return InstanceTarget.PopByObjThisAs(L, index, declaringType);
        }

        internal static object PopArg(IntPtr L, int index, Type declaredType)
        {
            return TypedMarshal.PopObject(L, index, declaredType);
        }

        internal static int PushReturn(IntPtr L, Type declaredType, object value)
        {
            if (declaredType == typeof(void))
            {
                return 0;
            }

            TypedMarshal.PushObject(L, value, declaredType);
            return 1;
        }

        internal static void ValidateExactArgCount(IntPtr L, int expectedArgs, int argStart)
        {
            ValidateArgCountRange(L, expectedArgs, expectedArgs, argStart);
        }

        /// <summary>
        /// Accepts Lua arg count in [minArgs, maxArgs] (inclusive). Used for C# default parameters.
        /// </summary>
        internal static void ValidateArgCountRange(IntPtr L, int minArgs, int maxArgs, int argStart)
        {
            int top = LuaDll.lua_gettop(L);
            int actual = top - argStart + 1;
            if (actual < 0)
            {
                actual = 0;
            }

            if (actual < minArgs || actual > maxArgs)
            {
                if (minArgs == maxArgs)
                {
                    LuaCallbackBoundary.Throw($"zlua argument mismatch: expected {minArgs} argument(s), got {actual}");
                }

                LuaCallbackBoundary.Throw(
                    $"zlua argument mismatch: expected {minArgs}..{maxArgs} argument(s), got {actual}");
            }
        }
    }
}
