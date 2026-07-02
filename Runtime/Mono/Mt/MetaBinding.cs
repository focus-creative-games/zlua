using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using ZLua.DelegateImpl;
using ZLua.Marshal;
using ZLua.MethodBridge;
using ZLua;

namespace ZLua.Mt
{
    /// <summary>Meta table member registration facade (aligns with Il2Cpp mt/MetaBinding).</summary>
    internal static class MetaBinding
    {
        internal static void EnsureMethodAliasKeysValid(Type type) =>
            MethodBridgeCore.EnsureMethodAliasKeysValid(type);

        internal static void RegisterStaticMethods(IntPtr luaState, int metatableIndex, Type type) =>
            MethodBridgeCore.RegisterStaticMethods(luaState, metatableIndex, type);

        internal static void RegisterInstanceMethods(IntPtr luaState, int metatableIndex, Type type) =>
            MethodBridgeCore.RegisterInstanceMethods(luaState, metatableIndex, type);

        internal static void RegisterConstructors(IntPtr luaState, int metatableIndex, Type type) =>
            MethodBridgeCore.RegisterConstructors(luaState, metatableIndex, type);
    }
}
