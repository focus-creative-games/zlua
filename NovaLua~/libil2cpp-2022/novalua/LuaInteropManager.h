#pragma once

namespace novalua
{
    class LuaInteropManager
    {
    public:
        static void EnsureCSharpRoot();
        static void RegisterNovaLuaApi();
    };
}
