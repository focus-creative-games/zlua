#include "StructOpaqueScope.h"

#include "marshal/ValueMarshaling.h"

#include "vm/Class.h"
#include "vm/Object.h"

#include <cstring>
#include <vector>

namespace zlua
{
    struct OpaqueEntry
    {
        Il2CppClass* klass = nullptr;
        std::vector<uint8_t> payload;
    };

    static uint32_t s_generation = 1;
    static int s_luaToCSharpDepth = 0;
    static std::vector<OpaqueEntry> s_entries;

    static void ResetScope()
    {
        s_entries.clear();
        s_generation++;
        if (s_generation == 0)
            s_generation = 1;
    }

    static intptr_t EncodeHandle(uint32_t generation, int index)
    {
        const uint64_t encoded = (static_cast<uint64_t>(generation) << 32) | static_cast<uint32_t>(index + 1);
        return static_cast<intptr_t>(encoded);
    }

    static bool DecodeHandle(intptr_t handleId, uint32_t& generationOut, int& indexOut)
    {
        generationOut = 0;
        indexOut = -1;
        if (handleId == 0)
            return false;

        const uint64_t encoded = static_cast<uint64_t>(handleId);
        generationOut = static_cast<uint32_t>(encoded >> 32);
        const uint32_t slot = static_cast<uint32_t>(encoded & 0xFFFFFFFFu);
        if (slot == 0)
            return false;

        indexOut = static_cast<int>(slot - 1);
        return true;
    }

    void StructOpaqueScope::EnterLuaToCSharp()
    {
        if (s_luaToCSharpDepth++ == 0)
            ResetScope();
    }

    void StructOpaqueScope::LeaveLuaToCSharp()
    {
        if (s_luaToCSharpDepth > 0)
            s_luaToCSharpDepth--;
    }

    void StructOpaqueScope::EnterStandaloneCSharpToLua()
    {
        if (s_luaToCSharpDepth == 0)
            ResetScope();
    }

    intptr_t StructOpaqueScope::RegisterStruct(Il2CppObject* boxedStruct, Il2CppClass* structClass)
    {
        if (boxedStruct == nullptr || structClass == nullptr)
            return 0;

        il2cpp::vm::Class::Init(structClass);
        if (!ValueMarshaling::IsStructClass(structClass))
            return 0;

        if (boxedStruct->klass != structClass)
            return 0;

        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(structClass);
        OpaqueEntry entry;
        entry.klass = structClass;
        entry.payload.resize(payloadSize);
        if (payloadSize > 0)
            std::memcpy(entry.payload.data(), il2cpp::vm::Object::Unbox(boxedStruct), payloadSize);

        const int index = static_cast<int>(s_entries.size());
        s_entries.push_back(std::move(entry));
        return EncodeHandle(s_generation, index);
    }

    bool StructOpaqueScope::TryResolveEntry(intptr_t handleId, Il2CppClass** klassOut, const uint8_t** payloadOut, size_t* payloadSizeOut)
    {
        if (klassOut != nullptr)
            *klassOut = nullptr;
        if (payloadOut != nullptr)
            *payloadOut = nullptr;
        if (payloadSizeOut != nullptr)
            *payloadSizeOut = 0;

        uint32_t generation = 0;
        int index = -1;
        if (!DecodeHandle(handleId, generation, index))
            return false;

        if (generation != s_generation || index < 0 || index >= static_cast<int>(s_entries.size()))
            return false;

        const OpaqueEntry& entry = s_entries[static_cast<size_t>(index)];
        if (entry.klass == nullptr)
            return false;

        if (klassOut != nullptr)
            *klassOut = entry.klass;
        if (payloadOut != nullptr)
            *payloadOut = entry.payload.data();
        if (payloadSizeOut != nullptr)
            *payloadSizeOut = entry.payload.size();
        return true;
    }
}
