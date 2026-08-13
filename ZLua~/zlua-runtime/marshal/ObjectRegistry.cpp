#include <cstring>
#include <vector>

#include "ObjectRegistry.h"
#include "StructMarshal.h"

#include "../LuaConsts.h"
#include "../mt/MetaTableCache.h"
#include "../mt/TypeRegistry.h"
#include "../utils/Collection.h"
#include "../utils/LuaException.h"

#include "gc/GarbageCollector.h"
#include "utils/Memory.h"
#include "vm/Class.h"

namespace zlua
{

static const uint32_t kInvalidSlotIndex = UINT32_MAX;
static int s_objectCacheRef = LUA_NOREF;

struct ObjectViewKey
{
    Il2CppObject* obj;
    Il2CppClass* viewKlass;
};

struct ObjectViewKeyHash
{
    size_t operator()(const ObjectViewKey& key) const
    {
        size_t h = reinterpret_cast<size_t>(key.obj);
        h ^= reinterpret_cast<size_t>(key.viewKlass) + 0x9e3779b9u + (h << 6) + (h >> 2);
        return h;
    }
};

struct ObjectViewKeyEqual
{
    bool operator()(const ObjectViewKey& lhs, const ObjectViewKey& rhs) const
    {
        return lhs.obj == rhs.obj && lhs.viewKlass == rhs.viewKlass;
    }
};

/* C++ map: (obj, view) -> int key in a single weak-values Lua table (no nested tables). */
static HashMap<ObjectViewKey, int, ObjectViewKeyHash, ObjectViewKeyEqual> s_objectViewRefs;

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
        _freeSlots.push_back(slotIndex);
    }

    Il2CppObject* GetObject(uint32_t slotIndex) const
    {
        if (slotIndex >= (uint32_t)_nextSlotIndex)
            return nullptr;
        return _registeredObjects[slotIndex];
    }

    void Clear()
    {
        if (_registeredObjects != nullptr)
        {
            il2cpp::gc::GarbageCollector::UnregisterRoot((char*)_registeredObjects);
            ZLuaIl2CppFree(_registeredObjects);
            _registeredObjects = nullptr;
        }
        _capacity = 0;
        _nextSlotIndex = 0;
        _freeSlots.clear();
    }

  private:
    // Larger initial capacity reduces RegisterRoot/UnregisterRoot churn on grow.
    static constexpr int32_t kInitialCapacity = 4096;

    Il2CppObject** _registeredObjects = nullptr;
    int32_t _capacity = 0;
    int32_t _nextSlotIndex = 0;
    std::vector<uint32_t> _freeSlots;

    uint32_t AllocateSlot()
    {
        if (!_freeSlots.empty())
        {
            const uint32_t slotIndex = _freeSlots.back();
            _freeSlots.pop_back();
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

        Il2CppObject** newObjects = (Il2CppObject**)ZLuaIl2CppCalloc((size_t)newCapacity, sizeof(Il2CppObject*));
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
            ZLuaIl2CppFree(oldObjects);
        }
    }
};

static ObjectSlotRegistry s_objectSlots;

void ObjectRegistry::Initialize(lua_State* L)
{
    IL2CPP_ASSERT(s_objectCacheRef == LUA_NOREF);
    IL2CPP_ASSERT(s_objectViewRefs.empty());
    /* Slots must be empty (Shutdown clears them); never leave a previous state's GC root. */
    s_objectSlots.Clear();

    /* Single weak-values table; C++ HashMap holds integer keys into this table. */
    lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, LuaConsts::WeakModeValue);
    lua_setfield(L, -2, LuaConsts::MetaMode);
    lua_setmetatable(L, -2);
    s_objectCacheRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

void ObjectRegistry::Shutdown(lua_State* L)
{
    s_objectViewRefs.clear();
    s_objectSlots.Clear();
    if (s_objectCacheRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, s_objectCacheRef);
        s_objectCacheRef = LUA_NOREF;
    }
}

static void RemoveFromObjectCache(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass)
{
    if (obj == nullptr || viewKlass == nullptr)
        return;

    /* LuaEnv::Shutdown clears the cache before lua_close; __gc still runs and must no-op. */
    if (s_objectCacheRef == LUA_NOREF)
        return;

    ObjectViewKey key{obj, viewKlass};
    auto it = s_objectViewRefs.find(key);
    if (it == s_objectViewRefs.end())
        return;

    const int cacheKey = it->second;
    s_objectViewRefs.erase(it);

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    luaL_unref(L, -1, cacheKey);
    lua_pop(L, 1);
}

static bool TryPushCachedObject(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass)
{
    IL2CPP_ASSERT(s_objectCacheRef != LUA_NOREF);
    IL2CPP_ASSERT(viewKlass != nullptr);

    ObjectViewKey key{obj, viewKlass};
    auto it = s_objectViewRefs.find(key);
    if (it == s_objectViewRefs.end())
        return false;

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    lua_rawgeti(L, -1, it->second);
    lua_remove(L, -2); /* drop weak table */

    if (!lua_isuserdata(L, -1))
    {
        lua_pop(L, 1);
        /* Stale entry (value already collected); drop C++ map slot. */
        const int cacheKey = it->second;
        s_objectViewRefs.erase(it);
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
        luaL_unref(L, -1, cacheKey);
        lua_pop(L, 1);
        return false;
    }

    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)lua_touserdata(L, -1);
    if (ud == nullptr || ud->obj != obj || ud->viewKlass != viewKlass || ud->slotIndex == kInvalidSlotIndex)
    {
        lua_pop(L, 1);
        const int cacheKey = it->second;
        s_objectViewRefs.erase(it);
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
        luaL_unref(L, -1, cacheKey);
        lua_pop(L, 1);
        return false;
    }

    return true;
}

static void AddToObjectCache(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass, int userdataIndex)
{
    IL2CPP_ASSERT(s_objectCacheRef != LUA_NOREF);
    IL2CPP_ASSERT(viewKlass != nullptr);

    const int absUserdataIndex = lua_absindex(L, userdataIndex);
    ObjectViewKey key{obj, viewKlass};

    auto existing = s_objectViewRefs.find(key);
    if (existing != s_objectViewRefs.end())
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
        luaL_unref(L, -1, existing->second);
        lua_pop(L, 1);
        s_objectViewRefs.erase(existing);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_objectCacheRef);
    lua_pushvalue(L, absUserdataIndex);
    const int cacheKey = luaL_ref(L, -2);
    lua_pop(L, 1); /* weak table */

    s_objectViewRefs.insert({key, cacheKey});
}

static void AttachObjectMetatable(lua_State* L, Il2CppClass* viewKlass, int metatableRefIndex)
{
    if (metatableRefIndex != LUA_NOREF)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, metatableRefIndex);
        lua_setmetatable(L, -2);
    }
    else
    {
        MetaTableCache::PushByObjMetatable(L, viewKlass);
        lua_setmetatable(L, -2);
    }
}

void ObjectRegistry::Push(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass, int metatableRefIndex)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    IL2CPP_ASSERT(viewKlass != nullptr);

    if (TryPushCachedObject(L, obj, viewKlass))
        return;

    ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaNewUserData(L, sizeof(ZLuaObjectUserData));
    ud->header.kind = UserDataKind::ByObj;
    ud->obj = obj;
    ud->viewKlass = viewKlass;
    ud->slotIndex = s_objectSlots.RegisterObject(obj);
    if (ud->slotIndex == kInvalidSlotIndex)
    {
        ud->obj = nullptr;
        LuaException::Throw("zlua internal error: failed to register managed object");
    }

    AttachObjectMetatable(L, viewKlass, metatableRefIndex);
    AddToObjectCache(L, obj, viewKlass, -1);
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
    Il2CppClass* viewKlass = ud->viewKlass;
    s_objectSlots.UnregisterObject(ud->slotIndex);
    RemoveFromObjectCache(L, obj, viewKlass);
    return 0;
}
} // namespace zlua
