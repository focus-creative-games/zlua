using System;
using System.Runtime.InteropServices;

namespace ZLua
{
    [UnmanagedFunctionPointer(LuaDll.CALLING_CONVENTION)]
    public delegate int LuaCSFunction(IntPtr luaState);

    public enum LuaDataType
    {
        Nil = 0,
        Boolean = 1,
        LightUserData = 2,
        Number = 3,
        String = 4,
        Table = 5,
        Function = 6,
        UserData = 7,
        Thread = 8,
    }

    public static class LuaDll
    {
        public const string LUA_DLL = LuaDllName.LUA_DLL;

        public const CallingConvention CALLING_CONVENTION = CallingConvention.Cdecl;

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr luaL_newstate();

#if ZLUA_LUA_5_5
        // Lua 5.5: luaL_openlibs is a C macro → luaL_openselectedlibs(L, ~0, 0); no DLL export.
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_openselectedlibs(IntPtr luaState, int load, int preload);

        public static void luaL_openlibs(IntPtr luaState)
        {
            luaL_openselectedlibs(luaState, ~0, 0);
        }
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_openlibs(IntPtr luaState);
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_close(IntPtr luaState);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_gettop(IntPtr luaState);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_absindex(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_settop(IntPtr luaState, int index);

        public static void lua_pop(IntPtr luaState, int count)
        {
            lua_settop(luaState, -count - 1);
        }

        // lua_remove / lua_insert / lua_replace 在 Lua 5.4 是宏，真实导出是 lua_rotate / lua_copy。
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_rotate(IntPtr luaState, int idx, int n);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_copy(IntPtr luaState, int fromIdx, int toIdx);

        public static void lua_remove(IntPtr luaState, int idx)
        {
            lua_rotate(luaState, idx, -1);
            lua_pop(luaState, 1);
        }

        public static void lua_insert(IntPtr luaState, int idx)
        {
            lua_rotate(luaState, idx, 1);
        }

        public static void lua_replace(IntPtr luaState, int idx)
        {
            lua_copy(luaState, -1, idx);
            lua_pop(luaState, 1);
        }

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushnil(IntPtr luaState);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushvalue(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushinteger(IntPtr luaState, long value);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushnumber(IntPtr luaState, double value);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushboolean(IntPtr luaState, int value);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_pushstring(IntPtr luaState, string value);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushlightuserdata(IntPtr luaState, IntPtr value);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_pushcclosure(IntPtr luaState, IntPtr fn, int nUpValue);

        public static void lua_pushcfunction(IntPtr luaState, IntPtr fn)
        {
            lua_pushcclosure(luaState, fn, 0);
        }

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_getglobal(IntPtr luaState, string name);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_getfield(IntPtr luaState, int index, string key);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setglobal(IntPtr luaState, string name);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setfield(IntPtr luaState, int index, string key);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_setmetatable(IntPtr luaState, int objIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_getmetatable(IntPtr luaState, int objIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_rawgeti(IntPtr luaState, int index, long n);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_rawget(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_rawset(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_rawseti(IntPtr luaState, int index, long n);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_createtable(IntPtr luaState, int nArray, int nRecord);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_newuserdatauv(IntPtr luaState, UIntPtr size, int nUv);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int luaL_ref(IntPtr luaState, int tableIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_unref(IntPtr luaState, int tableIndex, int reference);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_pcallk(IntPtr luaState, int nArgs, int nResults, int errFunc, IntPtr ctx, IntPtr k);

        public static int lua_pcall(IntPtr luaState, int nArgs, int nResults, int errFunc)
        {
            return lua_pcallk(luaState, nArgs, nResults, errFunc, IntPtr.Zero, IntPtr.Zero);
        }

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_error(IntPtr luaState);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_where(IntPtr luaState, int level);

        // lua_tointeger / lua_tonumber 在 Lua 5.4 是宏，真实导出是 *_x 版本。
        [DllImport(LUA_DLL, EntryPoint = "lua_tointegerx", CallingConvention = CALLING_CONVENTION)]
        private static extern long lua_tointegerx(IntPtr luaState, int index, IntPtr isNum);

        public static long lua_tointeger(IntPtr luaState, int index)
        {
            return lua_tointegerx(luaState, index, IntPtr.Zero);
        }

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_type(IntPtr luaState, int index);

        [DllImport(LUA_DLL, EntryPoint = "lua_tonumberx", CallingConvention = CALLING_CONVENTION)]
        private static extern double lua_tonumberx(IntPtr luaState, int index, IntPtr isNum);

        public static double lua_tonumber(IntPtr luaState, int index)
        {
            return lua_tonumberx(luaState, index, IntPtr.Zero);
        }

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_isinteger(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_toboolean(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_touserdata(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_topointer(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_iscfunction(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_getupvalue(IntPtr luaState, int funcIndex, int n);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_tolstring(IntPtr luaState, int index, out UIntPtr strLen);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_pushlstring(IntPtr luaState, IntPtr data, UIntPtr length);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr luaL_checklstring(IntPtr luaState, int arg, out UIntPtr length);

        public static string luaL_checkstring(IntPtr luaState, int arg)
        {
            IntPtr ptr = luaL_checklstring(luaState, arg, out UIntPtr length);
            if (ptr == IntPtr.Zero)
            {
                return null;
            }

            unsafe
            {
                return System.Text.Encoding.UTF8.GetString((byte*)ptr, (int)length);
            }
        }

        public static bool lua_isuserdata(IntPtr luaState, int index) => lua_type(luaState, index) == LuaDataType.UserData;

        public static bool lua_istable(IntPtr luaState, int index) => lua_type(luaState, index) == LuaDataType.Table;

        public static bool lua_isnil(IntPtr luaState, int index) => lua_type(luaState, index) == LuaDataType.Nil;

        public static bool lua_isnoneornil(IntPtr luaState, int index) => lua_type(luaState, index) <= LuaDataType.Nil;

        public static int lua_upvalueindex(int i) => LuaConsts.LuaRegistryIndex - i;
    }
}
