// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
            return luaL_error(L, "zlua: type not found: %s", typeName);

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
            return luaL_error(L, "zlua: assembly not found: %s", assemblyName);

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
