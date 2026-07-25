#include "MarshalAsXmlTable.h"

#include "../utils/MetadataUtil.h"

#include "il2cpp-class-internals.h"
#include "metadata/GenericMetadata.h"
#include "vm/Class.h"
#include "vm/GenericClass.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

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

std::vector<LuaMarshalAsXmlEntry>& RawEntries()
{
    static std::vector<LuaMarshalAsXmlEntry> s_entries;
    return s_entries;
}

/// Presence of key means this image was already bound (maps may be empty).
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

bool AssemblyNameEquals(const Il2CppImage* image, const char* assemblyName)
{
    if (image == nullptr || assemblyName == nullptr)
        return false;

    if (image->nameNoExt != nullptr && std::strcmp(image->nameNoExt, assemblyName) == 0)
        return true;

    if (image->assembly != nullptr && image->assembly->aname.name != nullptr
        && std::strcmp(image->assembly->aname.name, assemblyName) == 0)
        return true;

    return false;
}

bool IsDeterminedMarshalTargetType(const Il2CppType* type)
{
    if (type == nullptr)
        return false;
    if (il2cpp::metadata::GenericMetadata::ContainsGenericParameters(type))
        return false;

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (klass == nullptr)
        return false;

    il2cpp::vm::Class::Init(klass);
    if (klass->nullabletype)
        klass = klass->element_class;
    if (klass->is_generic)
        return false;
    return !il2cpp::metadata::GenericMetadata::ContainsGenericParameters(klass);
}

const MethodInfo* NormalizeMethodForToken(const MethodInfo* method)
{
    if (method == nullptr)
        return nullptr;

    if (method->is_inflated && method->genericMethod != nullptr && method->genericMethod->methodDefinition != nullptr)
        return method->genericMethod->methodDefinition;

    if (method->klass != nullptr && method->klass->generic_class != nullptr)
    {
        Il2CppClass* typeDef = il2cpp::vm::GenericClass::GetTypeDefinition(method->klass->generic_class);
        il2cpp::vm::Class::Init(typeDef);
        void* iter = nullptr;
        while (const MethodInfo* m = il2cpp::vm::Class::GetMethods(typeDef, &iter))
        {
            if (m->token == method->token)
                return m;
        }
    }

    return method;
}

/// Runtime bind: skip invalid entries silently (Generate already validated).
bool TryBindOneEntry(const LuaMarshalAsXmlEntry& e, const Il2CppImage* targetImage, BoundMaps& maps)
{
    if (!AssemblyNameEquals(targetImage, e.assemblyName))
        return false;

    const Il2CppAssembly* assembly = targetImage->assembly;
    if (assembly == nullptr)
        return false;

    Il2CppClass* klass = MetadataUtil::ResolveType(assembly, e.typeFullName);
    if (klass == nullptr)
        return false;

    il2cpp::vm::Class::Init(klass);
    if (klass->generic_class != nullptr)
        return false;

    LuaMarshalAsResolvedData data = MakeData(e);

    switch (e.kind)
    {
    case LuaMarshalAsXmlKind::Type:
        if (klass->is_generic)
            return false;
        maps.byMemberToken[klass->token] = std::move(data);
        return true;
    case LuaMarshalAsXmlKind::Field:
    {
        const FieldInfo* field = il2cpp::vm::Class::GetFieldFromName(klass, e.memberOrMethodName);
        if (field == nullptr || !IsDeterminedMarshalTargetType(field->type))
            return false;
        maps.byMemberToken[field->token] = std::move(data);
        return true;
    }
    case LuaMarshalAsXmlKind::Property:
    {
        const PropertyInfo* property = il2cpp::vm::Class::GetPropertyFromName(klass, e.memberOrMethodName);
        if (property == nullptr)
            return false;
        const Il2CppType* propType = property->get != nullptr ? property->get->return_type
                                                              : (property->set != nullptr ? property->set->parameters[0] : nullptr);
        if (!IsDeterminedMarshalTargetType(propType))
            return false;
        maps.byMemberToken[property->token] = std::move(data);
        return true;
    }
    case LuaMarshalAsXmlKind::Param:
    case LuaMarshalAsXmlKind::Return:
    {
        const char* signature = e.signature != nullptr ? e.signature : "()";
        const MethodInfo* method = MetadataUtil::FindMethodByParameterSignature(
            klass, e.memberOrMethodName, signature, /*isStatic*/ true);
        if (method == nullptr)
        {
            method = MetadataUtil::FindMethodByParameterSignature(
                klass, e.memberOrMethodName, signature, /*isStatic*/ false);
        }
        if (method == nullptr)
            return false;

        if (e.kind == LuaMarshalAsXmlKind::Param)
        {
            if (e.paramIndex < 0 || e.paramIndex >= method->parameters_count)
                return false;
            if (!IsDeterminedMarshalTargetType(method->parameters[e.paramIndex]))
                return false;
            maps.byMethodSlot[{method->token, e.paramIndex}] = std::move(data);
            return true;
        }

        if (!IsDeterminedMarshalTargetType(method->return_type))
            return false;
        maps.byMethodSlot[{method->token, -1}] = std::move(data);
        return true;
    }
    default:
        return false;
    }
}

BoundMaps& EnsureBound(const Il2CppImage* image)
{
    auto it = BoundByImage().find(image);
    if (it != BoundByImage().end())
        return it->second;

    BoundMaps maps;
    const std::vector<LuaMarshalAsXmlEntry>& entries = RawEntries();
    for (size_t i = 0; i < entries.size(); ++i)
        TryBindOneEntry(entries[i], image, maps);

    return BoundByImage().emplace(image, std::move(maps)).first->second;
}
} // namespace

void MarshalAsXmlTable::Clear()
{
    RawEntries().clear();
    BoundByImage().clear();
}

void MarshalAsXmlTable::RegisterEntries(const LuaMarshalAsXmlEntry* entries, size_t count)
{
    Clear();
    if (entries == nullptr || count == 0)
        return;

    RawEntries().assign(entries, entries + count);
}

bool MarshalAsXmlTable::TryGet(const Il2CppImage* image, uint32_t token, LuaMarshalAsResolvedData& outData)
{
    if (image == nullptr || token == 0)
        return false;

    BoundMaps& maps = EnsureBound(image);
    auto it = maps.byMemberToken.find(token);
    if (it == maps.byMemberToken.end())
        return false;

    outData = it->second;
    return true;
}

bool MarshalAsXmlTable::TryGetForMethodSlot(const MethodInfo* method, int argIndex, LuaMarshalAsResolvedData& outData)
{
    const MethodInfo* keyMethod = NormalizeMethodForToken(method);
    if (keyMethod == nullptr || keyMethod->klass == nullptr || keyMethod->token == 0)
        return false;

    BoundMaps& maps = EnsureBound(keyMethod->klass->image);
    auto it = maps.byMethodSlot.find({keyMethod->token, argIndex});
    if (it == maps.byMethodSlot.end())
        return false;

    outData = it->second;
    return true;
}

bool MarshalAsXmlTable::TryGetForType(Il2CppClass* klass, LuaMarshalAsResolvedData& outData)
{
    if (klass == nullptr)
        return false;

    il2cpp::vm::Class::Init(klass);
    if (klass->nullabletype)
        klass = klass->element_class;

    if (klass->is_generic || klass->generic_class != nullptr)
        return false;

    return TryGet(klass->image, klass->token, outData);
}
} // namespace zlua
