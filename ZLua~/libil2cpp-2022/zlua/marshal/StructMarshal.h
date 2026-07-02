#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;

namespace zlua
{
    struct ByValUserDataHeader
    {
        Il2CppClass* klass;

        uint8_t* Payload()
        {
            return reinterpret_cast<uint8_t*>(this + 1);
        }

        const uint8_t* Payload() const
        {
            return reinterpret_cast<const uint8_t*>(this + 1);
        }
    };

    class StructMarshal
    {
    public:
        static void PushValue(lua_State* L, void* dataAddr, Il2CppClass* klass);
        static bool PopValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr);

        static bool IsByValUserData(lua_State* L, int index);
        static ByValUserDataHeader* GetByValHeader(lua_State* L, int index);
        static size_t GetByValUserDataSize(Il2CppClass* klass);

        static bool CanPopFromTable(lua_State* L, int index, Il2CppClass* klass);
        static bool TryPopFromTable(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr);
    };
}
