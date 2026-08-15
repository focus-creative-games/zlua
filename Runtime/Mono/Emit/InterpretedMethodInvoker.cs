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
        internal static bool NeedsInterpreted(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                LuaMarshalBinding binding = LuaMarshalAsValidation.ResolveParameterBinding(
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
                LuaMarshalBinding ret = LuaMarshalAsValidation.ResolveReturnBinding(
                    methodInfo,
                    LuaMarshalDirection.CSharpToLua);
                if (ret.IsComposite)
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
                if (parameters[i].ParameterType.IsByRef)
                {
                    return false;
                }

                if (PointerMarshal.TryGetRejectMessage(parameters[i].ParameterType, out _))
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
        /// Lua arity for matching / exact-count. Extension methods omit CLR param 0 (this).
        /// </summary>
        internal static int GetLuaArity(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int start = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            int slots = 0;
            for (int i = start; i < parameters.Length; i++)
            {
                LuaMarshalBinding binding = LuaMarshalAsValidation.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
                slots += binding.StackSlots;
            }

            return slots;
        }

        internal static Func<IntPtr, int> CompileMethod(MethodInfo method, bool isStatic, bool isByVal)
        {
            ParameterInfo[] parameters = method.GetParameters();
            var paramBindings = new LuaMarshalBinding[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                paramBindings[i] = LuaMarshalAsValidation.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
            }

            LuaMarshalBinding returnBinding = method.ReturnType == typeof(void)
                ? LuaMarshalBinding.Default
                : LuaMarshalAsValidation.ResolveReturnBinding(method, LuaMarshalDirection.CSharpToLua);

            bool isExtension = ExtensionMethodUtil.IsExtensionMethod(method);
            int argStart = isStatic ? 1 : 2;
            int maxArgs = GetLuaArity(method);
            int minArgs = DefaultParameterUtil.GetMinLuaArity(method);
            object[] cachedDefaults = DefaultParameterUtil.HasOptionalDefaults(method)
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
                    return CompositeMarshal.Push(L, returnValue, method.ReturnType, returnBinding);
                }

                object target = null;
                if (!isStatic)
                {
                    if (isByVal && declaringType != null && declaringType.IsValueType)
                    {
                        var args = PopArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                        object result = ByValInstanceOps.InvokeInstance(L, 1, declaringType, method, args);
                        return CompositeMarshal.Push(L, result, method.ReturnType, returnBinding);
                    }

                    target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal: false);
                }

                object[] invokeArgsNormal = PopArgs(L, argStart, parameters, paramBindings, cachedDefaults);
                object returnValueNormal = method.Invoke(target, invokeArgsNormal);
                return CompositeMarshal.Push(L, returnValueNormal, method.ReturnType, returnBinding);
            };
        }

        internal static Func<IntPtr, int> CompileConstructor(ConstructorInfo ctor, Type type)
        {
            ParameterInfo[] parameters = ctor.GetParameters();
            var paramBindings = new LuaMarshalBinding[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                paramBindings[i] = LuaMarshalAsValidation.ResolveParameterBinding(
                    parameters[i],
                    ctor,
                    LuaMarshalDirection.LuaToCSharp);
            }

            const int argStart = 2;
            int maxArgs = GetLuaArity(ctor);
            int minArgs = DefaultParameterUtil.GetMinLuaArity(ctor);
            object[] cachedDefaults = DefaultParameterUtil.HasOptionalDefaults(ctor)
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
                args[i] = PopOne(L, ref slot, top, parameters[i].ParameterType, bindings[i], cachedDefaults, i);
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
                args[i] = PopOne(L, ref slot, top, parameters[i].ParameterType, bindings[i], cachedDefaults, i);
            }

            return args;
        }

        private static object PopOne(
            IntPtr L,
            ref int slot,
            int top,
            Type paramType,
            LuaMarshalBinding binding,
            object[] cachedDefaults,
            int paramIndex)
        {
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
    }
}
