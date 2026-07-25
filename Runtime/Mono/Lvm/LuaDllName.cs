namespace ZLua
{
    /// <summary>
    /// Editor native library logical name by Lua API family (see spec 11-MULTI-VERSION §8).
    /// </summary>
    public static class LuaDllName
    {
#if UNITY_IPHONE && !UNITY_EDITOR
        public const string LUA_DLL = "__Internal";
#elif ZLUA_USE_LUAJIT
        public const string LUA_DLL = "luajit";
#elif ZLUA_LUA_5_5
        public const string LUA_DLL = "lua55";
#elif ZLUA_LUA_5_4
        public const string LUA_DLL = "lua54";
#elif ZLUA_LUA_5_3
        public const string LUA_DLL = "lua53";
#elif ZLUA_LUA_5_1
        public const string LUA_DLL = "lua51";
#else
        // Default matches Settings default lua-5.3.6 → series lua53.
        public const string LUA_DLL = "lua53";
#endif
    }
}
