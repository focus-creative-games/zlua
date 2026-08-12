#pragma once

#include <cstddef>
#include <vector>

struct Il2CppClass;

namespace zlua
{
/// Name-based entry from ExtensionCodegen (classes resolved at RegisterEntries / startup).
struct LuaExtensionXmlEntry
{
    const char* targetAssemblyName;
    const char* targetTypeFullName;
    const char* extensionAssemblyName;
    const char* extensionTypeFullName;
};

class ExtensionXmlTable
{
  public:
    static void Clear();
    /// Resolve all entries immediately. Missing assembly/type → hard failure (spec 13).
    static void RegisterEntries(const LuaExtensionXmlEntry* entries, size_t count);

    /// Extension classes configured via XML for targetKlass (exact type only).
    static bool TryGetExtensionClasses(Il2CppClass* targetKlass, std::vector<Il2CppClass*>& outClasses);
};
} // namespace zlua
