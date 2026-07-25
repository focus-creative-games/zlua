#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;

namespace zlua
{
// Fast klass -> instance metatable cache (lightuserdata key) for byval / byobj.
class MetaTableCache
{
  public:
    static void Initialize(lua_State* L);
    static void Shutdown(lua_State* L);

    static int GetOrCreateByValMetatableRef(lua_State* L, Il2CppClass* klass);
    static int GetOrCreateByObjMetatableRef(lua_State* L, Il2CppClass* klass);

    static int PushByValMetatable(lua_State* L, Il2CppClass* klass);
    static int PushByObjMetatable(lua_State* L, Il2CppClass* klass);
};

} // namespace zlua
