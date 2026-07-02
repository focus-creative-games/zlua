#include "StringMarshal.h"

#include "vm/String.h"
#include "utils/StringUtils.h"

namespace zlua
{
    void StringMarshal::Push(lua_State* L, Il2CppString* str)
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

    Il2CppString* StringMarshal::Pop(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* str = lua_tolstring(L, idx, &len);
        if (str == nullptr)
            return nullptr;
        return il2cpp::vm::String::NewLen(str, (uint32_t)len);
    }

    bool StringMarshal::CanConvert(lua_State* L, int index)
    {
        const int type = lua_type(L, index);
        return type == LUA_TSTRING || type == LUA_TNIL;
    }

    bool StringMarshal::TryPop(lua_State* L, int index, Il2CppString** dest)
    {
        if (!CanConvert(L, index) || dest == nullptr)
            return false;

        *dest = Pop(L, index);
        return true;
    }
}
