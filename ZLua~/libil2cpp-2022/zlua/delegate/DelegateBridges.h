#pragma once

struct Il2CppClass;

namespace zlua
{
    class DelegateBridges
    {
    public:
        static void* Resolve(Il2CppClass* delegateClass);
    };
}
