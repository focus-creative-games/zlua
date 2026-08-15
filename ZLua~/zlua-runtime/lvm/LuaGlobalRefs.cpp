#include "LuaGlobalRefs.h"

#include "LuaEnv.h"
#include "LuaLoader.h"

#include "../utils/LuaException.h"
#include "../utils/LuaStackGuard.h"

#include <string>
#include <unordered_map>

namespace zlua
{
static std::unordered_map<std::string, int> s_ModuleRefs;
static std::unordered_map<std::string, int> s_ModuleFunctionRefs;

void LuaGlobalRefs::Clear()
{
    lua_State* L = LuaEnv::GetState();
    if (L != nullptr)
    {
        for (auto& kv : s_ModuleFunctionRefs)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, kv.second);
        }

        for (auto& kv : s_ModuleRefs)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, kv.second);
        }
    }

    s_ModuleFunctionRefs.clear();
    s_ModuleRefs.clear();
}

int LuaGlobalRefs::GetModuleRef(const char* moduleName)
{
    auto it = s_ModuleRefs.find(moduleName);
    if (it != s_ModuleRefs.end())
    {
        return it->second;
    }

    if (LuaLoader::GetModuleLoader() == nullptr)
    {
        LuaException::Throw("Lua module loader is not configured");
    }

    lua_State* L = LuaEnv::GetState();
    LuaStackGuard stackGuard(L);
    if (lua_getglobal(L, "require") != LUA_TFUNCTION)
    {
        LuaException::Throw("Lua global 'require' is not available.");
    }

    lua_pushstring(L, moduleName);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK)
    {
        const char* err = lua_tostring(L, -1);
        std::string msg = std::string("Error requiring lua module '") + moduleName + "': " + (err ? err : "");
        LuaException::Throw(msg.c_str());
    }

    if (!lua_istable(L, -1))
    {
        std::string msg = std::string("Lua module '") + moduleName + "' must return a table.";
        LuaException::Throw(msg.c_str());
    }

    const int moduleRef = luaL_ref(L, LUA_REGISTRYINDEX);
    s_ModuleRefs[moduleName] = moduleRef;
    return moduleRef;
}

int LuaGlobalRefs::FindModuleFunctionRef(const char* moduleName, const char* methodName)
{
    const int moduleRef = GetModuleRef(moduleName);
    lua_State* L = LuaEnv::GetState();
    LuaStackGuard stackGuard(L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, moduleRef);
    lua_getfield(L, -1, methodName);
    if (!lua_isfunction(L, -1))
    {
        std::string msg = std::string("Lua function '") + moduleName + "." + methodName + "' not found.";
        LuaException::Throw(msg.c_str());
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

int LuaGlobalRefs::GetOrCreateModuleFunctionRef(const char* moduleName, const char* methodName)
{
    std::string key = std::string(moduleName) + "::" + methodName;
    auto it = s_ModuleFunctionRefs.find(key);
    if (it != s_ModuleFunctionRefs.end())
        return it->second;

    const int functionRef = FindModuleFunctionRef(moduleName, methodName);
    s_ModuleFunctionRefs[key] = functionRef;
    return functionRef;
}
} // namespace zlua
