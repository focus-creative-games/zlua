#pragma once

#include "../ZLuaCommon.h"

struct Il2CppObject;

namespace zlua
{
    class ObjectMarshal
    {
    public:
        static void EnsureObjectCache(lua_State* L);
        static void Shutdown();

        static void Push(lua_State* L, Il2CppObject* obj);
        static Il2CppObject* Pop(lua_State* L, int idx);
        static void Release(lua_State* L, int idx);
    };
}
