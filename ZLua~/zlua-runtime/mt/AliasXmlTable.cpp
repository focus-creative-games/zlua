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
