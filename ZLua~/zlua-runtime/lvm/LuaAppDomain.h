#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
    class LuaAppDomain
    {
    public:
        static void Initialize();
        static void InitializeFromManaged(Il2CppDelegate* moduleLoaderDelegate);
    };
}
