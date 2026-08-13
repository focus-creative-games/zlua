#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
    class LuaAppDomain
    {
    public:
        /// Process-level bridges / metadata / icalls + first lua_State (Runtime.cpp).
        static void Initialize();

        /// Host Initialize: install loader; create state only if missing.
        static void InitializeFromManaged(Il2CppDelegate* moduleLoaderDelegate);

        /// Host Reset: teardown state, recreate, install loader.
        static void ResetFromManaged(Il2CppDelegate* moduleLoaderDelegate);

    private:
        static void InitializeProcessOnce();
        static void InitializeState();
        static void ShutdownState();
    };
}
