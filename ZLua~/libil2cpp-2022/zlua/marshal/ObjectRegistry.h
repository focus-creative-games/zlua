#pragma once

#include "../ZLuaCommon.h"
#include "MarshalDefs.h"

namespace zlua
{
class ObjectRegistry
{
  public:
    static void Initialize(lua_State* L);
    static void Shutdown(lua_State* L);

    static void Push(lua_State* L, Il2CppObject* obj, int metatableRefIndex);
    static Il2CppObject* Pop(lua_State* L, int idx);

    static inline Il2CppObject* PopThis(lua_State* L, int idx)
    {
        ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, idx);
        IL2CPP_ASSERT(ud != nullptr);
        IL2CPP_ASSERT(ud->header.kind == UserDataKind::ByObj);
        return ud->obj;
    }

    static int OnReleaseObjectUserData(lua_State* L);
};
} // namespace zlua
