#pragma once

#include "MarshalAsTypes.h"

struct MethodInfo;

namespace zlua
{
    class MarshalAsRegistry
    {
    public:
        static void RegisterGeneratedTables();
        static LuaMarshalType GetParameterMarshalType(const MethodInfo* method, int paramIndex);
        static const LuaMarshalSlotDesc* GetParameterSlotDesc(const MethodInfo* method, int paramIndex);
        static LuaMarshalType GetReturnMarshalType(const MethodInfo* method);
        static const LuaMarshalSlotDesc* GetReturnSlotDesc(const MethodInfo* method);
    };
}
