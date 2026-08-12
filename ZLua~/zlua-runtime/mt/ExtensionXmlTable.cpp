#include "ExtensionXmlTable.h"

#include "../utils/MetadataUtil.h"

#include "vm/Class.h"
#include "vm/Exception.h"

#include <string>
#include <unordered_map>

namespace zlua
{
namespace
{
std::unordered_map<Il2CppClass*, std::vector<Il2CppClass*>>& BoundByTarget()
{
    static std::unordered_map<Il2CppClass*, std::vector<Il2CppClass*>> s_map;
    return s_map;
}

[[noreturn]] void FailHard(const std::string& message)
{
    il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetInvalidOperationException(message.c_str()));
}

Il2CppClass* ResolveRequiredType(const char* assemblyName, const char* typeFullName, const char* role)
{
    if (assemblyName == nullptr || assemblyName[0] == '\0' || typeFullName == nullptr || typeFullName[0] == '\0')
    {
        FailHard(std::string("[ZLua] LuaExtension XML ") + role + " has empty assembly/type name");
    }

    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
    if (assembly == nullptr)
    {
        FailHard(std::string("[ZLua] LuaExtension XML ") + role + " Assembly '" + assemblyName + "' not loaded");
    }

    Il2CppClass* klass = MetadataUtil::ResolveType(assembly, typeFullName);
    if (klass == nullptr)
    {
        FailHard(std::string("[ZLua] LuaExtension XML ") + role + " type '" + typeFullName + "' not found in assembly '" +
                 assemblyName + "'");
    }

    il2cpp::vm::Class::Init(klass);
    return klass;
}

void BindOneEntry(const LuaExtensionXmlEntry& e)
{
    Il2CppClass* target = ResolveRequiredType(e.targetAssemblyName, e.targetTypeFullName, "target");
    Il2CppClass* extension = ResolveRequiredType(e.extensionAssemblyName, e.extensionTypeFullName, "extension");

    std::vector<Il2CppClass*>& list = BoundByTarget()[target];
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (list[i] == extension)
            return;
    }
    list.push_back(extension);
}
} // namespace

void ExtensionXmlTable::Clear()
{
    BoundByTarget().clear();
}

void ExtensionXmlTable::RegisterEntries(const LuaExtensionXmlEntry* entries, size_t count)
{
    Clear();
    if (entries == nullptr || count == 0)
        return;

    for (size_t i = 0; i < count; ++i)
        BindOneEntry(entries[i]);
}

bool ExtensionXmlTable::TryGetExtensionClasses(Il2CppClass* targetKlass, std::vector<Il2CppClass*>& outClasses)
{
    outClasses.clear();
    if (targetKlass == nullptr)
        return false;

    auto it = BoundByTarget().find(targetKlass);
    if (it == BoundByTarget().end() || it->second.empty())
        return false;

    outClasses = it->second;
    return true;
}
} // namespace zlua
