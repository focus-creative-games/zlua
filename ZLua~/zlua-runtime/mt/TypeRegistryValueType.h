#pragma once

struct lua_State;
struct Il2CppClass;

namespace zlua
{
class TypeRegistryValueType
{
  public:
    static void CreateTypeTable(lua_State* L, Il2CppClass* klass);
    static void CreateNullableTypeTable(lua_State* L, Il2CppClass* klass);
    static void CreateEnumTypeTable(lua_State* L, Il2CppClass* klass);
};
} // namespace zlua
