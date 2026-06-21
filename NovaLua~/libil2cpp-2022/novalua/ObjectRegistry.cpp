#include "ObjectRegistry.h"

#include "gc/GarbageCollector.h"
#include "utils/Memory.h"

#include <cstring>
#include <stack>

namespace novalua
{
    struct NovaLuaUserData
    {
        Il2CppObject* obj;
        uint32_t slotIndex;
    };

    static const uint32_t kInvalidSlotIndex = UINT32_MAX;

    class ObjectRegistryManager
    {
    public:
        uint32_t RegisterObject(Il2CppObject* obj)
        {
            const uint32_t slotIndex = AllocateSlot();
            if (slotIndex == kInvalidSlotIndex)
                return kInvalidSlotIndex;

            _registeredObjects[slotIndex] = obj;
            return slotIndex;
        }

        void UnregisterObject(uint32_t slotIndex)
        {
            if (slotIndex >= (uint32_t)_nextSlotIndex)
                return;

            _registeredObjects[slotIndex] = nullptr;
            _freeSlots.push(slotIndex);
        }

        Il2CppObject* GetObject(uint32_t slotIndex) const
        {
            if (slotIndex >= (uint32_t)_nextSlotIndex)
                return nullptr;
            return _registeredObjects[slotIndex];
        }

    private:
        static constexpr int32_t kInitialCapacity = 1024;

        Il2CppObject** _registeredObjects = nullptr;
        int32_t _capacity = 0;
        int32_t _nextSlotIndex = 0;
        std::stack<uint32_t> _freeSlots;

        uint32_t AllocateSlot()
        {
            if (!_freeSlots.empty())
            {
                const uint32_t slotIndex = _freeSlots.top();
                _freeSlots.pop();
                return slotIndex;
            }

            if (_nextSlotIndex >= _capacity)
                EnsureCapacity(_nextSlotIndex + 1);

            if (_nextSlotIndex >= _capacity)
                return kInvalidSlotIndex;

            return (uint32_t)_nextSlotIndex++;
        }

        void EnsureCapacity(int32_t minCapacity)
        {
            if (minCapacity <= _capacity)
                return;

            int32_t newCapacity = _capacity == 0 ? kInitialCapacity : _capacity;
            while (newCapacity < minCapacity)
                newCapacity *= 2;

            Il2CppObject** newObjects = (Il2CppObject**)IL2CPP_CALLOC(newCapacity, sizeof(Il2CppObject*));
            if (newObjects == nullptr)
                return;

            Il2CppObject** oldObjects = _registeredObjects;
            if (oldObjects != nullptr)
                std::memcpy(newObjects, oldObjects, (size_t)_capacity * sizeof(Il2CppObject*));

            _registeredObjects = newObjects;
            _capacity = newCapacity;

            il2cpp::gc::GarbageCollector::RegisterRoot(
                (char*)_registeredObjects,
                (size_t)_capacity * sizeof(Il2CppObject*));

            if (oldObjects != nullptr)
            {
                il2cpp::gc::GarbageCollector::UnregisterRoot((char*)oldObjects);
                IL2CPP_FREE(oldObjects);
            }
        }
    };

    static ObjectRegistryManager s_objRegMgr;

    void ObjectRegistry::PushObject(lua_State* L, Il2CppObject* obj)
    {
        if (obj == nullptr)
        {
            lua_pushnil(L);
            return;
        }

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_newuserdatauv(L, sizeof(NovaLuaUserData), 0);
        ud->obj = obj;
        ud->slotIndex = s_objRegMgr.RegisterObject(obj);
        if (ud->slotIndex == kInvalidSlotIndex)
        {
            ud->obj = nullptr;
            luaL_error(L, "novalua: failed to register managed object");
        }
    }

    Il2CppObject* ObjectRegistry::GetObject(lua_State* L, int idx)
    {
        if (!lua_isuserdata(L, idx))
            return nullptr;

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_touserdata(L, idx);
        if (ud == nullptr || ud->slotIndex == kInvalidSlotIndex)
            return nullptr;

        if (ud->obj != nullptr)
            return ud->obj;

        return s_objRegMgr.GetObject(ud->slotIndex);
    }

    void ObjectRegistry::ReleaseObject(lua_State* L, int idx)
    {
        if (!lua_isuserdata(L, idx))
            return;

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_touserdata(L, idx);
        if (ud == nullptr || ud->slotIndex == kInvalidSlotIndex)
            return;

        s_objRegMgr.UnregisterObject(ud->slotIndex);
        ud->obj = nullptr;
        ud->slotIndex = kInvalidSlotIndex;
    }
}
