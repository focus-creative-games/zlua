using ZLua.Mt;
using ZLua.MethodBridge;
using ZLua.Marshal;
using ZLua.DelegateImpl;

namespace ZLua
{
    internal static class ZLuaLib
    {
        internal static void RegisterGlobals(LuaEnv luaEnv)
        {
            Mt.TypeRegistry.RegisterZLuaApi(luaEnv);
        }
    }
}
