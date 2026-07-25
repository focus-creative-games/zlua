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

#if ZLUA_LUA_5_1
        // lua_absindex is Lua 5.2+.
        public static int lua_absindex(IntPtr luaState, int index)
        {
            if (index > 0 || index <= LuaConsts.LuaRegistryIndex)
            {
                return index;
            }

            return lua_gettop(luaState) + 1 + index;
        }
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_absindex(IntPtr luaState, int index);
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_settop(IntPtr luaState, int index);

        public static void lua_pop(IntPtr luaState, int count)
        {
            lua_settop(luaState, -count - 1);
        }

#if ZLUA_LUA_5_1 || ZLUA_LUA_5_2
        // Lua 5.1/5.2 export lua_remove / insert / replace (lua_rotate is 5.3+).
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_remove(IntPtr luaState, int idx);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_insert(IntPtr luaState, int idx);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_replace(IntPtr luaState, int idx);
#else
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
#endif

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

#if ZLUA_LUA_5_1 || ZLUA_LUA_5_2
        // 5.1/5.2: get* APIs return void (5.3+ return type). P/Invoke as int reads garbage → false "not available".
        [DllImport(LUA_DLL, EntryPoint = "lua_getfield", CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_getfield_void(IntPtr luaState, int index, string key);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setfield(IntPtr luaState, int index, string key);

        public static LuaDataType lua_getfield(IntPtr luaState, int index, string key)
        {
            lua_getfield_void(luaState, index, key);
            return lua_type(luaState, -1);
        }

#if ZLUA_LUA_5_1
        // 5.1: get/setglobal are macros over LUA_GLOBALSINDEX.
        public static LuaDataType lua_getglobal(IntPtr luaState, string name)
        {
            return lua_getfield(luaState, LuaConsts.LuaGlobalsIndex, name);
        }

        public static void lua_setglobal(IntPtr luaState, string name)
        {
            lua_setfield(luaState, LuaConsts.LuaGlobalsIndex, name);
        }
#else
        // 5.2: real void functions (globals live in registry).
        [DllImport(LUA_DLL, EntryPoint = "lua_getglobal", CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_getglobal_void(IntPtr luaState, string name);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setglobal(IntPtr luaState, string name);

        public static LuaDataType lua_getglobal(IntPtr luaState, string name)
        {
            lua_getglobal_void(luaState, name);
            return lua_type(luaState, -1);
        }
#endif

        [DllImport(LUA_DLL, EntryPoint = "lua_rawgeti", CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_rawgeti_void(IntPtr luaState, int index, int n);

        public static LuaDataType lua_rawgeti(IntPtr luaState, int index, long n)
        {
            lua_rawgeti_void(luaState, index, (int)n);
            return lua_type(luaState, -1);
        }

        [DllImport(LUA_DLL, EntryPoint = "lua_rawget", CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_rawget_void(IntPtr luaState, int index);

        public static LuaDataType lua_rawget(IntPtr luaState, int index)
        {
            lua_rawget_void(luaState, index);
            return lua_type(luaState, -1);
        }
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_getglobal(IntPtr luaState, string name);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_getfield(IntPtr luaState, int index, string key);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setglobal(IntPtr luaState, string name);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_setfield(IntPtr luaState, int index, string key);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_rawgeti(IntPtr luaState, int index, long n);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_rawget(IntPtr luaState, int index);
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_rawset(IntPtr luaState, int index);

#if ZLUA_LUA_5_1 || ZLUA_LUA_5_2
        [DllImport(LUA_DLL, EntryPoint = "lua_rawseti", CallingConvention = CALLING_CONVENTION)]
        private static extern void lua_rawseti_int(IntPtr luaState, int index, int n);

        public static void lua_rawseti(IntPtr luaState, int index, long n)
        {
            lua_rawseti_int(luaState, index, (int)n);
        }
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_rawseti(IntPtr luaState, int index, long n);
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_setmetatable(IntPtr luaState, int objIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_getmetatable(IntPtr luaState, int objIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void lua_createtable(IntPtr luaState, int nArray, int nRecord);

        // Lua 5.4+: lua_newuserdatauv. Lua 5.3 / default series: lua_newuserdata (nUv ignored).
#if ZLUA_LUA_5_4 || ZLUA_LUA_5_5
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern IntPtr lua_newuserdatauv(IntPtr luaState, UIntPtr size, int nUv);
#else
        [DllImport(LUA_DLL, EntryPoint = "lua_newuserdata", CallingConvention = CALLING_CONVENTION)]
        private static extern IntPtr lua_newuserdata(IntPtr luaState, UIntPtr size);

        public static IntPtr lua_newuserdatauv(IntPtr luaState, UIntPtr size, int nUv)
        {
            return lua_newuserdata(luaState, size);
        }
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int luaL_ref(IntPtr luaState, int tableIndex);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_unref(IntPtr luaState, int tableIndex, int reference);

#if ZLUA_LUA_5_1
        // lua_pcallk is Lua 5.2+.
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_pcall(IntPtr luaState, int nArgs, int nResults, int errFunc);
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_pcallk(IntPtr luaState, int nArgs, int nResults, int errFunc, IntPtr ctx, IntPtr k);

        public static int lua_pcall(IntPtr luaState, int nArgs, int nResults, int errFunc)
        {
            return lua_pcallk(luaState, nArgs, nResults, errFunc, IntPtr.Zero, IntPtr.Zero);
        }
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_error(IntPtr luaState);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern void luaL_where(IntPtr luaState, int level);

#if ZLUA_LUA_5_1
        // 5.1 exports lua_tointeger / lua_tonumber (no *_x).
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern long lua_tointeger(IntPtr luaState, int index);

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern double lua_tonumber(IntPtr luaState, int index);
#else
        // lua_tointeger / lua_tonumber 在 Lua 5.4 是宏，真实导出是 *_x 版本。
        [DllImport(LUA_DLL, EntryPoint = "lua_tointegerx", CallingConvention = CALLING_CONVENTION)]
        private static extern long lua_tointegerx(IntPtr luaState, int index, IntPtr isNum);

        public static long lua_tointeger(IntPtr luaState, int index)
        {
            return lua_tointegerx(luaState, index, IntPtr.Zero);
        }

        [DllImport(LUA_DLL, EntryPoint = "lua_tonumberx", CallingConvention = CALLING_CONVENTION)]
        private static extern double lua_tonumberx(IntPtr luaState, int index, IntPtr isNum);

        public static double lua_tonumber(IntPtr luaState, int index)
        {
            return lua_tonumberx(luaState, index, IntPtr.Zero);
        }
#endif

#if ZLUA_LUA_5_1 || ZLUA_LUA_5_2
        // lua_isinteger is Lua 5.3+.
        public static int lua_isinteger(IntPtr luaState, int index)
        {
            if (lua_type(luaState, index) != LuaDataType.Number)
            {
                return 0;
            }

            double n = lua_tonumber(luaState, index);
            return n == Math.Floor(n) ? 1 : 0;
        }
#else
        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern int lua_isinteger(IntPtr luaState, int index);
#endif

        [DllImport(LUA_DLL, CallingConvention = CALLING_CONVENTION)]
        public static extern LuaDataType lua_type(IntPtr luaState, int index);

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

#if ZLUA_LUA_5_1
        // 5.1: #define lua_upvalueindex(i) (LUA_GLOBALSINDEX-(i))
        public static int lua_upvalueindex(int i) => LuaConsts.LuaGlobalsIndex - i;
#else
        public static int lua_upvalueindex(int i) => LuaConsts.LuaRegistryIndex - i;
#endif
    }
}
