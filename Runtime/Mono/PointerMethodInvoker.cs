using System;
using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;

namespace ZLua
{
    /// <summary>
    /// Mono <see cref="MethodInfo.Invoke"/> cannot marshal pointer / function-pointer signatures; emit call stubs instead.
    /// </summary>
    internal static class PointerMethodInvoker
    {
        private static readonly Dictionary<MethodBase, Func<object, object[], object>> Invokers =
            new Dictionary<MethodBase, Func<object, object[], object>>();

        private static readonly MethodInfo PointerArgToNativeIntMethod =
            typeof(PointerMethodInvoker).GetMethod(nameof(PointerArgToNativeInt), BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo BoxNativePointerReturnMethod =
            typeof(PointerMethodInvoker).GetMethod(nameof(BoxNativePointerReturn), BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool RequiresPointerInvoke(MethodBase method)
        {
            if (method is MethodInfo methodInfo && PointerMarshaling.IsPointerLikeType(methodInfo.ReturnType))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (PointerMarshaling.IsPointerLikeType(parameters[i].ParameterType))
                {
                    return true;
                }
            }

            return PointerMarshaling.MethodSignatureContainsUnmanagedPointer(method);
        }

        internal static bool HasUnsafeReflectionSignature(MethodBase method)
        {
            if (method is MethodInfo methodInfo && PointerMarshaling.IsReflectionInvokeUnsafeType(methodInfo.ReturnType))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (PointerMarshaling.IsReflectionInvokeUnsafeType(parameters[i].ParameterType))
                {
                    return true;
                }
            }

            return false;
        }

        internal static object Invoke(MethodInfo method, object target, object[] args)
        {
            Func<object, object[], object> invoker;
            lock (Invokers)
            {
                if (!Invokers.TryGetValue(method, out invoker))
                {
                    invoker = CreateInvoker(method);
                    Invokers[method] = invoker;
                }
            }

            return invoker(target, args);
        }

        private static long PointerArgToNativeInt(object value)
        {
            return PointerMarshaling.CoerceToAddress(value).ToInt64();
        }

        private static object BoxNativePointerReturn(long nativeAddress)
        {
            if (nativeAddress == 0)
            {
                return null;
            }

            return new IntPtr(nativeAddress);
        }

        private static Func<object, object[], object> CreateInvoker(MethodInfo method)
        {
            if (PointerArgToNativeIntMethod == null || BoxNativePointerReturnMethod == null)
            {
                throw new NotSupportedException("pointer invoke helpers are unavailable.");
            }

            DynamicMethod dynamicMethod = new DynamicMethod(
                $"zlua_ptr_invoke_{method.DeclaringType?.Name}_{method.Name}",
                typeof(object),
                new[] { typeof(object), typeof(object[]) },
                true);

            ILGenerator il = dynamicMethod.GetILGenerator();
            ParameterInfo[] parameters = method.GetParameters();

            if (!method.IsStatic)
            {
                EmitLoadTarget(il, method.DeclaringType);
            }

            for (int i = 0; i < parameters.Length; i++)
            {
                EmitLoadArgumentFromObjectArray(il, parameters[i].ParameterType, i);
            }

            il.Emit(method.IsStatic ? OpCodes.Call : OpCodes.Callvirt, method);
            EmitBoxReturnValue(il, method.ReturnType);
            il.Emit(OpCodes.Ret);

            return (Func<object, object[], object>)dynamicMethod.CreateDelegate(typeof(Func<object, object[], object>));
        }

        private static void EmitLoadTarget(ILGenerator il, Type declaringType)
        {
            il.Emit(OpCodes.Ldarg_0);
            if (declaringType.IsValueType)
            {
                il.Emit(OpCodes.Unbox_Any, declaringType);
            }
            else
            {
                il.Emit(OpCodes.Castclass, declaringType);
            }
        }

        private static void EmitLoadArgumentFromObjectArray(ILGenerator il, Type parameterType, int index)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;
            if (targetType.IsByRef)
            {
                throw new NotSupportedException($"pointer invoke does not support by-ref parameter {parameterType.FullName}");
            }

            if (PointerMarshaling.IsPointerLikeType(targetType))
            {
                EmitLoadPointerArgument(il, index);
                return;
            }

            if (targetType.IsEnum)
            {
                EmitLoadObjectArrayElement(il, index);
                EmitUnboxOrCastEnum(il, targetType);
                return;
            }

            if (targetType.IsValueType)
            {
                EmitLoadObjectArrayElement(il, index);
                il.Emit(OpCodes.Unbox_Any, targetType);
                return;
            }

            EmitLoadReferenceArgument(il, targetType, index);
        }

        private static void EmitLoadObjectArrayElement(ILGenerator il, int index)
        {
            il.Emit(OpCodes.Ldarg_1);
            il.Emit(OpCodes.Ldc_I4, index);
            il.Emit(OpCodes.Ldelem_Ref);
        }

        private static void EmitLoadPointerArgument(ILGenerator il, int index)
        {
            EmitLoadObjectArrayElement(il, index);
            il.Emit(OpCodes.Call, PointerArgToNativeIntMethod);
            il.Emit(OpCodes.Conv_I);
        }

        private static void EmitLoadReferenceArgument(ILGenerator il, Type referenceType, int index)
        {
            Label hasValue = il.DefineLabel();
            Label done = il.DefineLabel();

            EmitLoadObjectArrayElement(il, index);
            il.Emit(OpCodes.Dup);
            il.Emit(OpCodes.Brtrue_S, hasValue);
            il.Emit(OpCodes.Pop);
            il.Emit(OpCodes.Ldnull);
            il.Emit(OpCodes.Br_S, done);
            il.MarkLabel(hasValue);
            il.Emit(OpCodes.Castclass, referenceType);
            il.MarkLabel(done);
        }

        private static void EmitUnboxOrCastEnum(ILGenerator il, Type enumType)
        {
            il.Emit(OpCodes.Unbox_Any, enumType);
        }

        private static void EmitBoxReturnValue(ILGenerator il, Type returnType)
        {
            if (returnType == typeof(void))
            {
                il.Emit(OpCodes.Ldnull);
                return;
            }

            if (PointerMarshaling.IsPointerLikeType(returnType))
            {
                if (IntPtr.Size == 8)
                {
                    il.Emit(OpCodes.Conv_I8);
                }
                else
                {
                    il.Emit(OpCodes.Conv_I4);
                    il.Emit(OpCodes.Conv_I8);
                }

                il.Emit(OpCodes.Call, BoxNativePointerReturnMethod);
                return;
            }

            if (returnType.IsEnum)
            {
                Type underlyingType = Enum.GetUnderlyingType(returnType);
                il.Emit(OpCodes.Box, underlyingType);
                return;
            }

            if (returnType.IsValueType)
            {
                il.Emit(OpCodes.Box, returnType);
                return;
            }

            il.Emit(OpCodes.Castclass, typeof(object));
        }
    }
}
