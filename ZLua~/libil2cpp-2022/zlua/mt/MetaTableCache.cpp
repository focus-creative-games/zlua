#include "MetaTableCache.h"

#include "TypeRegistry.h"
#include "../LuaConsts.h"
#include "../utils/Collection.h"

namespace zlua
{

static int s_byValCacheRef = LUA_NOREF;
static int s_byObjCacheRef = LUA_NOREF;
static HashMap<Il2CppClass*, int> s_byValMetatableRefs;
static HashMap<Il2CppClass*, int> s_byObjMetatableRefs;

void MetaTableCache::Initialize(lua_State* L)
{
    IL2CPP_ASSERT(s_byValCacheRef == LUA_NOREF);
    IL2CPP_ASSERT(s_byObjCacheRef == LUA_NOREF);

    lua_newtable(L);
    s_byValCacheRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_newtable(L);
    s_byObjCacheRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

void MetaTableCache::Shutdown(lua_State* L)
{
    for (auto& kv : s_byValMetatableRefs)
        luaL_unref(L, LUA_REGISTRYINDEX, kv.second);
    s_byValMetatableRefs.clear();

    for (auto& kv : s_byObjMetatableRefs)
        luaL_unref(L, LUA_REGISTRYINDEX, kv.second);
    s_byObjMetatableRefs.clear();

    if (s_byValCacheRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, s_byValCacheRef);
        s_byValCacheRef = LUA_NOREF;
    }
    if (s_byObjCacheRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, s_byObjCacheRef);
        s_byObjCacheRef = LUA_NOREF;
    }
}

static void PushCachedMetatable(lua_State* L, Il2CppClass* klass, int cacheRef, const char* mtField)
{
    IL2CPP_ASSERT(cacheRef != LUA_NOREF);
    IL2CPP_ASSERT(klass != nullptr);

    lua_rawgeti(L, LUA_REGISTRYINDEX, cacheRef);
    lua_pushlightuserdata(L, klass);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1))
    {
        lua_remove(L, -2);
        return;
    }

    lua_pop(L, 1);

    TypeRegistry::PushInternedTypeTable(L, klass);
    lua_getfield(L, -1, mtField);
    lua_remove(L, -2);

    lua_pushlightuserdata(L, klass);
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);

    lua_remove(L, -2);
}

static int GetOrCreateMetatableRef(lua_State* L, Il2CppClass* klass, int cacheRef, const char* mtField, HashMap<Il2CppClass*, int>& refMap)
{
    HashMap<Il2CppClass*, int>::iterator it = refMap.find(klass);
    if (it != refMap.end())
        return it->second;

    PushCachedMetatable(L, klass, cacheRef, mtField);
    lua_pushvalue(L, -1);
    const int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
    refMap[klass] = ref;
    return ref;
}

int MetaTableCache::GetOrCreateByValMetatableRef(lua_State* L, Il2CppClass* klass)
{
    return GetOrCreateMetatableRef(L, klass, s_byValCacheRef, LuaConsts::ByValInstanceMt, s_byValMetatableRefs);
}

int MetaTableCache::GetOrCreateByObjMetatableRef(lua_State* L, Il2CppClass* klass)
{
    return GetOrCreateMetatableRef(L, klass, s_byObjCacheRef, LuaConsts::ByObjInstanceMt, s_byObjMetatableRefs);
}

int MetaTableCache::PushByValMetatable(lua_State* L, Il2CppClass* klass)
{
    const int ref = GetOrCreateByValMetatableRef(L, klass);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    return 1;
}

int MetaTableCache::PushByObjMetatable(lua_State* L, Il2CppClass* klass)
{
    const int ref = GetOrCreateByObjMetatableRef(L, klass);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    return 1;
}

} // namespace zlua
