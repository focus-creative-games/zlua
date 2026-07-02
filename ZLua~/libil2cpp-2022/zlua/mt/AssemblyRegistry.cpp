#include "AssemblyRegistry.h"

#include "../LuaConsts.h"
#include "../utils/MetadataUtil.h"
#include "TypeRegistry.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaException.h"

#include "il2cpp-config.h"
#include "lua/lauxlib.h"

namespace zlua
{
    constexpr const char* kCSharpRootName = "CSharp";

    static int ResolveAssemblyTypeIndex(lua_State* L);
    static int ResolveAssemblyIndex(lua_State* L);

    static bool TryGetTableByKey(lua_State* L, int idx, const char* key)
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
        ZLUA_TRY_BEGIN()
        const Il2CppAssembly* assembly = (const Il2CppAssembly*)lua_touserdata(L, lua_upvalueindex(1));
        IL2CPP_ASSERT(assembly != nullptr);
        const char* typeName = luaL_checkstring(L, 2);

        if (TryGetTableByKey(L, 1, typeName))
            return 1;

        Il2CppClass* klass = MetadataUtil::ResolveType(assembly, typeName);
        if (klass == nullptr)
            return 0;

        TypeRegistry::PushInternedTypeTable(L, klass);
        lua_pushvalue(L, -1);
        lua_setfield(L, 1, typeName);
        return 1;
        ZLUA_TRY_END();
    }

    static int ResolveAssemblyIndex(lua_State* L)
    {
        ZLUA_TRY_BEGIN()
        const char* assemblyName = luaL_checkstring(L, 2);

        if (TryGetTableByKey(L, 1, assemblyName))
            return 1;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
        if (assembly == nullptr)
            return 0;

        // create a new table for the assembly
        lua_newtable(L);

        // create a new table for meta table of the assembly
        lua_newtable(L);
        lua_pushlightuserdata(L, (void*)assembly);
        lua_pushcclosure(L, ResolveAssemblyTypeIndex, 1);
        lua_setfield(L, -2, LuaConsts::MetaIndex);
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, 1, assemblyName);
        return 1;
        ZLUA_TRY_END();
    }

    void AssemblyRegistry::InitializeCSharpRoot(lua_State* L)
    {
        IL2CPP_ASSERT(L != nullptr);
        LuaStackGuard stackGuard(L);

        lua_getglobal(L, kCSharpRootName);
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_newtable(L);
            lua_pushcfunction(L, ResolveAssemblyIndex);
            lua_setfield(L, -2, LuaConsts::MetaIndex);
            lua_setmetatable(L, -2);
            lua_setglobal(L, kCSharpRootName);
        }
        else
        {
            lua_pop(L, 1);
        }
    }
}
