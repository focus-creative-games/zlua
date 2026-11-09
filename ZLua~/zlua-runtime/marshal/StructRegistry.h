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

#pragma once

#include "../ZLuaCommon.h"
#include "MarshalDefs.h"
#include "ByValPayloadPool.h"

namespace zlua
{
using StructPoolStats = ByValPayloadPool::Stats;

class StructRegistry
{
public:
    static void Initialize(lua_State* L);
    static void BeginShutdown();
    // Called only after lua_close: no page may disappear while a closing callback uses it.
    static void Shutdown(lua_State* L);

    static size_t GetPooledUserDataSize();
    static void InitializeUserData(ByValUserDataHeader* header);
    static void AllocatePayload(ByValUserDataHeader* header, size_t payloadSize);
    static void Unregister(ByValUserDataHeader* header);
    static void AccountAllocation(lua_State* L, size_t payloadSize);
    static StructPoolStats GetStats();

    static size_t GetCallDepth();
    // Protected Lua calls may longjmp past native scope destructors.
    static void RestoreCallDepth(size_t depth);
    static void BeginCall();
    static void EndCall();
    static int OnReleaseByValUserData(lua_State* L);
};

// A protected call always returns to this native frame, even after Lua longjmp.
// Restore its entry depth, preserving any outer bridge that still owns pointers.
class StructPayloadProtectedCallScope
{
public:
    StructPayloadProtectedCallScope() : _depth(StructRegistry::GetCallDepth()) {}
    ~StructPayloadProtectedCallScope()
    {
        StructRegistry::RestoreCallDepth(_depth);
    }
    StructPayloadProtectedCallScope(const StructPayloadProtectedCallScope&) = delete;
    StructPayloadProtectedCallScope& operator=(const StructPayloadProtectedCallScope&) = delete;

private:
    size_t _depth;
};

// Prevent a reentrant/manual __gc from recycling storage still used by a native
// bridge. Pending slots are cleared/reused only after the outer bridge returns.
class StructPayloadCallScope
{
public:
    StructPayloadCallScope()
    {
        StructRegistry::BeginCall();
    }
    ~StructPayloadCallScope()
    {
        StructRegistry::EndCall();
    }
    StructPayloadCallScope(const StructPayloadCallScope&) = delete;
    StructPayloadCallScope& operator=(const StructPayloadCallScope&) = delete;
};
} // namespace zlua
