#pragma once

namespace zlua
{
class LuaGlobalRefs
{
  public:
    static int GetModuleRef(const char* moduleName);
    static int GetOrCreateModuleFunctionRef(const char* moduleName, const char* methodName);
    static void Clear();

  private:
    static int FindModuleFunctionRef(const char* moduleName, const char* methodName);
};
} // namespace zlua
