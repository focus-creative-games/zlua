using System;
using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using Unity.Collections.LowLevel.Unsafe;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// In-place ByVal field/method ops: blittable payload or non-blittable companion unbox (no WriteBack).
    /// </summary>
    internal static class ByValInstanceOps
    {
        private static readonly Dictionary<MethodInfo, Func<IntPtr, int, object[], object>> s_blittableInvokers =
            new Dictionary<MethodInfo, Func<IntPtr, int, object[], object>>();

        private static readonly MethodInfo AsRefOpen =
            typeof(StructMarshal).GetMethod(
                nameof(StructMarshal.AsRef),
                BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic)
            ?? throw new InvalidOperationException("zlua: StructMarshal.AsRef not found");

        private static readonly MethodInfo PointerArgToNativeIntMethod =
            typeof(PointerMethodInvoker).GetMethod(
                "PointerArgToNativeInt",
                BindingFlags.NonPublic | BindingFlags.Static)
            ?? throw new InvalidOperationException("zlua: PointerArgToNativeInt not found");

        private static readonly MethodInfo BoxNativePointerReturnMethod =
            typeof(PointerMethodInvoker).GetMethod(
                "BoxNativePointerReturn",
                BindingFlags.NonPublic | BindingFlags.Static)
            ?? throw new InvalidOperationException("zlua: BoxNativePointerReturn not found");

        internal static object GetField(IntPtr L, int index, Type structType, FieldInfo field)
        {
            if (!StructMarshal.IsBlittable(structType))
            {
                return field.GetValue(StructMarshal.GetCompanionBoxed(L, index, structType));
            }

            unsafe
            {
                IntPtr data = StructMarshal.GetDataPointer(L, index, structType);
                IntPtr fieldPtr = (IntPtr)((byte*)data + UnsafeUtility.GetFieldOffset(field));
                Type fieldType = field.FieldType;
                if (fieldType.IsEnum)
                {
                    Type underlying = Enum.GetUnderlyingType(fieldType);
                    object raw = Marshal.PtrToStructure(fieldPtr, underlying);
                    return Enum.ToObject(fieldType, raw);
                }

                return Marshal.PtrToStructure(fieldPtr, fieldType);
            }
        }

        internal static void SetField(IntPtr L, int index, Type structType, FieldInfo field, object value)
        {
            if (!StructMarshal.IsBlittable(structType))
            {
                field.SetValue(StructMarshal.GetCompanionBoxed(L, index, structType), value);
                return;
            }

            unsafe
            {
                IntPtr data = StructMarshal.GetDataPointer(L, index, structType);
                IntPtr fieldPtr = (IntPtr)((byte*)data + UnsafeUtility.GetFieldOffset(field));
                Type fieldType = field.FieldType;
                if (value == null)
                {
                    value = Activator.CreateInstance(fieldType);
                }
                else if (fieldType.IsEnum)
                {
                    value = Enum.ToObject(fieldType, value);
                    value = Convert.ChangeType(value, Enum.GetUnderlyingType(fieldType));
                    Marshal.StructureToPtr(value, fieldPtr, false);
                    return;
                }
                else if (value.GetType() != fieldType)
                {
                    value = Convert.ChangeType(value, fieldType);
                }

                Marshal.StructureToPtr(value, fieldPtr, false);
            }
        }

        internal static object InvokeInstance(IntPtr L, int index, Type structType, MethodInfo method, object[] args)
        {
            if (args == null)
            {
                args = Array.Empty<object>();
            }

            if (!StructMarshal.IsBlittable(structType))
            {
                object companion = StructMarshal.GetCompanionBoxed(L, index, structType);
                if (PointerMarshal.MethodRequiresPointerInvoke(method))
                {
                    return PointerMethodInvoker.Invoke(method, companion, args);
                }

                return method.Invoke(companion, args);
            }

            Func<IntPtr, int, object[], object> invoker;
            lock (s_blittableInvokers)
            {
                if (!s_blittableInvokers.TryGetValue(method, out invoker))
                {
                    invoker = CreateBlittableInvoker(method, structType);
                    s_blittableInvokers[method] = invoker;
                }
            }

            return invoker(L, index, args);
        }

        internal static object InvokePropertyGetter(IntPtr L, int index, Type structType, MethodInfo getter)
        {
            return InvokeInstance(L, index, structType, getter, Array.Empty<object>());
        }

        internal static void InvokePropertySetter(IntPtr L, int index, Type structType, MethodInfo setter, object value)
        {
            InvokeInstance(L, index, structType, setter, new[] { value });
        }

        private static Func<IntPtr, int, object[], object> CreateBlittableInvoker(MethodInfo method, Type structType)
        {
            DynamicMethod dm = new DynamicMethod(
                $"zlua_byval_{structType.Name}_{method.Name}",
                typeof(object),
                new[] { typeof(IntPtr), typeof(int), typeof(object[]) },
                typeof(ByValInstanceOps).Module,
                skipVisibility: true);

            ILGenerator il = dm.GetILGenerator();
            MethodInfo asRef = AsRefOpen.MakeGenericMethod(structType);

            // this = ref StructMarshal.AsRef<T>(L, index)
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldarg_1);
            il.Emit(OpCodes.Call, asRef);

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                EmitLoadArg(il, parameters[i].ParameterType, i);
            }

            il.Emit(OpCodes.Call, method);
            EmitBoxReturn(il, method.ReturnType);
            il.Emit(OpCodes.Ret);

            return (Func<IntPtr, int, object[], object>)dm.CreateDelegate(
                typeof(Func<IntPtr, int, object[], object>));
        }

        private static void EmitLoadArg(ILGenerator il, Type parameterType, int index)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;
            if (targetType.IsByRef)
            {
                throw new NotSupportedException($"zlua byval invoke: by-ref parameter {parameterType.FullName}");
            }

            il.Emit(OpCodes.Ldarg_2);
            il.Emit(OpCodes.Ldc_I4, index);
            il.Emit(OpCodes.Ldelem_Ref);

            if (PointerMarshal.IsPointerLikeType(targetType))
            {
                il.Emit(OpCodes.Call, PointerArgToNativeIntMethod);
                il.Emit(OpCodes.Conv_I);
                return;
            }

            if (targetType.IsEnum || targetType.IsValueType)
            {
                il.Emit(OpCodes.Unbox_Any, targetType);
                return;
            }

            Label hasValue = il.DefineLabel();
            Label done = il.DefineLabel();
            il.Emit(OpCodes.Dup);
            il.Emit(OpCodes.Brtrue_S, hasValue);
            il.Emit(OpCodes.Pop);
            il.Emit(OpCodes.Ldnull);
            il.Emit(OpCodes.Br_S, done);
            il.MarkLabel(hasValue);
            il.Emit(OpCodes.Castclass, targetType);
            il.MarkLabel(done);
        }

        private static void EmitBoxReturn(ILGenerator il, Type returnType)
        {
            if (returnType == typeof(void))
            {
                il.Emit(OpCodes.Ldnull);
                return;
            }

            if (PointerMarshal.IsPointerLikeType(returnType))
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

            if (returnType.IsValueType)
            {
                il.Emit(OpCodes.Box, returnType);
                return;
            }

            il.Emit(OpCodes.Castclass, typeof(object));
        }
    }
}
