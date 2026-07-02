#include "MetaTableCache.h"



#include "../marshal/ValueMarshaling.h"



#include "lua/lauxlib.h"



#include <unordered_map>



namespace zlua

{

    static std::unordered_map<Il2CppClass*, int> s_byValMetatableRefs;

    static std::unordered_map<Il2CppClass*, int> s_byObjMetatableRefs;

    static std::unordered_map<Il2CppClass*, int> s_referenceMetatableRefs;



    static void RegisterMetatable(

        lua_State* L,

        Il2CppClass* klass,

        int metatableIndex,

        std::unordered_map<Il2CppClass*, int>& refs)

    {

        if (klass == nullptr)

            return;



        const int absIndex = lua_absindex(L, metatableIndex);

        if (!lua_istable(L, absIndex))

            return;



        std::unordered_map<Il2CppClass*, int>::iterator it = refs.find(klass);

        if (it != refs.end())

        {

            if (it->second != LUA_NOREF)

                luaL_unref(L, LUA_REGISTRYINDEX, it->second);

            refs.erase(it);

        }



        lua_pushvalue(L, absIndex);

        refs[klass] = luaL_ref(L, LUA_REGISTRYINDEX);

    }



    static bool TryPushCachedMetatable(

        lua_State* L,

        Il2CppClass* klass,

        const std::unordered_map<Il2CppClass*, int>& refs)

    {

        std::unordered_map<Il2CppClass*, int>::const_iterator it = refs.find(klass);

        if (it == refs.end() || it->second == LUA_NOREF)

            return false;



        lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);

        if (!lua_istable(L, -1))

        {

            lua_pop(L, 1);

            return false;

        }

        return true;

    }



    void MetaTableCache::RegisterByValMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex)

    {

        if (!ValueMarshaling::IsStructClass(klass) && !ValueMarshaling::IsEnumClass(klass))

            return;

        RegisterMetatable(L, klass, metatableIndex, s_byValMetatableRefs);

    }



    void MetaTableCache::RegisterByObjMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex)

    {

        if (!ValueMarshaling::IsStructClass(klass) && !ValueMarshaling::IsEnumClass(klass))
            return;
        RegisterMetatable(L, klass, metatableIndex, s_byObjMetatableRefs);

    }



    void MetaTableCache::RegisterReferenceMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex)

    {

        RegisterMetatable(L, klass, metatableIndex, s_referenceMetatableRefs);

    }



    void MetaTableCache::PushByValMetatable(lua_State* L, Il2CppClass* klass)

    {

        if (TryPushCachedMetatable(L, klass, s_byValMetatableRefs))

            return;

        luaL_error(L, "zlua: ByVal instance metatable missing for type: %s", klass != nullptr ? klass->name : "?");

    }



    bool MetaTableCache::TryPushByObjMetatable(lua_State* L, Il2CppClass* klass)

    {

        return TryPushCachedMetatable(L, klass, s_byObjMetatableRefs);

    }



    bool MetaTableCache::TryPushReferenceMetatable(lua_State* L, Il2CppClass* klass)

    {

        return TryPushCachedMetatable(L, klass, s_referenceMetatableRefs);

    }

}

