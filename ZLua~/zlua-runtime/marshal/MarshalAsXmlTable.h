#pragma once

#include "MarshalDefs.h"

#include <cstdint>
#include <string>
#include <vector>

struct MethodInfo;
struct Il2CppClass;
struct Il2CppImage;

namespace zlua
{
enum class LuaMarshalAsXmlKind : uint8_t
{
    Type = 0,
    Field = 1,
    Property = 2,
    Param = 3,
    Return = 4,
};

/// Name-based entry from MarshalAsCodegen (tokens resolved at RegisterEntries / startup).
struct LuaMarshalAsXmlEntry
{
    LuaMarshalAsXmlKind kind;
    const char* assemblyName;
    const char* typeFullName;
    const char* memberOrMethodName; // Field/Property/Method name; nullptr for Type
    const char* signature;          // Method signature "(T1,T2)"; nullptr unless Param/Return
    int32_t paramIndex;             // Param index; unused otherwise (-1)
    LuaMarshalType marshalType;
    const char* const* members;
    uint16_t memberCount;
};

struct LuaMarshalAsResolvedData
{
    LuaMarshalType marshalType = LuaMarshalType::Default;
    std::vector<std::string> members;
};

class MarshalAsXmlTable
{
  public:
    static void Clear();
    /// Store entries and resolve all tokens immediately (Il2Cpp has loaded assemblies).
    static void RegisterEntries(const LuaMarshalAsXmlEntry* entries, size_t count);

    /// Type / Field / Property: image → memberToken → Rule
    static bool TryGet(const Il2CppImage* image, uint32_t token, LuaMarshalAsResolvedData& outData);

    /// Param (index>=0) / Return (index=-1): image → (methodDefToken, index) → Rule
    static bool TryGetForMethodSlot(const MethodInfo* method, int argIndex, LuaMarshalAsResolvedData& outData);

    static bool TryGetForType(Il2CppClass* klass, LuaMarshalAsResolvedData& outData);
};
} // namespace zlua
