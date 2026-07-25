#include "LuaMetadataAlloc.h"

#include "utils/MemoryPool.h"

namespace zlua
{
    static il2cpp::utils::MemoryPool* s_memoryPool = nullptr;

    void LuaMetadataAlloc::Initialize()
    {
        s_memoryPool = new il2cpp::utils::MemoryPool(1024 * 1024 * 1);
    }

    void LuaMetadataAlloc::Shutdown()
    {
        delete s_memoryPool;
        s_memoryPool = nullptr;
    }

    void* LuaMetadataAlloc::Malloc(size_t size)
    {
        return s_memoryPool->Malloc(size);
    }

    void* LuaMetadataAlloc::Calloc(size_t count, size_t size)
    {
        return s_memoryPool->Calloc(count, size);
    }
}