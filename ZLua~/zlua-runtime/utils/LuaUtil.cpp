#include <cmath>

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

void LuaUtil::PCall(lua_State* L, int nargs, int nresults, int errfunc)
{
    if (lua_pcall(L, nargs, nresults, errfunc) != LUA_OK)
    {
        const char* err = lua_tostring(L, -1);
        LuaException::Throw(err != nullptr ? err : "lua pcall failed");
    }
}
} // namespace zlua
