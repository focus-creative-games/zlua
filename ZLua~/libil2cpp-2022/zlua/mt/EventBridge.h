#pragma once

#include "../ZLuaCommon.h"

struct EventInfo;

namespace zlua
{
    class EventBridge
    {
    public:
        static int RegisterBinding(const EventInfo* eventInfo, bool isStatic);
        static int CreateAddClosureRef(lua_State* L, int eventId);
        static int CreateRemoveClosureRef(lua_State* L, int eventId);
        static int CreateFireClosureRef(lua_State* L, int eventId);
    };
}
