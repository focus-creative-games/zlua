using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using ZLua;
using ZLua.Marshal;
using ZLua.MethodBridge;
using ZLua.DelegateImpl;

namespace ZLua.Mt
{
    internal static class AssemblyRegistry
    {
        internal static void EnsureCSharpRoot()
        {
            IntPtr luaState = LuaMonoAppDomain.LuaEnv.LuaState;
            int oldTop = LuaDll.lua_gettop(luaState);
            try
            {
                TypeRegistry.EnsureCSharpRoot(luaState);
            }
            finally
            {
                LuaDll.lua_settop(luaState, oldTop);
            }
        }
    }
}
