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

#include <climits>
#include <cstddef>
#include <cstdint>

namespace zlua
{
// Domain-thread allocator. Owns pages and retired slots, with no Lua dependency.
class ByValPayloadPool
{
    struct Page;

public:
    struct Stats
    {
        size_t pages = 0;
        size_t reservedBytes = 0;
        size_t liveSlots = 0;
        size_t pendingSlots = 0;
        size_t allocatedPages = 0;
    };

    class Handle
    {
    public:
        Handle() = default;
        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;
        Handle(Handle&& other) noexcept;
        Handle& operator=(Handle&& other) noexcept;

    private:
        friend class ByValPayloadPool;
        Handle(Page* page, uint32_t index) : _page(page), _index(index) {}
        Page* _page = nullptr;
        uint32_t _index = UINT32_MAX;
    };

    ByValPayloadPool() = default;
    ~ByValPayloadPool();
    ByValPayloadPool(const ByValPayloadPool&) = delete;
    ByValPayloadPool& operator=(const ByValPayloadPool&) = delete;

    Handle Allocate(size_t payloadSize);
    uint8_t* GetData(const Handle& handle) const;
    void Release(Handle& handle);
    void Retire(Handle& handle);
    void DrainRetired();
    void BeginShutdown();
    void Clear();
    Stats GetStats() const
    {
        return _stats;
    }

private:
    static constexpr size_t kPageBytes = 16 * 1024;
    static constexpr size_t kMinStride = 16;
    static constexpr size_t kBucketCount = sizeof(size_t) * CHAR_BIT - 4;
    static constexpr uint32_t kNoSlot = UINT32_MAX;

    struct Bucket
    {
        Page* available = nullptr;
        Page* empty = nullptr;
    };

    void AddAvailable(Page* page);
    void RemoveAvailable(Page* page);
    void DestroyPage(Page* page);
    Page* CreatePage(size_t bucket, size_t stride);
    void FreeSlot(Page* page, uint32_t index);

    Bucket _buckets[kBucketCount];
    Page* _pages = nullptr;
    Page* _pending = nullptr;
    bool _closing = false;
    Stats _stats;
};
} // namespace zlua
