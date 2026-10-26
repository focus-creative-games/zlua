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
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// Bind-time snapshot for overload scoring. Call path must not call GetParameters /
    /// ResolveParameterBinding again.
    /// </summary>
    internal sealed class LuaArgMatchCandidate
    {
        private static readonly ConcurrentDictionary<MethodBase, LuaArgMatchCandidate> s_cache =
            new ConcurrentDictionary<MethodBase, LuaArgMatchCandidate>();

        internal readonly MethodBase Method;
        internal readonly ParameterInfo[] Parameters;
        /// <summary>Parallel to <see cref="Parameters"/>; null for out parameters (0 Lua slots).</summary>
        internal readonly LuaMarshalBinding[] Bindings;
        internal readonly Type[] ParameterTypes;
        internal readonly bool[] IsOut;
        internal readonly bool[] IsParams;
        internal readonly bool[] HasDefault;
        internal readonly int ParamStart;
        internal readonly int ParamsKind;

        private LuaArgMatchCandidate(MethodBase method)
        {
            Method = method;
            Parameters = method.GetParameters();
            ParamStart = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            ParamsKind = ParamsArrayUtil.HasParamsArray(method) ? 1 : 0;

            int n = Parameters.Length;
            Bindings = new LuaMarshalBinding[n];
            ParameterTypes = new Type[n];
            IsOut = new bool[n];
            IsParams = new bool[n];
            HasDefault = new bool[n];

            for (int i = 0; i < n; i++)
            {
                ParameterInfo p = Parameters[i];
                ParameterTypes[i] = p.ParameterType;
                IsOut[i] = InterpretedMethodInvoker.IsOutParameter(p);
                IsParams[i] = ParamsArrayUtil.IsParamsParameter(p);
                HasDefault[i] = DefaultParameterUtil.TryGetDefaultValue(p, out _);
                if (IsOut[i])
                {
                    continue;
                }

                Bindings[i] = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    p,
                    method,
                    LuaMarshalDirection.LuaToCSharp);
            }
        }

        /// <summary>
        /// Shared cache for ad-hoc single-candidate scoring (e.g. GTAP closed methods).
        /// Overload groups should prefer <see cref="Create"/> once and close over the instance.
        /// </summary>
        internal static LuaArgMatchCandidate GetOrCreate(MethodBase method)
        {
            if (method == null)
            {
                throw new ArgumentNullException(nameof(method));
            }

            return s_cache.GetOrAdd(method, m => new LuaArgMatchCandidate(m));
        }

        /// <summary>
        /// Build without inserting into the shared cache (overload group owns the instance).
        /// </summary>
        internal static LuaArgMatchCandidate Create(MethodBase method)
        {
            if (method == null)
            {
                throw new ArgumentNullException(nameof(method));
            }

            return new LuaArgMatchCandidate(method);
        }

        internal static LuaArgMatchCandidate[] CreateAll(MethodBase[] methods)
        {
            var result = new LuaArgMatchCandidate[methods.Length];
            for (int i = 0; i < methods.Length; i++)
            {
                result[i] = Create(methods[i]);
            }

            return result;
        }
    }
}
