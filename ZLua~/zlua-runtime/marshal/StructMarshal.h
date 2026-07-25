#pragma once

#include "MarshalDefs.h"

namespace zlua
{

class StructMarshal
{
  public:
    static void PushValue(lua_State* L, void* dataAddr, Il2CppClass* klass, int metatableRefIndex);
    static inline void PushValue(lua_State* L, void* dataAddr, Il2CppClass* klass)
    {
        PushValue(L, dataAddr, klass, LUA_NOREF);
    }
    static void* PushZeroedValue(lua_State* L, Il2CppClass* klass);
    static void PopValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr);
    static void PushNullableValue(lua_State* L, void* dataAddr, Il2CppClass* klass, int metatableRefIndex);
    static void PopNullableValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr);

    static ByValUserDataHeader* GetByValHeader(lua_State* L, int index);

    static inline ByValUserDataHeader* GetByValHeaderUnchecked(lua_State* L, int index)
    {
        ByValUserDataHeader* header = (ByValUserDataHeader*)lua_touserdata(L, index);
        IL2CPP_ASSERT(header->header.kind == UserDataKind::ByVal);
        return header;
    }
};
} // namespace zlua
