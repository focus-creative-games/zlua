#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class DelegateMarshal
{
public:
    static void Push(lua_State* L, Il2CppDelegate* delegate, int metatableRefIndex);
    static Il2CppDelegate* Pop(lua_State* L, int delegateIndex, Il2CppClass* delegateClass);
    static Il2CppDelegate* CreateFromFuncRef(lua_State* L, Il2CppClass* delegateClass, int funcRef);
};
}