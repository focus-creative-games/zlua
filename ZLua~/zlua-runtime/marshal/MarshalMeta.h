#pragma once

#include "MarshalDefs.h"

namespace zlua
{


class MarshalMeta
{
  public:
    static MarshalMetaInfo* Create(lua_State* L, const MethodInfo* method, int argIndex /* -1 for return type */);
    static MarshalMetaInfo* Create(lua_State* L, const FieldInfo* field);
    static MarshalMetaInfo* Create(lua_State* L, const PropertyInfo* property);

    static inline int EnsureByValMetatableRef(lua_State* L, const MarshalMetaInfo* meta)
    {
        MarshalMetaInfo* mutableMeta = const_cast<MarshalMetaInfo*>(meta);
        if (mutableMeta->luaByValRefIndex != LUA_NOREF)
            return mutableMeta->luaByValRefIndex;
        return EnsureByValMetatableRefSlow(L, mutableMeta);
    }

    static inline int EnsureByObjMetatableRef(lua_State* L, const MarshalMetaInfo* meta)
    {
        MarshalMetaInfo* mutableMeta = const_cast<MarshalMetaInfo*>(meta);
        if (mutableMeta->luaByObjRefIndex != LUA_NOREF)
            return mutableMeta->luaByObjRefIndex;
        return EnsureByObjMetatableRefSlow(L, mutableMeta);
    }

  private:
    static int EnsureByValMetatableRefSlow(lua_State* L, MarshalMetaInfo* meta);
    static int EnsureByObjMetatableRefSlow(lua_State* L, MarshalMetaInfo* meta);
};
} // namespace zlua
