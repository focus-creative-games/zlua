#include "Marshaling.h"

#include "ObjectRegistry.h"
#include "LuaEnv.h"

namespace novalua
{
    void Marshaling::LuaPCall(lua_State* L, int nargs, int nresults)
    {
        if (lua_pcall(L, nargs, nresults, 0) != LUA_OK)
        {
            const char* err = lua_tostring(L, -1);
            LuaEnv::RaiseLuaException(err);
        }
    }

    void Marshaling::PushCString(lua_State* L, Il2CppString* str)
    {
        if (str == nullptr)
        {
            lua_pushnil(L);
            return;
        }

        std::string utf8 = il2cpp::utils::StringUtils::Utf16ToUtf8(
            il2cpp::utils::StringUtils::GetChars(str),
            il2cpp::utils::StringUtils::GetLength(str));
        lua_pushlstring(L, utf8.c_str(), utf8.size());
    }

    Il2CppString* Marshaling::PopCString(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* str = lua_tolstring(L, idx, &len);
        if (str == nullptr)
            return nullptr;
        return il2cpp::vm::String::NewLen(str, (uint32_t)len);
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
        ObjectRegistry::PushObject(L, obj);
    }

    Il2CppObject* Marshaling::PopUserData(lua_State* L, int idx)
    {
        return ObjectRegistry::GetObject(L, idx);
    }

    void detail::DefaultMarshaling<Il2CppString*>::Push(lua_State* L, Il2CppString* v)
    {
        Marshaling::PushCString(L, v);
    }

    Il2CppString* detail::DefaultMarshaling<Il2CppString*>::Pop(lua_State* L, int idx)
    {
        return Marshaling::PopCString(L, idx);
    }
}
