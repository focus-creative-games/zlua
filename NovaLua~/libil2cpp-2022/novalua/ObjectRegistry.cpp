#include "ObjectRegistry.h"

#include "gc/GCHandle.h"

namespace novalua
{
    struct NovaLuaUserData
    {
        uint32_t gcHandle;
    };

    void ObjectRegistry::PushObject(lua_State* L, Il2CppObject* obj)
    {
        if (obj == nullptr)
        {
            lua_pushnil(L);
            return;
        }

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_newuserdatauv(L, sizeof(NovaLuaUserData), 0);
        ud->gcHandle = il2cpp::gc::GCHandle::New(obj, false);
    }

    Il2CppObject* ObjectRegistry::GetObject(lua_State* L, int idx)
    {
        if (!lua_isuserdata(L, idx))
            return nullptr;

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_touserdata(L, idx);
        if (ud == nullptr || ud->gcHandle == 0)
            return nullptr;

        return il2cpp::gc::GCHandle::GetTarget(ud->gcHandle);
    }

    void ObjectRegistry::ReleaseObject(lua_State* L, int idx)
    {
        if (!lua_isuserdata(L, idx))
            return;

        NovaLuaUserData* ud = (NovaLuaUserData*)lua_touserdata(L, idx);
        if (ud == nullptr || ud->gcHandle == 0)
            return;

        il2cpp::gc::GCHandle::Free(ud->gcHandle);
        ud->gcHandle = 0;
    }
}
