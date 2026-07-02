#pragma once

struct lua_State;
struct Il2CppClass;

namespace zlua
{
class TypeRegistryReference
{
  public:
    static void CreateTypeTable(lua_State* L, Il2CppClass* klass);
};
} // namespace zlua
