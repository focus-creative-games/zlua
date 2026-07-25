#pragma once

#include "../ZLuaCommon.h"


namespace zlua
{
class LuaStackGuard
{
  private:
    lua_State* _L;
    int _top;

  public:
    LuaStackGuard(lua_State* L) : _L(L), _top(lua_gettop(L)) {}
    ~LuaStackGuard()
    {
        lua_settop(_L, _top);
    }

    int GetTop() const
    {
        return _top;
    }
};
} // namespace zlua
