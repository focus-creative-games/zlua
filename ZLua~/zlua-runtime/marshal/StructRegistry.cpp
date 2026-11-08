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
#include "../utils/LuaException.h"

#include <new>
#include <stdexcept>

namespace zlua
{
namespace
{
// Explicitly cleared on domain shutdown. A process-lifetime instance avoids
// invoking GC frees from a static destructor after IL2CPP's GC has shut down.
ByValPayloadPool* const s_pool = new ByValPayloadPool();
constexpr size_t kPressureBytes = 64 * 1024;
size_t s_callDepth = 0;
size_t s_pressure = 0;
bool s_closing = false;

ByValPayloadPool::Handle* Storage(ByValUserDataHeader* header)
{
    return reinterpret_cast<ByValPayloadPool::Handle*>(header->InlinePayload());
}
} // namespace

uint8_t* GetPooledByValPayload(const ByValUserDataHeader* header)
{
    const auto* handle = reinterpret_cast<const ByValPayloadPool::Handle*>(header->InlinePayload());
    uint8_t* data = s_pool->GetData(*handle);
    if (data == nullptr)
        LuaException::Throw("zlua: ByVal userdata has been released");
    return data;
}

size_t StructRegistry::GetPooledUserDataSize()
{
    return sizeof(ByValUserDataHeader) + sizeof(ByValPayloadPool::Handle);
}

void StructRegistry::InitializeUserData(ByValUserDataHeader* header)
{
    new (header->InlinePayload()) ByValPayloadPool::Handle();
}

void StructRegistry::AllocatePayload(ByValUserDataHeader* header, size_t payloadSize)
{
    IL2CPP_ASSERT(header->pooled);
    try
    {
        *Storage(header) = s_pool->Allocate(payloadSize);
    }
    catch (const std::bad_alloc&)
    {
        LuaException::Throw("zlua: unable to allocate ByVal payload page");
    }
    catch (const std::length_error&)
    {
        LuaException::Throw("zlua: ByVal payload is too large");
    }
}

void StructRegistry::Unregister(ByValUserDataHeader* header)
{
    if (!header->pooled)
        return;
    if (s_callDepth == 0)
        s_pool->Release(*Storage(header));
    else
        s_pool->Retire(*Storage(header));
}

void StructRegistry::BeginCall()
{
    ++s_callDepth;
}

void StructRegistry::EndCall()
{
    IL2CPP_ASSERT(s_callDepth != 0);
    if (--s_callDepth == 0)
        s_pool->DrainRetired();
}

void StructRegistry::AccountAllocation(lua_State* L, size_t payloadSize)
{
    if (s_closing)
        return;
    s_pressure = payloadSize > SIZE_MAX - s_pressure ? SIZE_MAX : s_pressure + payloadSize;
    if (s_pressure < kPressureBytes)
        return;
#if defined(LUA_GCISRUNNING)
    if (lua_gc(L, LUA_GCISRUNNING) == 0)
        return; // Respect collectgarbage("stop").
#endif
    // Compensate for payload bytes no longer counted by Lua's allocator. Step
    // only after initialization, with the userdata rooted in its Lua stack slot.
    const size_t work = kPressureBytes / 1024;
    s_pressure -= kPressureBytes;
    lua_gc(L, LUA_GCSTEP, static_cast<int>(work));
}

StructPoolStats StructRegistry::GetStats()
{
    return s_pool->GetStats();
}

void StructRegistry::Initialize(lua_State* L)
{
    (void)L;
    IL2CPP_ASSERT(s_pool->GetStats().pages == 0 && s_callDepth == 0);
    s_pool->Clear();
    s_pressure = 0;
    s_closing = false;
}

void StructRegistry::BeginShutdown()
{
    s_closing = true;
    s_pool->BeginShutdown();
}

void StructRegistry::Shutdown(lua_State* L)
{
    (void)L;
    IL2CPP_ASSERT(s_callDepth == 0);
    // Lua has closed; no live userdata or native call can use these addresses.
    s_pool->Clear();
    s_pressure = 0;
}

int StructRegistry::OnReleaseByValUserData(lua_State* L)
{
    Unregister(StructMarshal::GetByValHeaderUnchecked(L, 1));
    return 0;
}
} // namespace zlua
