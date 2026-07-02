#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;



namespace zlua

{

    // Registry refs for instance metatables, populated when TypeRegistry creates a type table.

    class MetaTableCache

    {

    public:

        static void RegisterByValMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex);

        static void RegisterByObjMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex);

        static void RegisterReferenceMetatable(lua_State* L, Il2CppClass* klass, int metatableIndex);



        static void PushByValMetatable(lua_State* L, Il2CppClass* klass);

        static bool TryPushByObjMetatable(lua_State* L, Il2CppClass* klass);

        static bool TryPushReferenceMetatable(lua_State* L, Il2CppClass* klass);

    };

}

