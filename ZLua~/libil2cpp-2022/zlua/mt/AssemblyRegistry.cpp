#include "AssemblyRegistry.h"

#include "../LuaEnv.h"
#include "../MetadataUtil.h"
#include "MetaBinding.h"
#include "TypeRegistry.h"

#include "lua/lauxlib.h"

namespace zlua
{
    static int ResolveAssemblyTypeIndex(lua_State* L);
    static int ResolveAssemblyIndex(lua_State* L);

    static bool TryGetFieldAsTable(lua_State* L, int idx, const char* key)
    {
        IL2CPP_ASSERT(idx > 0);
        lua_pushstring(L, key);
        lua_rawget(L, idx);
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        return true;
    }

    static int ResolveAssemblyTypeIndex(lua_State* L)
    {
        const Il2CppAssembly* assembly = (const Il2CppAssembly*)lua_touserdata(L, lua_upvalueindex(1));
        const char* typeName = luaL_checkstring(L, 2);
        if (assembly == nullptr || typeName == nullptr || typeName[0] == '\0')
            return 0;

        if (TryGetFieldAsTable(L, 1, typeName))
            return 1;

        Il2CppClass* klass = MetadataUtil::ResolveType(assembly, typeName);
        if (klass == nullptr)
            return 0;

        MetaBinding::ValidateMethodAliasKeysOrThrow(L, klass);
        TypeRegistry::PushTypeTable(L, klass);
        lua_pushvalue(L, -1);
        lua_setfield(L, 1, typeName);
        return 1;
    }

    static int ResolveAssemblyIndex(lua_State* L)
    {
        const char* assemblyName = luaL_checkstring(L, 2);
        if (assemblyName == nullptr || assemblyName[0] == '\0')
            return 0;

        if (TryGetFieldAsTable(L, 1, assemblyName))
            return 1;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
        if (assembly == nullptr)
            return 0;

        lua_newtable(L);

        lua_newtable(L);
        lua_pushlightuserdata(L, (void*)assembly);
        lua_pushcclosure(L, ResolveAssemblyTypeIndex, 1);
        lua_setfield(L, -2, "__index");
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, 1, assemblyName);
        return 1;
    }

    void AssemblyRegistry::EnsureCSharpRoot()
    {
        lua_State* L = LuaEnv::GetState();
        const int oldTop = lua_gettop(L);

        lua_getglobal(L, "CSharp");
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_newtable(L);
            lua_pushcfunction(L, ResolveAssemblyIndex);
            lua_setfield(L, -2, "__index");
            lua_setmetatable(L, -2);
            lua_setglobal(L, "CSharp");
        }
        else
        {
            lua_pop(L, 1);
        }

        lua_settop(L, oldTop);
    }
}
