using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ZLua.Emit
{
    /// <summary>
    /// Keeps <see cref="LuaCSFunction"/> delegates alive for native Lua closures.
    /// </summary>
    internal static class ClosurePin
    {
        private static readonly List<LuaCSFunction> s_pins = new List<LuaCSFunction>();
        private static readonly List<GCHandle> s_tagHandles = new List<GCHandle>();

        internal static void Pin(LuaCSFunction fn)
        {
            if (fn == null)
            {
                return;
            }

            lock (s_pins)
            {
                s_pins.Add(fn);
            }
        }

        private static GCHandle PinTag(MethodClosureTag tag)
        {
            GCHandle handle = GCHandle.Alloc(tag, GCHandleType.Normal);
            lock (s_tagHandles)
            {
                s_tagHandles.Add(handle);
            }

            return handle;
        }

        internal static void Push(IntPtr L, LuaCSFunction fn)
        {
            Pin(fn);
            IntPtr ptr = Marshal.GetFunctionPointerForDelegate(fn);
            LuaDll.lua_pushcfunction(L, ptr);
        }

        internal static void PushWithTag(IntPtr L, LuaCSFunction fn, MethodClosureTag tag)
        {
            Pin(fn);
            GCHandle handle = PinTag(tag);
            IntPtr fnPtr = Marshal.GetFunctionPointerForDelegate(fn);
            LuaDll.lua_pushlightuserdata(L, GCHandle.ToIntPtr(handle));
            LuaDll.lua_pushcclosure(L, fnPtr, 1);
        }

        internal static void WriteToTable(IntPtr L, int tableRef, string name, LuaCSFunction fn)
        {
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, tableRef);
            Push(L, fn);
            LuaDll.lua_setfield(L, -2, name);
            LuaDll.lua_pop(L, 1);
        }

        internal static void WriteToTableWithTag(IntPtr L, int tableRef, string name, LuaCSFunction fn, MethodClosureTag tag)
        {
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, tableRef);
            PushWithTag(L, fn, tag);
            LuaDll.lua_setfield(L, -2, name);
            LuaDll.lua_pop(L, 1);
        }

        internal static bool TryGetMethodTag(IntPtr L, int index, out MethodClosureTag tag)
        {
            tag = null;
            if (LuaDll.lua_iscfunction(L, index) == 0)
            {
                return false;
            }

            IntPtr upvalueName = LuaDll.lua_getupvalue(L, index, 1);
            if (upvalueName == IntPtr.Zero)
            {
                return false;
            }

            try
            {
                if (LuaDll.lua_type(L, -1) != LuaDataType.LightUserData)
                {
                    return false;
                }

                IntPtr ptr = LuaDll.lua_touserdata(L, -1);
                if (ptr == IntPtr.Zero)
                {
                    return false;
                }

                GCHandle handle = GCHandle.FromIntPtr(ptr);
                if (!handle.IsAllocated)
                {
                    return false;
                }

                tag = handle.Target as MethodClosureTag;
                return tag != null;
            }
            finally
            {
                LuaDll.lua_pop(L, 1);
            }
        }
    }
}
