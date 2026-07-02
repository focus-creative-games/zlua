#pragma once

#include "../ZLuaCommon.h"
#include "ObjectRegistry.h"
#include "MarshalDefs.h"

namespace zlua
{
class ObjectMarshal
{
  public:

    static void Push(lua_State* L, Il2CppObject* obj, const MarshalMetaInfo* meta);
    static void Push(lua_State* L, Il2CppObject* obj);
    static Il2CppObject* Pop(lua_State* L, int objIndex, Il2CppClass* klass);
    static Il2CppObject* PopNotDelegate(lua_State* L, int objIndex, Il2CppClass* klass);

    static inline Il2CppObject* PopByObjThis(lua_State* L, int objIndex)
    {
        IL2CPP_ASSERT(lua_type(L, objIndex) == LUA_TUSERDATA);
        return ObjectRegistry::PopThis(L, objIndex);
    }
};
} // namespace zlua
