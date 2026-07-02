#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class LuaMetadataAlloc
{
  public:
    static void Initialize();
    static void Shutdown();

    static void* Malloc(size_t size);
    static void* Calloc(size_t count, size_t size);

    static void* MallocZeroed(size_t size)
    {
        return Calloc(1, size);
    }

    template <typename T>
    static T* MallocAny()
    {
        return (T*)Malloc(sizeof(T));
    }

    template <typename T>
    static T* MallocAnyZeroed()
    {
        return (T*)Calloc(1, sizeof(T));
    }

    template <typename T>
    static T* CallocArray(size_t count)
    {
        return (T*)Calloc(count, sizeof(T));
    }

    template <typename T>
    static T* NewAny()
    {
        return new (MallocAnyZeroed<T>()) T();
    }
};
} // namespace zlua