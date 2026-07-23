using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ZLua.Utils
{
    /// <summary>
    /// Stable lightuserdata tokens for <see cref="Type"/> (Il2Cpp <c>Il2CppClass*</c> equivalent).
    /// </summary>
    internal static class TypeHandleStore
    {
        private static readonly Dictionary<Type, IntPtr> TypeToHandle = new Dictionary<Type, IntPtr>();
        private static readonly Dictionary<IntPtr, Type> HandleToType = new Dictionary<IntPtr, Type>();
        private static readonly object Sync = new object();

        internal static IntPtr GetHandle(Type type)
        {
            if (type == null)
            {
                throw new ArgumentNullException(nameof(type));
            }

            lock (Sync)
            {
                if (TypeToHandle.TryGetValue(type, out IntPtr existing))
                {
                    return existing;
                }

                GCHandle handle = GCHandle.Alloc(type, GCHandleType.Normal);
                IntPtr ptr = GCHandle.ToIntPtr(handle);
                TypeToHandle[type] = ptr;
                HandleToType[ptr] = type;
                return ptr;
            }
        }

        internal static Type GetType(IntPtr handle)
        {
            if (handle == IntPtr.Zero)
            {
                return null;
            }

            lock (Sync)
            {
                return HandleToType.TryGetValue(handle, out Type type) ? type : null;
            }
        }

        internal static void PushLightUserData(IntPtr luaState, Type type)
        {
            LuaDll.lua_pushlightuserdata(luaState, GetHandle(type));
        }

        internal static Type ReadLightUserData(IntPtr luaState, int index)
        {
            IntPtr ptr = LuaDll.lua_touserdata(luaState, index);
            return GetType(ptr);
        }
    }
}
