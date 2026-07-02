#pragma once

#include "../ZLuaCommon.h"

struct Il2CppString;

namespace zlua
{
    class StringMarshal
    {
    public:
        static void Push(lua_State* L, Il2CppString* str);
        static Il2CppString* Pop(lua_State* L, int idx);

        static bool CanConvert(lua_State* L, int index);
        static bool TryPop(lua_State* L, int index, Il2CppString** dest);
    };
}
