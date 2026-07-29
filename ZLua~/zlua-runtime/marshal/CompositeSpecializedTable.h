#pragma once

#include "MarshalDefs.h"

namespace zlua
{

struct CompositeSpecializedEntry
{
    const char* assemblyName;
    const char* typeFullName;
    LuaMarshalType marshalType;
    FnMarshalLua2Cs lua2csWriter;
    FnMarshalCs2Lua cs2luaWriter;
    uint16_t stackSlots;
};

class CompositeSpecializedTable
{
  public:
    static void Clear();
    static void RegisterEntries(const CompositeSpecializedEntry* entries, size_t count);

    static bool TryGet(
        Il2CppClass* klass,
        LuaMarshalType marshalType,
        FnMarshalLua2Cs* outLua2Cs,
        FnMarshalCs2Lua* outCs2Lua,
        uint16_t* outStackSlots);
};

} // namespace zlua
