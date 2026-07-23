using System;
using System.Collections.Generic;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Mt
{
    internal static class MetaTableCache
    {
        private static int s_byValCacheRef = LuaConsts.LuaNoRef;
        private static int s_byObjCacheRef = LuaConsts.LuaNoRef;
        private static readonly Dictionary<Type, int> s_byValMetatableRefs = new Dictionary<Type, int>();
        private static readonly Dictionary<Type, int> s_byObjMetatableRefs = new Dictionary<Type, int>();

        internal static void Initialize(IntPtr L)
        {
            if (s_byValCacheRef != LuaConsts.LuaNoRef)
            {
                return;
            }

            LuaDll.lua_createtable(L, 0, 0);
            s_byValCacheRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            LuaDll.lua_createtable(L, 0, 0);
            s_byObjCacheRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);

            MetatableHooks.PushByObjMetatable = PushByObjMetatable;
            MetatableHooks.PushByValMetatable = PushByValMetatable;
        }

        internal static void Shutdown(IntPtr L)
        {
            foreach (KeyValuePair<Type, int> kv in s_byValMetatableRefs)
            {
                LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, kv.Value);
            }

            s_byValMetatableRefs.Clear();

            foreach (KeyValuePair<Type, int> kv in s_byObjMetatableRefs)
            {
                LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, kv.Value);
            }

            s_byObjMetatableRefs.Clear();

            if (s_byValCacheRef != LuaConsts.LuaNoRef)
            {
                LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, s_byValCacheRef);
                s_byValCacheRef = LuaConsts.LuaNoRef;
            }

            if (s_byObjCacheRef != LuaConsts.LuaNoRef)
            {
                LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, s_byObjCacheRef);
                s_byObjCacheRef = LuaConsts.LuaNoRef;
            }

            MetatableHooks.PushByObjMetatable = null;
            MetatableHooks.PushByValMetatable = null;
        }

        internal static void PushByValMetatable(IntPtr L, Type type)
        {
            PushCachedMetatable(L, type, s_byValCacheRef, LuaConsts.ByValInstanceMt);
        }

        internal static void PushByObjMetatable(IntPtr L, Type type)
        {
            PushCachedMetatable(L, type, s_byObjCacheRef, LuaConsts.ByObjInstanceMt);
        }

        internal static int GetOrCreateByObjMetatableRef(IntPtr L, Type type)
        {
            if (s_byObjMetatableRefs.TryGetValue(type, out int existing))
            {
                return existing;
            }

            PushByObjMetatable(L, type);
            LuaDll.lua_pushvalue(L, -1);
            int tableRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            LuaDll.lua_pop(L, 1);
            s_byObjMetatableRefs[type] = tableRef;
            return tableRef;
        }

        private static void PushCachedMetatable(IntPtr L, Type type, int cacheRef, string mtField)
        {
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, cacheRef);
            TypeHandleStore.PushLightUserData(L, type);
            LuaDll.lua_rawget(L, -2);
            if (!LuaDll.lua_isnil(L, -1))
            {
                LuaDll.lua_remove(L, -2);
                return;
            }

            LuaDll.lua_pop(L, 1);

            TypeRegistry.PushInternedTypeTable(L, type);
            LuaDll.lua_getfield(L, -1, mtField);
            if (LuaDll.lua_isnil(L, -1))
            {
                LuaDll.lua_pop(L, 3); // nil + type table + cache table
                LuaCallbackBoundary.Throw($"zlua: missing {mtField} on type table for {type.FullName}");
            }

            LuaDll.lua_remove(L, -2);

            TypeHandleStore.PushLightUserData(L, type);
            LuaDll.lua_pushvalue(L, -2);
            LuaDll.lua_rawset(L, -4);

            LuaDll.lua_remove(L, -2);
        }
    }
}
