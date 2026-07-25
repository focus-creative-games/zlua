#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
    class AssemblyRegistry
    {
    public:
        static void InitializeCSharpRoot(lua_State* L);
    };
}
