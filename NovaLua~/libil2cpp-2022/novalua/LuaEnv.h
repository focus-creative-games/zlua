#pragma once

#include <string>

#include "NovaLuaCommon.h"

struct Il2CppDelegate;
struct Il2CppString;

namespace novalua
{
    class LuaEnv
    {
    public:
        static void Create(Il2CppDelegate* moduleLoader);
        static void Shutdown();

        static lua_State* GetState();
        static Il2CppDelegate* GetModuleLoader();

        static void EnsureModuleLoaded(const char* moduleName);
        static int GetModuleRef(const char* moduleName);
        static int GetOrCreateModuleFunctionRef(const char* moduleName, const char* methodName);

        static void DoStringIgnoreResult(const char* chunk);
        static void RegisterPrintCallback();

        static void RaiseLuaException(const char* msg);
    private:
        static std::string LoadModuleSource(const char* moduleName);
        static int FindModuleFunctionRef(const char* moduleName, const char* methodName);
    };
}
