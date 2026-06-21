#pragma once

#include "NovaLuaCommon.h"

struct Il2CppObject;

namespace novalua
{
    class ObjectRegistry
    {
    public:
        static void PushObject(lua_State* L, Il2CppObject* obj);
        static Il2CppObject* GetObject(lua_State* L, int idx);
        static void ReleaseObject(lua_State* L, int idx);
    };
}
