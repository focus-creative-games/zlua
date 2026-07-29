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

        internal static int GetLuaArity(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int slots = 0;
            for (int i = 0; i < parameters.Length; i++)
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

            int argStart = isStatic ? 1 : 2;
            int expectedArgs = GetLuaArity(method);
            Type declaringType = method.DeclaringType;

            return L =>
            {
                BridgeMarshaling.ValidateExactArgCount(L, expectedArgs, argStart);

                object target = null;
                if (!isStatic)
                {
                    if (isByVal && declaringType != null && declaringType.IsValueType)
                    {
                        var args = PopArgs(L, argStart, parameters, paramBindings);
                        object result = ByValInstanceOps.InvokeInstance(L, 1, declaringType, method, args);
                        return CompositeMarshal.Push(L, result, method.ReturnType, returnBinding);
                    }

                    target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal: false);
                }

                object[] invokeArgs = PopArgs(L, argStart, parameters, paramBindings);
                object returnValue = method.Invoke(target, invokeArgs);
                return CompositeMarshal.Push(L, returnValue, method.ReturnType, returnBinding);
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
            int expectedArgs = GetLuaArity(ctor);

            return L =>
            {
                BridgeMarshaling.ValidateExactArgCount(L, expectedArgs, argStart);
                object[] args = PopArgs(L, argStart, parameters, paramBindings);
                object instance = ctor.Invoke(args);
                TypeRegistry.PushConstructorInstance(L, instance, type);
                return 1;
            };
        }

        private static object[] PopArgs(
            IntPtr L,
            int argStart,
            ParameterInfo[] parameters,
            LuaMarshalBinding[] bindings)
        {
            var args = new object[parameters.Length];
            int slot = argStart;
            for (int i = 0; i < parameters.Length; i++)
            {
                Type paramType = parameters[i].ParameterType;
                LuaMarshalBinding binding = bindings[i];
                args[i] = CompositeMarshal.Pop(L, slot, paramType, binding);
                slot += binding.StackSlots;
            }

            return args;
        }
    }
}
