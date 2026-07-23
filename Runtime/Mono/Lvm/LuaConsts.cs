namespace ZLua
{
    /// <summary>
    /// Mirrors <c>zlua/LuaConsts.h</c>.
    /// </summary>
    public static class LuaConsts
    {
        public const int LuaMultiRet = -1;
        public const int LuaiMaxStack = 1_000_000;
        public const int LuaRegistryIndex = -LuaiMaxStack - 1000;
        public const int LuaNoRef = -1;
        public const int LuaRefNil = -2;

        public const string MetaIndex = "__index";
        public const string MetaNewIndex = "__newindex";
        public const string MetaCall = "__call";
        public const string MetaGc = "__gc";
        public const string MetaToString = "__tostring";
        public const string MetaLen = "__len";
        public const string MetaMode = "__mode";

        public const string WeakModeValue = "v";

        public const string FullName = "__fullname";
        public const string Klass = "__klass";
        public const string ByValInstanceMt = "__byval_instance_mt";
        public const string ByObjInstanceMt = "__byobj_instance_mt";
        public const string Struct = "__struct";
        public const string Enum = "__enum";
        public const string Type = "__type";
        public const string Nullable = "__nullable";
        public const string UdKind = "__zlua_ud_kind";

        public const string UdKindByVal = "byval";
        public const string UdKindByObj = "byobj";
        public const string UdKindOpaqueParameter = "opaque_parameter";

        public const string Get = "get";
        public const string Set = "set";
        public const string Fire = "fire";
        public const string Default = "_default";
    }
}
