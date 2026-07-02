#include "Marshaling.h"

#include "../LuaEnv.h"

namespace zlua
{
    void Marshaling::LuaPCall(lua_State* L, int nargs, int nresults, int errfunc)
    {
        if (lua_pcall(L, nargs, nresults, errfunc) != LUA_OK)
        {
            const char* err = lua_tostring(L, -1);
            LuaEnv::RaiseLuaException(err);
        }
    }

    void Marshaling::PushCString(lua_State* L, Il2CppString* str)
    {
        StringMarshal::Push(L, str);
    }

    Il2CppString* Marshaling::PopCString(lua_State* L, int idx)
    {
        return StringMarshal::Pop(L, idx);
    }

    void Marshaling::PushLightUserData(lua_State* L, void* ptr)
    {
        lua_pushlightuserdata(L, ptr);
    }

    void* Marshaling::PopLightUserData(lua_State* L, int idx)
    {
        return lua_touserdata(L, idx);
    }

    void Marshaling::PushUserData(lua_State* L, Il2CppObject* obj)
    {
        ObjectMarshal::Push(L, obj);
    }

    Il2CppObject* Marshaling::PopUserData(lua_State* L, int idx)
    {
        return ObjectMarshal::Pop(L, idx);
    }
}
