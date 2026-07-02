#pragma once

struct Il2CppClass;
struct Il2CppDelegate;

struct lua_State;

namespace zlua
{
    class LuaDelegateBinder
    {
    public:
        static int CreateFunctionRef(lua_State* L, int index);
        static Il2CppDelegate* Create(lua_State* L, Il2CppClass* delegateClass, int funcRef);
        static Il2CppDelegate* CreateFromStack(lua_State* L, int index, Il2CppClass* delegateClass);
    };
}
