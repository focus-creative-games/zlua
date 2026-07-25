#pragma once

#include "../ZLuaCommon.h"
#include "MarshalDefs.h"

namespace zlua
{
class StructRegistry
{
  public:
    static void Initialize(lua_State* L);
    static void Shutdown(lua_State* L);

    // Track a non-blittable ByVal userdata payload so GC can scan embedded references.
    static void Register(ByValUserDataHeader* header);
    static void Unregister(ByValUserDataHeader* header);

    static int OnReleaseByValUserData(lua_State* L);
};
} // namespace zlua
