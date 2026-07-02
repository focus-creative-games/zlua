#pragma once

#include "il2cpp-object-internals.h"
#include "../ZLuaCommon.h"

namespace zlua
{
    struct LuaMethodTarget : Il2CppObject
    {
        bool disposed;
        lua_State* L;
        int32_t funcRef;
    };
}
