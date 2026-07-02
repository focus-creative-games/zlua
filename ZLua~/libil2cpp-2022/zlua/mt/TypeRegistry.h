#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;

namespace zlua
{
    class TypeRegistry
    {
    public:
        static void PushTypeTable(lua_State* L, Il2CppClass* klass);
        static void PushInternedTypeTable(lua_State* L, Il2CppClass* klass);
        static Il2CppClass* GetClassFromTypeTable(lua_State* L, int index);
    };
}
