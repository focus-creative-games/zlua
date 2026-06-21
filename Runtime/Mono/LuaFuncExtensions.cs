namespace NovaLua
{
    public static class LuaFuncExtensions
    {
        public static LuaFunc ToFunc(this LuaMethod method)
        {
            return new LuaFunc(method);
        }

        public static LuaAction ToAction(this LuaMethod method)
        {
            return new LuaAction(method);
        }
    }
}
