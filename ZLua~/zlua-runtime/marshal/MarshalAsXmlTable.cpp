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

#include "MarshalAsXmlTable.h"

#include "../utils/MetadataUtil.h"
#include "../utils/XmlBindingUtil.h"

#include "il2cpp-class-internals.h"
#include "metadata/GenericMetadata.h"
#include "vm/Class.h"

#include <string>
#include <unordered_map>

namespace zlua
{
namespace
{
struct MethodSlotKey
{
    uint32_t methodToken;
    int32_t index; // >=0 param; -1 return

    bool operator==(const MethodSlotKey& other) const
    {
        return methodToken == other.methodToken && index == other.index;
    }
};

struct MethodSlotKeyHash
{
    size_t operator()(const MethodSlotKey& key) const
    {
        return (static_cast<size_t>(key.methodToken) * 397u) ^ static_cast<size_t>(key.index);
    }
};

struct BoundMaps
{
    std::unordered_map<uint32_t, LuaMarshalAsResolvedData> byMemberToken;
    std::unordered_map<MethodSlotKey, LuaMarshalAsResolvedData, MethodSlotKeyHash> byMethodSlot;
};

std::unordered_map<const Il2CppImage*, BoundMaps>& BoundByImage()
{
    static std::unordered_map<const Il2CppImage*, BoundMaps> s_maps;
    return s_maps;
}

LuaMarshalAsResolvedData MakeData(const LuaMarshalAsXmlEntry& e)
{
    LuaMarshalAsResolvedData data;
    data.marshalType = e.marshalType;
    data.members.reserve(e.memberCount);
    for (uint16_t m = 0; m < e.memberCount; ++m)
    {
        if (e.members != nullptr && e.members[m] != nullptr)
            data.members.emplace_back(e.members[m]);
    }
    return data;
}

bool IsDeterminedMarshalTargetType(const Il2CppType* type)
{
    return !il2cpp::metadata::GenericMetadata::ContainsGenericParameters(type);
}

/// Runtime bind: skip invalid entries silently (Generate already validated).
void TryBindOneEntry(const LuaMarshalAsXmlEntry& e)
{
    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(e.assemblyName);
    if (assembly == nullptr)
        return;

    Il2CppClass* klass = MetadataUtil::ResolveType(assembly, e.typeFullName);
    if (klass == nullptr)
        return;

    il2cpp::vm::Class::Init(klass);
    if (klass->generic_class != nullptr)
        return;

    LuaMarshalAsResolvedData data = MakeData(e);
    BoundMaps& maps = BoundByImage()[assembly->image];

    switch (e.kind)
    {
    case LuaMarshalAsXmlKind::Type:
        if (klass->is_generic)
            return;
        maps.byMemberToken[klass->token] = std::move(data);
        return;
    case LuaMarshalAsXmlKind::Field:
    {
        const FieldInfo* field = il2cpp::vm::Class::GetFieldFromName(klass, e.memberOrMethodName);
        if (field == nullptr || !IsDeterminedMarshalTargetType(field->type))
            return;
        maps.byMemberToken[field->token] = std::move(data);
        return;
    }
    case LuaMarshalAsXmlKind::Property:
    {
        const PropertyInfo* property = il2cpp::vm::Class::GetPropertyFromName(klass, e.memberOrMethodName);
        if (property == nullptr)
            return;
        const Il2CppType* propType = property->get != nullptr ? property->get->return_type
                                                              : (property->set != nullptr ? property->set->parameters[0] : nullptr);
        if (!IsDeterminedMarshalTargetType(propType))
            return;
        maps.byMemberToken[property->token] = std::move(data);
        return;
    }
    case LuaMarshalAsXmlKind::Param:
    case LuaMarshalAsXmlKind::Return:
    {
        const char* signature = e.signature != nullptr ? e.signature : "()";
        const MethodInfo* method =
            MetadataUtil::FindMethodByParameterSignature(klass, e.memberOrMethodName, signature);
        if (method == nullptr)
            return;

        if (e.kind == LuaMarshalAsXmlKind::Param)
        {
            if (e.paramIndex < 0 || e.paramIndex >= method->parameters_count)
                return;
            if (!IsDeterminedMarshalTargetType(method->parameters[e.paramIndex]))
                return;
            maps.byMethodSlot[{method->token, e.paramIndex}] = std::move(data);
            return;
        }

        if (!IsDeterminedMarshalTargetType(method->return_type))
            return;
        maps.byMethodSlot[{method->token, -1}] = std::move(data);
        return;
    }
    default:
        return;
    }
}
} // namespace

void MarshalAsXmlTable::Clear()
{
    BoundByImage().clear();
}

void MarshalAsXmlTable::RegisterEntries(const LuaMarshalAsXmlEntry* entries, size_t count)
{
    Clear();
    if (count == 0)
        return;

    for (size_t i = 0; i < count; ++i)
        TryBindOneEntry(entries[i]);
}

bool MarshalAsXmlTable::TryGet(const Il2CppImage* image, uint32_t token, LuaMarshalAsResolvedData& outData)
{
    if (token == 0)
        return false;

    auto imageIt = BoundByImage().find(image);
    if (imageIt == BoundByImage().end())
        return false;

    auto it = imageIt->second.byMemberToken.find(token);
    if (it == imageIt->second.byMemberToken.end())
        return false;

    outData = it->second;
    return true;
}

bool MarshalAsXmlTable::TryGetForMethodSlot(const MethodInfo* method, int argIndex, LuaMarshalAsResolvedData& outData)
{
    const MethodInfo* keyMethod = XmlBindingUtil::NormalizeMethodForToken(method);
    if (keyMethod->token == 0)
        return false;

    auto imageIt = BoundByImage().find(keyMethod->klass->image);
    if (imageIt == BoundByImage().end())
        return false;

    auto it = imageIt->second.byMethodSlot.find({keyMethod->token, argIndex});
    if (it == imageIt->second.byMethodSlot.end())
        return false;

    outData = it->second;
    return true;
}

bool MarshalAsXmlTable::TryGetForType(Il2CppClass* klass, LuaMarshalAsResolvedData& outData)
{
    il2cpp::vm::Class::Init(klass);
    if (klass->nullabletype)
        klass = klass->element_class;

    if (klass->is_generic || klass->generic_class != nullptr)
        return false;

    return TryGet(klass->image, klass->token, outData);
}
} // namespace zlua
