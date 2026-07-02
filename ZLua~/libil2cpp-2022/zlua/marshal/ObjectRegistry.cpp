#include "ObjectRegistry.h"
#include "StructMarshal.h"

#include "../LuaConsts.h"
#include "../mt/MetaTableCache.h"
#include "../mt/TypeRegistry.h"
#include "../utils/LuaException.h"

#include "gc/GarbageCollector.h"
#include "utils/Memory.h"
#include "vm/Class.h"

#include <cstring>
#include <stack>

namespace zlua
{

static const uint32_t kInvalidSlotIndex = UINT32_MAX;
static int s_objectCacheRef = LUA_NOREF;

class ObjectSlotRegistry
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

        il2cpp::gc::GarbageCollector::RegisterRoot((char*)_registeredObjects, (size_t)_capacity * sizeof(Il2CppObject*));

        if (oldObjects != nullptr)
        {
            il2cpp::gc::GarbageCollector::UnregisterRoot((char*)oldObjects);
            IL2CPP_FREE(oldObjects);
        }
    }
};

static ObjectSlotRegistry s_objectSlots;

void ObjectRegistry::Initialize(lua_State* L)
{
    IL2CPP_ASSERT(s_objectCacheRef == LUA_NOREF);

    lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, LuaConsts::WeakModeValue);
    lua_setfield(L, -2, LuaConsts::MetaMode);
    lua_setmetatable(L, -2);
    s_objectCacheRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

void ObjectRegistry::Shutdown(lua_State* L)
{
    if (s_objectCacheRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, s_objectCacheRef);
        s_objectCacheRef = LUA_NOREF;
    }
}

static void RemoveFromObjectCache(lua_State* L, Il2CppObject* obj)
{
    if (obj == nullptr)
        return;

    IL2CPP_ASSERT(s_objectCacheRef != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    lua_pushlightuserdata(L, obj);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

static bool TryPushCachedObject(lua_State* L, Il2CppObject* obj)
{
    IL2CPP_ASSERT(s_objectCacheRef != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    lua_pushlightuserdata(L, obj);
    lua_rawget(L, -2);
    if (!lua_isuserdata(L, -1))
    {
        lua_pop(L, 2);
        return false;
    }

    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, -1);
    if (ud == nullptr || ud->obj != obj || ud->slotIndex == kInvalidSlotIndex)
    {
        lua_pop(L, 2);
        return false;
    }

    lua_remove(L, -2);
    return true;
}

static void AddToObjectCache(lua_State* L, Il2CppObject* obj, int userdataIndex)
{
    IL2CPP_ASSERT(s_objectCacheRef != LUA_NOREF);

    const int absUserdataIndex = lua_absindex(L, userdataIndex);
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    lua_pushlightuserdata(L, obj);
    lua_pushvalue(L, absUserdataIndex);
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

static void AttachObjectMetatable(lua_State* L, Il2CppClass* klass, int metatableRefIndex)
{
    if (metatableRefIndex != LUA_NOREF)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, metatableRefIndex);
        lua_setmetatable(L, -2);
    }
    else
    {
        MetaTableCache::PushByObjMetatable(L, klass);
        lua_setmetatable(L, -2);
    }
}

void ObjectRegistry::Push(lua_State* L, Il2CppObject* obj, int metatableRefIndex)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    if (TryPushCachedObject(L, obj))
        return;

    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_newuserdatauv(L, sizeof(ZLuaObjectUserData), 0);
    ud->header.kind = UserDataKind::ByObj;
    ud->obj = obj;
    ud->slotIndex = s_objectSlots.RegisterObject(obj);
    if (ud->slotIndex == kInvalidSlotIndex)
    {
        ud->obj = nullptr;
        LuaException::Throw("zlua internal error: failed to register managed object");
    }

    AttachObjectMetatable(L, obj->klass, metatableRefIndex);
    AddToObjectCache(L, obj, -1);
}

Il2CppObject* ObjectRegistry::Pop(lua_State* L, int idx)
{
    if (lua_isnil(L, idx))
        return nullptr;
    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, idx);
    if (ud == nullptr || ud->header.kind != UserDataKind::ByObj)
    {
        LuaException::Throw("zlua argument mismatch: expected by-obj userdata");
    }
    return ud->obj;
}

int ObjectRegistry::OnReleaseObjectUserData(lua_State* L)
{
    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, 1);
    IL2CPP_ASSERT(ud != nullptr);
    IL2CPP_ASSERT(ud->slotIndex != kInvalidSlotIndex);

    Il2CppObject* obj = ud->obj;
    s_objectSlots.UnregisterObject(ud->slotIndex);
    RemoveFromObjectCache(L, obj);
    return 0;
}
} // namespace zlua
