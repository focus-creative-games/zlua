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
    /// C# optional/default parameter support for Mono Lua→C# invoke (spec 04-METHOD-OVERLOAD §3.3).
    /// Defaults are cached at bind/compile time; call path only reads the cache.
    /// </summary>
    internal static class DefaultParameterUtil
    {
        private static readonly ConcurrentDictionary<MethodBase, object[]> s_cachedDefaults =
            new ConcurrentDictionary<MethodBase, object[]>();

        private static readonly ConcurrentDictionary<MethodBase, bool> s_hasOptionalDefaults =
            new ConcurrentDictionary<MethodBase, bool>();

        private static readonly ConcurrentDictionary<MethodBase, int> s_minLuaArity =
            new ConcurrentDictionary<MethodBase, int>();

        /// <summary>
        /// True when at least one trailing CLR parameter (after extension this) has a usable C# default.
        /// </summary>
        internal static bool HasOptionalDefaults(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            return s_hasOptionalDefaults.GetOrAdd(method, ComputeHasOptionalDefaults);
        }

        /// <summary>
        /// Minimum Lua stack slots when all trailing defaults are omitted.
        /// </summary>
        internal static int GetMinLuaArity(MethodBase method)
        {
            if (method == null)
            {
                return 0;
            }

            return s_minLuaArity.GetOrAdd(method, ComputeMinLuaArity);
        }

        /// <summary>
        /// Per CLR parameter: usable default value, or <see cref="Missing.Value"/> if not applicable.
        /// Built once per method.
        /// </summary>
        internal static object[] GetCachedDefaults(MethodBase method)
        {
            if (method == null)
            {
                return Array.Empty<object>();
            }

            return s_cachedDefaults.GetOrAdd(method, BuildCachedDefaults);
        }

        internal static bool TryGetDefaultValue(ParameterInfo parameter, out object value)
        {
            value = null;
            if (parameter == null)
            {
                return false;
            }

            if ((parameter.Attributes & ParameterAttributes.HasDefault) == 0)
            {
                return false;
            }

            try
            {
                value = parameter.DefaultValue;
            }
            catch (FormatException)
            {
                return false;
            }
            catch (InvalidOperationException)
            {
                return false;
            }

            if (value == DBNull.Value || value == Missing.Value)
            {
                value = null;
                return false;
            }

            return true;
        }

        private static bool ComputeHasOptionalDefaults(MethodBase method)
        {
            return GetMinLuaArity(method) < InterpretedMethodInvoker.GetLuaArity(method);
        }

        private static int ComputeMinLuaArity(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int paramStart = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            int maxSlots = InterpretedMethodInvoker.GetLuaArity(method);
            int omitSlots = 0;
            for (int i = parameters.Length - 1; i >= paramStart; i--)
            {
                if (!TryGetDefaultValue(parameters[i], out _))
                {
                    break;
                }

                LuaMarshalBinding binding = LuaMarshalAsValidation.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
                omitSlots += binding.StackSlots;
            }

            int min = maxSlots - omitSlots;
            return min < 0 ? 0 : min;
        }

        private static object[] BuildCachedDefaults(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            var defaults = new object[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                if (TryGetDefaultValue(parameters[i], out object value))
                {
                    defaults[i] = value;
                }
                else
                {
                    defaults[i] = Missing.Value;
                }
            }

            return defaults;
        }
    }
}
