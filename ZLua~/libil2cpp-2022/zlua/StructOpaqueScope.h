#pragma once

#include <cstddef>
#include <cstdint>

struct Il2CppClass;
struct Il2CppObject;

namespace zlua
{
    class StructOpaqueScope
    {
    public:
        static void EnterLuaToCSharp();
        static void LeaveLuaToCSharp();
        static void EnterStandaloneCSharpToLua();

        static intptr_t RegisterStruct(Il2CppObject* boxedStruct, Il2CppClass* structClass);
        static bool TryResolveEntry(intptr_t handleId, Il2CppClass** klassOut, const uint8_t** payloadOut, size_t* payloadSizeOut);
    };
}
