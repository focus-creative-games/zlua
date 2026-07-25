#pragma once

#include "MarshalDefs.h"
#include "../mt/MetaBinding.h"

namespace zlua
{
    class MethodOverloadResolver
    {

    public:
        static MethodOverloadResolutionResult Resolve(lua_State* L, const MethodGroups* groups, int32_t argStart, int32_t argCount);
    };
}