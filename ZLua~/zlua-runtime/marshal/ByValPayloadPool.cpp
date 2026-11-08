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

#include "ByValPayloadPool.h"

#include "il2cpp-config.h"
#include "il2cpp-object-internals.h"
#include "gc/GarbageCollector.h"

#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

namespace zlua
{
struct ByValPayloadPool::Page
{
    ByValPayloadPool* owner = nullptr;
    Page* previous = nullptr;
    Page* next = nullptr;
    Page* availablePrevious = nullptr;
    Page* availableNext = nullptr;
    Page* pendingNext = nullptr;
    uint8_t* data = nullptr;
    uint32_t* nextSlot = nullptr; // Native metadata is never conservatively scanned.
    size_t stride = 0;
    size_t bytes = 0;
    size_t bucket = 0;
    uint32_t capacity = 0;
    uint32_t used = 0; // Includes deferred releases whose payload may still be in use.
    uint32_t freeHead = kNoSlot;
    uint32_t pendingHead = kNoSlot;
};

void ByValPayloadPool::AddAvailable(Page* page)
{
    Bucket& bucket = _buckets[page->bucket];
    page->availablePrevious = nullptr;
    page->availableNext = bucket.available;
    if (bucket.available != nullptr)
        bucket.available->availablePrevious = page;
    bucket.available = page;
}

void ByValPayloadPool::RemoveAvailable(Page* page)
{
    Bucket& bucket = _buckets[page->bucket];
    if (page->availablePrevious != nullptr)
        page->availablePrevious->availableNext = page->availableNext;
    else
        bucket.available = page->availableNext;
    if (page->availableNext != nullptr)
        page->availableNext->availablePrevious = page->availablePrevious;
    page->availablePrevious = page->availableNext = nullptr;
}

void ByValPayloadPool::DestroyPage(Page* page)
{
    IL2CPP_ASSERT(page->used == 0 && page->pendingHead == kNoSlot);
    RemoveAvailable(page);
    if (_buckets[page->bucket].empty == page)
        _buckets[page->bucket].empty = nullptr;
    if (page->previous != nullptr)
        page->previous->next = page->next;
    else
        _pages = page->next;
    if (page->next != nullptr)
        page->next->previous = page->previous;
    --_stats.pages;
    _stats.reservedBytes -= page->bytes;
    il2cpp::gc::GarbageCollector::FreeFixed(page->data);
    delete[] page->nextSlot;
    delete page;
}

ByValPayloadPool::Page* ByValPayloadPool::CreatePage(size_t bucket, size_t stride)
{
    std::unique_ptr<Page> page(new Page());
    page->owner = this;
    page->bucket = bucket;
    page->stride = stride;
    page->capacity = static_cast<uint32_t>(stride < kPageBytes ? kPageBytes / stride : 1);
    page->bytes = stride * page->capacity;
    std::unique_ptr<uint32_t[]> indices(new uint32_t[page->capacity]);
    for (uint32_t i = 0; i < page->capacity; ++i)
        indices[i] = i + 1 < page->capacity ? i + 1 : kNoSlot;
    page->data = static_cast<uint8_t*>(il2cpp::gc::GarbageCollector::AllocateFixed(page->bytes, nullptr));
    if (page->data == nullptr)
        throw std::bad_alloc();
    std::memset(page->data, 0, page->bytes);
    page->nextSlot = indices.release();
    page->freeHead = 0;
    page->next = _pages;
    if (_pages != nullptr)
        _pages->previous = page.get();
    _pages = page.get();
    AddAvailable(page.get());
    ++_stats.pages;
    ++_stats.allocatedPages;
    _stats.reservedBytes += page->bytes;
    return page.release();
}

void ByValPayloadPool::FreeSlot(Page* page, uint32_t index)
{
    uint8_t* payload = page->data + index * page->stride;
    // Clear the entire slot, including padding, before making it reusable.
    std::memset(payload, 0, page->stride);
    il2cpp::gc::GarbageCollector::SetWriteBarrier(reinterpret_cast<void**>(payload), page->stride);
    if (page->used == page->capacity)
        AddAvailable(page);
    --page->used;
    page->nextSlot[index] = page->freeHead;
    page->freeHead = index;
    if (page->used == 0)
    {
        Bucket& bucket = _buckets[page->bucket];
        if (_closing || page->capacity == 1 || bucket.empty != nullptr)
            DestroyPage(page);
        else
            bucket.empty = page;
    }
}

void ByValPayloadPool::DrainRetired()
{
    while (_pending != nullptr)
    {
        Page* page = _pending;
        _pending = page->pendingNext;
        page->pendingNext = nullptr;
        uint32_t index = page->pendingHead;
        page->pendingHead = kNoSlot;
        while (index != kNoSlot)
        {
            const uint32_t next = page->nextSlot[index];
            --_stats.pendingSlots;
            FreeSlot(page, index); // The final slot can destroy this page.
            index = next;
        }
    }
}

ByValPayloadPool::Handle ByValPayloadPool::Allocate(size_t payloadSize)
{
    IL2CPP_ASSERT(payloadSize != 0);
    size_t bucket = 0;
    size_t stride = kMinStride;
    while (stride < payloadSize)
    {
        if (++bucket == kBucketCount)
            throw std::length_error("ByVal payload is too large");
        stride *= 2;
    }
    Page* page = _buckets[bucket].available;
    if (page == nullptr)
        page = CreatePage(bucket, stride);
    if (_buckets[bucket].empty == page)
        _buckets[bucket].empty = nullptr;
    const uint32_t index = page->freeHead;
    page->freeHead = page->nextSlot[index];
    ++page->used;
    if (page->freeHead == kNoSlot)
        RemoveAvailable(page);
    ++_stats.liveSlots;
    return Handle(page, index);
}

ByValPayloadPool::Handle::Handle(Handle&& other) noexcept : _page(std::exchange(other._page, nullptr)), _index(std::exchange(other._index, kNoSlot)) {}

ByValPayloadPool::Handle& ByValPayloadPool::Handle::operator=(Handle&& other) noexcept
{
    IL2CPP_ASSERT(_page == nullptr);
    _page = std::exchange(other._page, nullptr);
    _index = std::exchange(other._index, kNoSlot);
    return *this;
}

ByValPayloadPool::~ByValPayloadPool()
{
    Clear();
}

uint8_t* ByValPayloadPool::GetData(const Handle& handle) const
{
    if (handle._page == nullptr)
        return nullptr;
    IL2CPP_ASSERT(handle._page->owner == this);
    return handle._page->data + handle._index * handle._page->stride;
}

void ByValPayloadPool::Release(Handle& handle)
{
    Page* page = handle._page;
    if (page == nullptr)
        return;
    IL2CPP_ASSERT(page->owner == this);
    const uint32_t index = handle._index;
    handle._page = nullptr;
    handle._index = kNoSlot;
    --_stats.liveSlots;
    FreeSlot(page, index);
}

void ByValPayloadPool::Retire(Handle& handle)
{
    Page* page = handle._page;
    if (page == nullptr)
        return;
    IL2CPP_ASSERT(page->owner == this);
    const uint32_t index = handle._index;
    handle._page = nullptr;
    handle._index = kNoSlot;
    --_stats.liveSlots;
    if (page->pendingHead == kNoSlot)
    {
        page->pendingNext = _pending;
        _pending = page;
    }
    page->nextSlot[index] = page->pendingHead;
    page->pendingHead = index;
    ++_stats.pendingSlots;
}

void ByValPayloadPool::BeginShutdown()
{
    _closing = true;
}

void ByValPayloadPool::Clear()
{
    // Call only when no client can use a payload address. Handles are borrowed
    // slot identities; Clear does not dereference their possibly dead owners.
    _pending = nullptr;
    while (_pages != nullptr)
    {
        Page* page = _pages;
        _pages = page->next;
        il2cpp::gc::GarbageCollector::FreeFixed(page->data);
        delete[] page->nextSlot;
        delete page;
    }
    for (Bucket& bucket : _buckets)
        bucket = {};
    _stats = {};
    _closing = false;
}
} // namespace zlua
