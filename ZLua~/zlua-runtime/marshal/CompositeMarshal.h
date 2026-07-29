#pragma once

#include "MarshalDefs.h"

namespace zlua
{

class CompositeMarshal
{
  public:
    static void PopTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void PushTable(lua_State* L, void* address, const MarshalMetaInfo* meta);
    static void PopUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void PushUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta);

    static void Lua2CSMarshalTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void CS2LuaMarshalTable(lua_State* L, void* address, const MarshalMetaInfo* meta);
    static void Lua2CSMarshalUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void CS2LuaMarshalUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta);
};

} // namespace zlua
