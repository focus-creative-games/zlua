#pragma once

namespace zlua
{
    class LuaInteropManager
    {
    public:
        static void EnsureCSharpRoot();
        static void RegisterZLuaApi();
    };
}
