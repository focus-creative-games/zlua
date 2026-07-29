#pragma once

#include <cstddef>
#include <string>

struct MethodInfo;

namespace zlua
{
/// Name-based entry from AliasCodegen (tokens resolved at RegisterEntries / startup).
struct LuaAliasXmlEntry
{
    const char* assemblyName;
    const char* typeFullName;
    const char* methodName;
    const char* signature;
    const char* alias;
};

class AliasXmlTable
{
  public:
    static void Clear();
    /// Store entries and resolve all tokens immediately (Il2Cpp has loaded assemblies).
    static void RegisterEntries(const LuaAliasXmlEntry* entries, size_t count);

    /// Resolve XML alias for a method (Attribute aliases are read separately).
    static bool TryGetAlias(const MethodInfo* method, std::string& aliasOut);
};
} // namespace zlua
