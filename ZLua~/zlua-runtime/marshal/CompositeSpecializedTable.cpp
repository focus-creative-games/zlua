#include "CompositeSpecializedTable.h"

#include "../utils/MetadataUtil.h"

#include "vm/Class.h"

#include <unordered_map>

namespace zlua
{
namespace
{

struct SpecializedWriters
{
    LuaMarshalType marshalType;
    FnMarshalLua2Cs lua2csWriter;
    FnMarshalCs2Lua cs2luaWriter;
    uint16_t stackSlots;
};

std::unordered_map<const Il2CppClass*, SpecializedWriters> s_table;

} // namespace

void CompositeSpecializedTable::Clear()
{
    s_table.clear();
}

void CompositeSpecializedTable::RegisterEntries(const CompositeSpecializedEntry* entries, size_t count)
{
    Clear();
    if (entries == nullptr || count == 0)
        return;

    for (size_t i = 0; i < count; ++i)
    {
        const CompositeSpecializedEntry& e = entries[i];
        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(e.assemblyName);
        if (assembly == nullptr)
            continue;

        Il2CppClass* klass = MetadataUtil::ResolveType(assembly, e.typeFullName);
        if (klass == nullptr)
            continue;

        il2cpp::vm::Class::Init(klass);
        SpecializedWriters writers{e.marshalType, e.lua2csWriter, e.cs2luaWriter, e.stackSlots};
        s_table.insert({klass, writers});
    }
}

bool CompositeSpecializedTable::TryGet(
    Il2CppClass* klass,
    LuaMarshalType marshalType,
    FnMarshalLua2Cs* outLua2Cs,
    FnMarshalCs2Lua* outCs2Lua,
    uint16_t* outStackSlots)
{
    if (klass == nullptr)
        return false;

    auto it = s_table.find(klass);
    if (it == s_table.end())
        return false;
    if (it->second.marshalType != marshalType)
        return false;

    if (outLua2Cs != nullptr)
        *outLua2Cs = it->second.lua2csWriter;
    if (outCs2Lua != nullptr)
        *outCs2Lua = it->second.cs2luaWriter;
    if (outStackSlots != nullptr)
        *outStackSlots = it->second.stackSlots;
    return true;
}

} // namespace zlua
