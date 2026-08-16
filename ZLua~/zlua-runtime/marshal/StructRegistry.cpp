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
