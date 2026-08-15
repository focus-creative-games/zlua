#include "StructRegistry.h"
#include "StructMarshal.h"

#include "../utils/MetadataUtil.h"

#include "gc/GarbageCollector.h"
#include "vm/Class.h"

#include <unordered_set>

namespace zlua
{

static std::unordered_set<ByValUserDataHeader*> s_byValUserDataSet;

void StructRegistry::Initialize(lua_State* L)
{
    (void)L;
    IL2CPP_ASSERT(s_byValUserDataSet.empty());
}

void StructRegistry::Shutdown(lua_State* L)
{
    (void)L;
    for (auto it = s_byValUserDataSet.begin(); it != s_byValUserDataSet.end(); ++it)
    {
        ByValUserDataHeader* header = *it;
        if (header == nullptr)
            continue;
        il2cpp::gc::GarbageCollector::UnregisterRoot(reinterpret_cast<char*>(header->Payload()));
    }
    s_byValUserDataSet.clear();
}

void StructRegistry::Register(ByValUserDataHeader* header)
{
    IL2CPP_ASSERT(header != nullptr);
    IL2CPP_ASSERT(header->header.kind == UserDataKind::ByVal);
    IL2CPP_ASSERT(header->klass != nullptr);
    // Intentional: each non-blittable ByVal payload needs its own GC root so embedded
    // references remain visible. Blittable structs must not reach here.
    IL2CPP_ASSERT(!header->klass->is_blittable);
    IL2CPP_ASSERT(s_byValUserDataSet.find(header) == s_byValUserDataSet.end());

    const size_t payloadSize = (size_t)MetadataUtil::GetInstanceSizeWithoutHeader(header->klass);
    il2cpp::gc::GarbageCollector::RegisterRoot(reinterpret_cast<char*>(header->Payload()), payloadSize);
    s_byValUserDataSet.insert(header);
}

void StructRegistry::Unregister(ByValUserDataHeader* header)
{
    IL2CPP_ASSERT(header != nullptr);
    if (s_byValUserDataSet.find(header) == s_byValUserDataSet.end())
        return;

    il2cpp::gc::GarbageCollector::UnregisterRoot(reinterpret_cast<char*>(header->Payload()));
    s_byValUserDataSet.erase(header);
}

int StructRegistry::OnReleaseByValUserData(lua_State* L)
{
    ByValUserDataHeader* userdata = StructMarshal::GetByValHeaderUnchecked(L, 1);
    IL2CPP_ASSERT(userdata != nullptr);
    IL2CPP_ASSERT(!userdata->klass->is_blittable);
    Unregister(userdata);
    return 0;
}

} // namespace zlua
