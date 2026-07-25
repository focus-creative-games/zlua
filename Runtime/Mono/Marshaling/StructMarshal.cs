using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using Unity.Collections.LowLevel.Unsafe;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class StructMarshal
    {
        private static readonly Dictionary<Type, bool> s_blittableCache = new Dictionary<Type, bool>();
        private static readonly Dictionary<Type, Func<object, object>> s_copyCache =
            new Dictionary<Type, Func<object, object>>();
        private static readonly Dictionary<Type, Func<object, IntPtr>> s_unboxPtrCache =
            new Dictionary<Type, Func<object, IntPtr>>();

        internal static bool IsStructType(Type type)
        {
            return type != null
                   && type.IsValueType
                   && !type.IsEnum
                   && !type.IsPrimitive
                   && type != typeof(decimal)
                   && !type.IsPointer
                   && !PointerMarshal.IsByRefLikeType(type)
                   && Nullable.GetUnderlyingType(type) == null;
        }

        internal static bool IsBlittable(Type type)
        {
            if (type == null)
            {
                return false;
            }

            lock (s_blittableCache)
            {
                if (s_blittableCache.TryGetValue(type, out bool cached))
                {
                    return cached;
                }

                bool blittable = UnsafeUtility.IsBlittable(type);
                s_blittableCache[type] = blittable;
                return blittable;
            }
        }

        internal static unsafe ByValUserDataHeader* GetByValHeader(IntPtr L, int index)
        {
            UserDataHeader* header = (UserDataHeader*)LuaDll.lua_touserdata(L, index);
            if (header == null || header->Kind != UserDataKind.ByVal)
            {
                return null;
            }

            return (ByValUserDataHeader*)header;
        }

        internal static unsafe byte* PayloadOf(ByValUserDataHeader* header)
        {
            return (byte*)(header + 1);
        }

        /// <summary>
        /// Live data pointer for ByVal userdata: blittable payload, or unboxed companion interior.
        /// </summary>
        internal static unsafe IntPtr GetDataPointer(IntPtr L, int index, Type structType)
        {
            ByValUserDataHeader* header = GetByValHeader(L, index);
            if (header == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: GetDataPointer expects ByVal userdata");
            }

            Type headerType = TypeHandleStore.GetType(header->TypeHandle);
            if (headerType != structType)
            {
                LuaCallbackBoundary.Throw(
                    $"zlua argument mismatch: cannot convert userdata {headerType?.FullName} to struct: {structType.FullName}");
            }

            if (IsBlittable(structType))
            {
                return (IntPtr)PayloadOf(header);
            }

            IntPtr udPtr = (IntPtr)header;
            if (!StructRegistry.TryGetBoxed(udPtr, out object boxed) || boxed == null)
            {
                LuaCallbackBoundary.Throw($"zlua internal error: missing boxed companion for {structType.FullName}");
            }

            if (!structType.IsInstanceOfType(boxed))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua internal error: companion type mismatch for {structType.FullName}, got {boxed.GetType().FullName}");
            }

            return GetUnboxedDataPointer(boxed, structType);
        }

        /// <summary>
        /// Non-blittable GC root box for ByVal userdata (null for blittable).
        /// </summary>
        internal static unsafe object GetCompanionBoxed(IntPtr L, int index, Type structType)
        {
            if (IsBlittable(structType))
            {
                return null;
            }

            ByValUserDataHeader* header = GetByValHeader(L, index);
            if (header == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: GetCompanionBoxed expects ByVal userdata");
            }

            Type headerType = TypeHandleStore.GetType(header->TypeHandle);
            if (headerType != structType)
            {
                LuaCallbackBoundary.Throw(
                    $"zlua argument mismatch: cannot convert userdata {headerType?.FullName} to struct: {structType.FullName}");
            }

            IntPtr udPtr = (IntPtr)header;
            if (!StructRegistry.TryGetBoxed(udPtr, out object boxed) || boxed == null)
            {
                LuaCallbackBoundary.Throw($"zlua internal error: missing boxed companion for {structType.FullName}");
            }

            return boxed;
        }

        /// <summary>Blittable ByVal payload as <c>ref T</c> (requires unmanaged / blittable T).</summary>
        internal static unsafe ref T AsRef<T>(IntPtr L, int index) where T : unmanaged
        {
            return ref *(T*)(void*)GetDataPointer(L, index, typeof(T));
        }

        internal static void PushValue(IntPtr L, object boxedOrValue, Type structType, int metatableRefIndex)
        {
            if (structType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: structType is null");
            }

            object boxed = boxedOrValue ?? Activator.CreateInstance(structType);
            if (!structType.IsInstanceOfType(boxed))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua argument mismatch: cannot convert {boxed.GetType().FullName} to {structType.FullName}");
            }

            object owned = CopyValue(boxed, structType);
            int payloadSize = GetPayloadSize(structType);
            unsafe
            {
                ByValUserDataHeader* header = CreateByValUserDataHeader(L, structType, payloadSize, metatableRefIndex);
                IntPtr udPtr = (IntPtr)header;
                if (IsBlittable(structType) && payloadSize > 0)
                {
                    global::System.Runtime.InteropServices.Marshal.StructureToPtr(owned, (IntPtr)PayloadOf(header), false);
                }
                else if (!IsBlittable(structType))
                {
                    StructRegistry.RegisterBoxed(udPtr, owned);
                }
            }
        }

        internal static void PushValue(IntPtr L, object boxedOrValue, Type structType)
        {
            PushValue(L, boxedOrValue, structType, LuaConsts.LuaNoRef);
        }

        internal static object PushZeroedValue(IntPtr L, Type structType)
        {
            object zero = Activator.CreateInstance(structType);
            PushValue(L, zero, structType, LuaConsts.LuaNoRef);
            return zero;
        }

        /// <summary>
        /// By-val ownership copy out of userdata (Lua → C# argument).
        /// </summary>
        internal static object PopValue(IntPtr L, int index, Type structType)
        {
            unsafe
            {
                ByValUserDataHeader* header = GetByValHeader(L, index);
                if (header == null)
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: cannot convert non-by-val userdata to struct: {structType.FullName}");
                }

                Type headerType = TypeHandleStore.GetType(header->TypeHandle);
                if (headerType != structType)
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: cannot convert userdata {headerType?.FullName} to struct: {structType.FullName}");
                }

                if (IsBlittable(structType))
                {
                    return global::System.Runtime.InteropServices.Marshal.PtrToStructure(
                        (IntPtr)PayloadOf(header),
                        structType);
                }

                IntPtr udPtr = (IntPtr)header;
                if (!StructRegistry.TryGetBoxed(udPtr, out object boxed) || boxed == null)
                {
                    LuaCallbackBoundary.Throw($"zlua internal error: missing boxed companion for {structType.FullName}");
                }

                return CopyValue(boxed, structType);
            }
        }

        private static IntPtr GetUnboxedDataPointer(object boxed, Type structType)
        {
            Func<object, IntPtr> getter;
            lock (s_unboxPtrCache)
            {
                if (!s_unboxPtrCache.TryGetValue(structType, out getter))
                {
                    getter = BuildUnboxDataPointer(structType);
                    s_unboxPtrCache[structType] = getter;
                }
            }

            return getter(boxed);
        }

        private static Func<object, IntPtr> BuildUnboxDataPointer(Type structType)
        {
            DynamicMethod dm = new DynamicMethod(
                $"zlua_unbox_ptr_{structType.Name}",
                typeof(IntPtr),
                new[] { typeof(object) },
                typeof(StructMarshal).Module,
                skipVisibility: true);
            ILGenerator il = dm.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Unbox, structType);
            il.Emit(OpCodes.Conv_I);
            il.Emit(OpCodes.Ret);
            return (Func<object, IntPtr>)dm.CreateDelegate(typeof(Func<object, IntPtr>));
        }

        private static unsafe ByValUserDataHeader* CreateByValUserDataHeader(IntPtr L, Type klass, int payloadSize, int metatableRefIndex)
        {
            UIntPtr totalSize = (UIntPtr)(sizeof(ByValUserDataHeader) + Math.Max(payloadSize, 0));
            ByValUserDataHeader* header = (ByValUserDataHeader*)LuaDll.lua_newuserdatauv(L, totalSize, 0);
            header->Header.Kind = UserDataKind.ByVal;
            header->TypeHandle = TypeHandleStore.GetHandle(klass);

            if (metatableRefIndex != LuaConsts.LuaNoRef)
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, metatableRefIndex);
                LuaDll.lua_setmetatable(L, -2);
            }
            else
            {
                Action<IntPtr, Type> push = MetatableHooks.PushByValMetatable;
                if (push == null)
                {
                    LuaCallbackBoundary.Throw("zlua internal error: ByVal metatable hook not registered");
                }

                push(L, klass);
                LuaDll.lua_setmetatable(L, -2);
            }

            return header;
        }

        private static int GetPayloadSize(Type structType)
        {
            if (!IsBlittable(structType))
            {
                return 0;
            }

            return UnsafeUtility.SizeOf(structType);
        }

        private static object CopyValue(object value, Type structType)
        {
            if (value == null)
            {
                return Activator.CreateInstance(structType);
            }

            Func<object, object> copy;
            lock (s_copyCache)
            {
                if (!s_copyCache.TryGetValue(structType, out copy))
                {
                    copy = BuildCopy(structType);
                    s_copyCache[structType] = copy;
                }
            }

            return copy(value);
        }

        private static Func<object, object> BuildCopy(Type structType)
        {
            ParameterExpression valueParam = Expression.Parameter(typeof(object), "value");
            Expression body = Expression.Convert(Expression.Convert(valueParam, structType), typeof(object));
            return Expression.Lambda<Func<object, object>>(body, valueParam).Compile();
        }
    }
}
