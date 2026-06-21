#pragma once

namespace novalua
{
    class LuaAppDomain
    {
    public:
        static void Initialize();
        static void InitializeFromManaged(void* moduleLoaderDelegate);
    };
}
