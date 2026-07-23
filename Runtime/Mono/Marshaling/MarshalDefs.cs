using System;
using System.Runtime.InteropServices;

namespace ZLua.Marshaling
{
    internal enum UserDataKind : byte
    {
        Unknown = 0,
        ByObj = 1,
        ByVal = 2,
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct UserDataHeader
    {
        public UserDataKind Kind;
    }

    /// <summary>
    /// By-obj userdata header. Managed object / view type live in <see cref="ObjectRegistry"/> slots.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct ZLuaObjectUserData
    {
        public UserDataHeader Header;
        public uint SlotIndex;
    }

    /// <summary>
    /// By-val userdata header; payload bytes follow immediately after this struct.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct ByValUserDataHeader
    {
        public UserDataHeader Header;
        public IntPtr TypeHandle;
    }

    /// <summary>
    /// Injected by Mt so Marshal does not take a hard dependency on TypeRegistry internals.
    /// </summary>
    internal static class MetatableHooks
    {
        internal static Action<IntPtr, Type> PushByObjMetatable;
        internal static Action<IntPtr, Type> PushByValMetatable;
    }
}
