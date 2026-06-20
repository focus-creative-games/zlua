using System;
using System.Runtime.InteropServices;
using System.Text;

namespace NextLua
{
    public static class LuaDllExtension
    {
        [DllImport(LuaDll.LUA_DLL, EntryPoint = "luaL_loadstring", CallingConvention = LuaDll.CALLING_CONVENTION)]
        public static extern int loadstring(IntPtr luaState, string chunk);

        public static int dostring(IntPtr luaState, string chunk)
        {
            int result = loadstring(luaState, chunk);
            if (result != 0)
            {
                return result;
            }

            return LuaDll.lua_pcall(luaState, 0, LuaConsts.LuaMultiRet, 0);
        }

        public static unsafe string tostring(IntPtr luaState, int index)
        {
            IntPtr strPtr = LuaDll.lua_tolstring(luaState, index, out UIntPtr len);
            if (strPtr == IntPtr.Zero)
            {
                return null;
            }

            return Encoding.UTF8.GetString((byte*)strPtr, (int)len);
        }

        public static int error(IntPtr luaState, string msg)
        {
            LuaDll.lua_pushstring(luaState, msg);
            return LuaDll.lua_error(luaState);
        }

        public static void RegisterCallback(IntPtr luaState, string globalName, LuaCSFunction func)
        {
            IntPtr function = Marshal.GetFunctionPointerForDelegate(func);
            LuaDll.lua_pushcfunction(luaState, function);
            LuaDll.lua_setglobal(luaState, globalName);
        }
    }
}
