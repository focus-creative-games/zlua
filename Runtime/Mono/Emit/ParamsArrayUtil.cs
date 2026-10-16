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
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// C# <c>params T[]</c> detection. Trailing Lua args expand into elements;
    /// a single matching szarray userdata may bind as the whole array.
    /// Table / nil are never treated as a whole <c>T[]</c> (nil is a null element).
    /// </summary>
    internal static class ParamsArrayUtil
    {
        /// <summary>
        /// Overload arity buckets register params candidates up to this Lua arg count;
        /// higher counts fall through to a params-only overflow core.
        /// </summary>
        internal const int MaxExpandArityBucket = 16;

        internal static bool IsParamsParameter(ParameterInfo parameter)
        {
            return parameter != null
                && parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false);
        }

        internal static bool TryGetParamsParameterIndex(MethodBase method, out int index)
        {
            index = -1;
            if (method == null)
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            int start = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            if (parameters.Length <= start)
            {
                return false;
            }

            int last = parameters.Length - 1;
            if (!IsParamsParameter(parameters[last]))
            {
                return false;
            }

            index = last;
            return true;
        }

        internal static bool HasParamsArray(MethodBase method)
        {
            return TryGetParamsParameterIndex(method, out _);
        }

        /// <summary>
        /// True when a single Lua value is an existing <paramref name="arrayType"/> userdata
        /// (pass-through whole array). Not table, not nil.
        /// </summary>
        internal static bool LooksLikeArrayUserDataArg(IntPtr L, int index, Type arrayType)
        {
            if (arrayType == null || !arrayType.IsArray || arrayType.GetArrayRank() != 1)
            {
                return false;
            }

            if (LuaDll.lua_type(L, index) != LuaDataType.UserData)
            {
                return false;
            }

            return ObjectRegistry.TryGetObject(L, index, out object obj)
                && obj != null
                && arrayType.IsInstanceOfType(obj);
        }
    }
}
