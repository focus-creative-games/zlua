// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
