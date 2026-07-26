using System;
using System.Collections.Generic;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Mt
{
    internal static class AssemblyRegistry
    {
        private static readonly LuaCSFunction s_resolveAssemblyIndex = ResolveAssemblyIndex;
        private static readonly LuaCSFunction s_resolveAssemblyTypeIndex = ResolveAssemblyTypeIndex;

        private static readonly Dictionary<string, Assembly> s_assemblyByLuaName =
            new Dictionary<string, Assembly>(StringComparer.Ordinal);

        internal static void EnsureCSharpRoot()
        {
            IntPtr L = LuaMonoAppDomain.LuaEnv.L;
            int oldTop = LuaDll.lua_gettop(L);
            try
            {
                InitializeCSharpRoot(L);
            }
            finally
            {
                LuaDll.lua_settop(L, oldTop);
            }
        }

        internal static void InitializeCSharpRoot(IntPtr L)
        {
            LuaDataType csharpType = LuaDll.lua_getglobal(L, "CSharp");
            if (csharpType == LuaDataType.Table)
            {
                LuaDll.lua_pop(L, 1);
                return;
            }

            LuaDll.lua_pop(L, 1);
            LuaDll.lua_createtable(L, 0, 8);
            LuaDll.lua_createtable(L, 0, 1);
            LuaCallbackGate.PushCFunction(
                L,
                global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_resolveAssemblyIndex));
            LuaDll.lua_setfield(L, -2, LuaConsts.MetaIndex);
            LuaDll.lua_setmetatable(L, -2);
            LuaDll.lua_setglobal(L, "CSharp");
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ResolveAssemblyIndex(IntPtr L)
        {
            LuaCallbackBoundary.Enter();
            try
            {
                try
                {
                    string assemblyName = LuaDll.luaL_checkstring(L, 2);
                    if (TryGetTableByKey(L, 1, assemblyName))
                    {
                        return 1;
                    }

                    Assembly assembly = ResolveAssembly(assemblyName);
                    if (assembly == null)
                    {
                        return 0;
                    }

                    LuaDll.lua_createtable(L, 0, 16);
                    LuaDll.lua_createtable(L, 0, 1);
                    LuaDll.lua_pushstring(L, assemblyName);
                    LuaCallbackGate.PushCClosure(
                        L,
                        global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_resolveAssemblyTypeIndex),
                        1);
                    LuaDll.lua_setfield(L, -2, LuaConsts.MetaIndex);
                    LuaDll.lua_setmetatable(L, -2);

                    LuaDll.lua_pushvalue(L, -1);
                    LuaDll.lua_setfield(L, 1, assemblyName);
                    return 1;
                }
                catch (Exception ex)
                {
                    return LuaCallbackBoundary.ToLuaError(L, ex);
                }
            }
            finally
            {
                LuaCallbackBoundary.Leave();
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ResolveAssemblyTypeIndex(IntPtr L)
        {
            LuaCallbackBoundary.Enter();
            try
            {
                try
                {
                    string assemblyName = LuaDllExtension.tostring(L, LuaDll.lua_upvalueindex(LuaCallbackGate.ManagedUpvalueIndex(1)));
                    string typeName = LuaDll.luaL_checkstring(L, 2);

                    if (TryGetTableByKey(L, 1, typeName))
                    {
                        return 1;
                    }

                    Assembly assembly = ResolveAssembly(assemblyName);
                    if (assembly == null)
                    {
                        return 0;
                    }

                    Type type = FindTypeInAssembly(assembly, typeName);
                    if (type == null)
                    {
                        return 0;
                    }

                    TypeRegistry.PushInternedTypeTable(L, type);
                    LuaDll.lua_pushvalue(L, -1);
                    LuaDll.lua_setfield(L, 1, typeName);
                    return 1;
                }
                catch (Exception ex)
                {
                    return LuaCallbackBoundary.ToLuaError(L, ex);
                }
            }
            finally
            {
                LuaCallbackBoundary.Leave();
            }
        }

        private static Type FindTypeInAssembly(Assembly assembly, string luaTypeName)
        {
            Type direct = assembly.GetType(luaTypeName, throwOnError: false);
            if (direct != null)
            {
                return direct;
            }

            // Namespace.Outer.Inner → Namespace.Outer+Inner (and deeper nestings)
            string[] parts = luaTypeName.Split('.');
            if (parts.Length < 2)
            {
                return null;
            }

            for (int nestStart = parts.Length - 1; nestStart >= 1; nestStart--)
            {
                string nsAndOuter = string.Join(".", parts, 0, nestStart);
                string nested = string.Join("+", parts, nestStart, parts.Length - nestStart);
                string candidate = nsAndOuter + "+" + nested;
                direct = assembly.GetType(candidate, throwOnError: false);
                if (direct != null)
                {
                    return direct;
                }
            }

            return null;
        }

        private static Assembly ResolveAssembly(string luaAssemblyName)
        {
            if (string.IsNullOrEmpty(luaAssemblyName))
            {
                return null;
            }

            if (s_assemblyByLuaName.TryGetValue(luaAssemblyName, out Assembly cached))
            {
                return cached;
            }

            foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                string name = assembly.GetName().Name;
                if (string.Equals(name, luaAssemblyName, StringComparison.Ordinal))
                {
                    s_assemblyByLuaName[luaAssemblyName] = assembly;
                    return assembly;
                }
            }

            return null;
        }

        private static bool TryGetTableByKey(IntPtr L, int idx, string key)
        {
            LuaDll.lua_pushstring(L, key);
            LuaDll.lua_rawget(L, idx);
            if (!LuaDll.lua_istable(L, -1))
            {
                LuaDll.lua_pop(L, 1);
                return false;
            }

            return true;
        }
    }
}
