#pragma once

#include <cstdint>

namespace zlua
{
    enum class LuaMarshalType : uint8_t
    {
        Default = 0,
        UserData,
        Bytes,
        OpaqueLightUserData,
        UnpackedValues,
        Table,
        ParamsTable,
    };

    struct LuaMarshalMemberDesc
    {
        const char* clrName;
        bool optional;
    };

    struct LuaMarshalSlotDesc
    {
        LuaMarshalType marshalType;
        const LuaMarshalMemberDesc* members;
        uint16_t memberCount;
    };
}
