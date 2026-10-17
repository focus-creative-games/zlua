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
    /// Interpreted Lua→C# method bridge for Table / UnpackedValues (stack-cursor + Σ stackSlots).
    /// </summary>
    internal static class InterpretedMethodInvoker
    {
        /// <summary>
        /// Lua→C# <c>out</c>: omitted from arity / Pop; pushed as multi-return after Invoke (spec 03-BYREF §5).
        /// </summary>
        internal static bool IsOutParameter(ParameterInfo parameter)
        {
            return parameter != null
                && parameter.ParameterType.IsByRef
                && parameter.IsOut;
        }

        internal static bool NeedsInterpreted(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (IsOutParameter(parameters[i]))
                {
                    return true;
                }

                LuaMarshalBinding binding = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
                if (binding.IsComposite)
                {
                    return true;
                }
            }

            if (method is MethodInfo methodInfo && methodInfo.ReturnType != typeof(void))
            {
                LuaMarshalBinding ret = LuaMarshalAsValidation.Current.ResolveReturnBinding(
                    methodInfo,
                    LuaMarshalDirection.CSharpToLua);
                if (ret.IsComposite || ret.MarshalType == LuaMarshalType.ActualTypeUserData)
                {
                    return true;
                }
            }

            return false;
        }

        internal static bool CanBind(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                if (paramType.IsByRef)
                {
                    // Out → interpreted multi-return. ref/in still rejected here until temp-slot path lands.
                    if (!IsOutParameter(parameters[i]))
                    {
                        return false;
                    }

                    paramType = paramType.GetElementType();
                }

                if (PointerMarshal.TryGetRejectMessage(paramType, out _))
                {
                    return false;
                }
            }

            if (method is MethodInfo mi
                && mi.ReturnType != typeof(void)
                && PointerMarshal.TryGetRejectMessage(mi.ReturnType, out _))
            {
                return false;
            }

            return NeedsInterpreted(method) || BridgeMarshaling.CanExpressionEmit(method);
        }

        /// <summary>
        /// Lua arity treating <c>params T[]</c> as one logical slot for min/default math
        /// (runtime expand uses <see cref="GetMaxLuaArity"/>). Extension methods omit CLR param 0 (this).
        /// <c>out</c> parameters contribute 0 slots (spec 03-BYREF §5 / 04-METHOD-OVERLOAD §3.3).
        /// </summary>
        internal static int GetLuaArity(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int start = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            int slots = 0;
            for (int i = start; i < parameters.Length; i++)
            {
                if (IsOutParameter(parameters[i]))
                {
                    continue;
                }

                LuaMarshalBinding binding = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
                slots += binding.StackSlots;
            }

            return slots;
        }

        /// <summary>
        /// Inclusive max Lua arg count for validate / GTAP. Unbounded when the method has params.
        /// </summary>
        internal static int GetMaxLuaArity(MethodBase method)
        {
            if (ParamsArrayUtil.HasParamsArray(method))
            {
                return int.MaxValue;
            }

            return GetLuaArity(method);
        }

        internal static Func<IntPtr, int> CompileMethod(MethodInfo method, bool isStatic, bool isByVal)
        {
            ParameterInfo[] parameters = method.GetParameters();
            var paramBindings = new LuaMarshalBinding[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                paramBindings[i] = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
            }

            LuaMarshalBinding returnBinding = method.ReturnType == typeof(void)
                ? LuaMarshalBinding.Default
                : LuaMarshalAsValidation.Current.ResolveReturnBinding(method, LuaMarshalDirection.CSharpToLua);

            bool isExtension = ExtensionMethodUtil.IsExtensionMethod(method);
            int argStart = isStatic ? 1 : 2;
            int maxArgs = GetMaxLuaArity(method);
            int minArgs = DefaultParameterUtil.GetMinLuaArity(method);
            object[] cachedDefaults = DefaultParameterUtil.HasOptionalDefaults(method)
                || ParamsArrayUtil.HasParamsArray(method)
                ? DefaultParameterUtil.GetCachedDefaults(method)
                : null;
            Type declaringType = method.DeclaringType;

            return L =>
            {
                BridgeMarshaling.ValidateArgCountRange(L, minArgs, maxArgs, argStart);

                if (isExtension)
                {
                    object[] invokeArgs = PopExtensionArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                    object returnValue = method.Invoke(null, invokeArgs);
                    return PushInvokeResults(L, method, parameters, returnValue, invokeArgs, returnBinding);
                }

                object target = null;
                if (!isStatic)
                {
                    if (isByVal && declaringType != null && declaringType.IsValueType)
                    {
                        var args = PopArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                        object result = ByValInstanceOps.InvokeInstance(L, 1, declaringType, method, args);
                        return PushInvokeResults(L, method, parameters, result, args, returnBinding);
                    }

                    target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal: false);
                }

                object[] invokeArgsNormal = PopArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                object returnValueNormal = method.Invoke(target, invokeArgsNormal);
                return PushInvokeResults(L, method, parameters, returnValueNormal, invokeArgsNormal, returnBinding);
            };
        }

        internal static Func<IntPtr, int> CompileConstructor(ConstructorInfo ctor, Type type)
        {
            ParameterInfo[] parameters = ctor.GetParameters();
            var paramBindings = new LuaMarshalBinding[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                paramBindings[i] = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    parameters[i],
                    ctor,
                    LuaMarshalDirection.LuaToCSharp);
            }

            const int argStart = 2;
            int maxArgs = GetMaxLuaArity(ctor);
            int minArgs = DefaultParameterUtil.GetMinLuaArity(ctor);
            object[] cachedDefaults = DefaultParameterUtil.HasOptionalDefaults(ctor)
                || ParamsArrayUtil.HasParamsArray(ctor)
                ? DefaultParameterUtil.GetCachedDefaults(ctor)
                : null;

            return L =>
            {
                BridgeMarshaling.ValidateArgCountRange(L, minArgs, maxArgs, argStart);
                object[] args = PopArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                object instance = ctor.Invoke(args);
                TypeRegistry.PushConstructorInstance(L, instance, type);
                return 1;
            };
        }

        private static object[] PopExtensionArgs(
            IntPtr L,
            int argStart,
            ParameterInfo[] parameters,
            LuaMarshalBinding[] bindings,
            object[] cachedDefaults)
        {
            var args = new object[parameters.Length];
            // Slot 1 = receiver → CLR param 0 (P0 type; ByObj or ByVal via CompositeMarshal).
            args[0] = CompositeMarshal.Pop(L, 1, parameters[0].ParameterType, bindings[0]);
            int slot = argStart;
            int top = LuaDll.lua_gettop(L);
            for (int i = 1; i < parameters.Length; i++)
            {
                args[i] = PopOne(L, ref slot, top, parameters, bindings, cachedDefaults, i);
            }

            return args;
        }

        private static object[] PopArgs(
            IntPtr L,
            int argStart,
            ParameterInfo[] parameters,
            LuaMarshalBinding[] bindings,
            object[] cachedDefaults)
        {
            var args = new object[parameters.Length];
            int slot = argStart;
            int top = LuaDll.lua_gettop(L);
            for (int i = 0; i < parameters.Length; i++)
            {
                args[i] = PopOne(L, ref slot, top, parameters, bindings, cachedDefaults, i);
            }

            return args;
        }

        private static object PopOne(
            IntPtr L,
            ref int slot,
            int top,
            ParameterInfo[] parameters,
            LuaMarshalBinding[] bindings,
            object[] cachedDefaults,
            int paramIndex)
        {
            ParameterInfo parameter = parameters[paramIndex];
            Type paramType = parameter.ParameterType;
            LuaMarshalBinding binding = bindings[paramIndex];

            // out: no Lua slot; Method.Invoke needs a placeholder (value-type boxed default / null).
            if (IsOutParameter(parameter))
            {
                Type elementType = paramType.GetElementType();
                return elementType.IsValueType ? Activator.CreateInstance(elementType) : null;
            }

            if (ParamsArrayUtil.IsParamsParameter(parameter))
            {
                return PopParamsArray(L, ref slot, top, paramType);
            }

            int need = binding.StackSlots > 0 ? binding.StackSlots : 1;
            int available = top >= slot ? top - slot + 1 : 0;
            if (available >= need)
            {
                object value = CompositeMarshal.Pop(L, slot, paramType, binding);
                slot += need;
                return value;
            }

            if (cachedDefaults != null
                && paramIndex < cachedDefaults.Length
                && !ReferenceEquals(cachedDefaults[paramIndex], Missing.Value))
            {
                return cachedDefaults[paramIndex];
            }

            LuaCallbackBoundary.Throw(
                $"zlua argument mismatch: missing argument for parameter {paramIndex}");
            return null;
        }

        /// <summary>
        /// Push CLR return then each <c>out</c> in declaration order (spec 03-BYREF §5).
        /// </summary>
        private static int PushInvokeResults(
            IntPtr L,
            MethodInfo method,
            ParameterInfo[] parameters,
            object returnValue,
            object[] invokeArgs,
            LuaMarshalBinding returnBinding)
        {
            int pushed = CompositeMarshal.Push(L, returnValue, method.ReturnType, returnBinding);
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!IsOutParameter(parameters[i]))
                {
                    continue;
                }

                Type elementType = parameters[i].ParameterType.GetElementType();
                LuaMarshalBinding outBinding = LuaMarshalAsValidation.Current.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.CSharpToLua,
                    elementType);
                pushed += CompositeMarshal.Push(L, invokeArgs[i], elementType, outBinding);
            }

            return pushed;
        }

        private static object PopParamsArray(
            IntPtr L,
            ref int slot,
            int top,
            Type arrayType)
        {
            int available = top >= slot ? top - slot + 1 : 0;
            Type elementType = arrayType.GetElementType();
            if (available == 0)
            {
                return Array.CreateInstance(elementType, 0);
            }

            // Single matching szarray userdata → pass through as whole T[] (not table / nil).
            if (available == 1 && ParamsArrayUtil.LooksLikeArrayUserDataArg(L, slot, arrayType))
            {
                Array existing = ArrayMarshal.PopSzArray(L, slot, arrayType);
                slot += 1;
                return existing;
            }

            // Expand: each Lua arg is one T element (nil → null element, not null T[]).
            Array packed = Array.CreateInstance(elementType, available);
            for (int i = 0; i < available; i++)
            {
                ArrayMarshal.SetElement1D(L, packed, i, slot, elementType);
                slot++;
            }

            return packed;
        }
    }
}
