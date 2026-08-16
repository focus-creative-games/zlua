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

#include <cmath>
#include <string>

#include "LuaUtil.h"

#include "../lvm/LuaEnv.h"
#include "LuaException.h"

#include "utils/StringUtils.h"
#include "vm/Type.h"

namespace zlua
{
bool LuaUtil::IsStrictLuaInteger(lua_State* L, int index, lua_Integer& value)
{
    if (lua_isinteger(L, index))
    {
        value = lua_tointeger(L, index);
        return true;
    }
    if (!lua_isnumber(L, index))
        return false;

    const lua_Number number = lua_tonumber(L, index);
    lua_Number intPart = 0.0;
    if (std::modf(number, &intPart) != 0.0)
        return false;

    value = (lua_Integer)intPart;
    return true;
}

int LuaUtil::PushRef(lua_State* L, int refIndex)
{
    IL2CPP_ASSERT(refIndex != LUA_NOREF);
    lua_rawgeti(L, LUA_REGISTRYINDEX, refIndex);
    return 1;
}

// int LuaUtil::PCallClosureRefAt(lua_State* L, int closureRef, const int* argStackIndices, int argCount, int resultCount)
// {
//     IL2CPP_ASSERT(closureRef != LUA_NOREF);

//     lua_rawgeti(L, LUA_REGISTRYINDEX, closureRef);
//     for (int i = 0; i < argCount; ++i)
//         lua_pushvalue(L, argStackIndices[i]);
//     if (lua_pcall(L, argCount, resultCount, 0) != LUA_OK)
//         return lua_error(L);
//     return resultCount;
// }

std::string LuaUtil::FormatErrorObject(lua_State* L, int index)
{
    index = lua_absindex(L, index);
    const int top = lua_gettop(L);
    size_t len = 0;
    const char* str = luaL_tolstring(L, index, &len);
    if (str != nullptr)
    {
        std::string message(str, len);
        lua_settop(L, top);
        if (!message.empty())
            return message;
    }
    else
    {
        lua_settop(L, top);
    }

    return std::string("lua pcall failed (error object is ") + luaL_typename(L, index) + ")";
}

void LuaUtil::PCall(lua_State* L, int nargs, int nresults, int errfunc)
{
    if (lua_pcall(L, nargs, nresults, errfunc) != LUA_OK)
    {
        LuaException::Throw(FormatErrorObject(L, -1));
    }
}
} // namespace zlua
