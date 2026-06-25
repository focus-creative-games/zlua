#pragma once

namespace zlua
{
    class LuaAppDomain
    {
    public:
        static void Initialize();
        static void InitializeFromManaged(void* moduleLoaderDelegate);
    };
}
