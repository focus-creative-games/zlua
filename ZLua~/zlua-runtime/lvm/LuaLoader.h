#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class LuaLoader
{
  public:
    static void RegisterRoots();
    static void SetModuleLoader(Il2CppDelegate* moduleLoader);
    static Il2CppDelegate* GetModuleLoader();
    static void InstallHooks();
    static void Clear();
};
} // namespace zlua
