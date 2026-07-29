#include "AliasXmlTable.h"

#include "../utils/MetadataUtil.h"
#include "../utils/XmlBindingUtil.h"

#include "vm/Class.h"

#include <string>
#include <unordered_map>

namespace zlua
{
namespace
{
std::unordered_map<const Il2CppImage*, std::unordered_map<uint32_t, std::string>>& BoundByImage()
{
    static std::unordered_map<const Il2CppImage*, std::unordered_map<uint32_t, std::string>> s_maps;
    return s_maps;
}

void TryBindOneEntry(const LuaAliasXmlEntry& e)
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

    const char* signature = e.signature != nullptr ? e.signature : "()";
    const MethodInfo* method = MetadataUtil::FindMethodByParameterSignature(klass, e.methodName, signature);
    if (method == nullptr || e.alias == nullptr || e.alias[0] == '\0')
        return;

    BoundByImage()[assembly->image][method->token] = e.alias;
}
} // namespace

void AliasXmlTable::Clear()
{
    BoundByImage().clear();
}

void AliasXmlTable::RegisterEntries(const LuaAliasXmlEntry* entries, size_t count)
{
    Clear();
    if (count == 0)
        return;

    for (size_t i = 0; i < count; ++i)
        TryBindOneEntry(entries[i]);
}

bool AliasXmlTable::TryGetAlias(const MethodInfo* method, std::string& aliasOut)
{
    aliasOut.clear();
    method = XmlBindingUtil::NormalizeMethodForToken(method);

    auto imageIt = BoundByImage().find(method->klass->image);
    if (imageIt == BoundByImage().end())
        return false;

    auto it = imageIt->second.find(method->token);
    if (it == imageIt->second.end())
        return false;

    aliasOut = it->second;
    return !aliasOut.empty();
}
} // namespace zlua
