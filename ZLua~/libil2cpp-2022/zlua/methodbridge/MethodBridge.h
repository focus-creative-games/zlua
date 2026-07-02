#pragma once

#include "../ZLuaCommon.h"

struct MethodInfo;
struct Il2CppObject;
struct Il2CppClass;

namespace zlua
{
    class MethodBridge
    {
    public:
        static int InvokeMethod(lua_State* L, const MethodInfo* method, void* target, int argStart);
        static int InvokeConstructor(lua_State* L, const MethodInfo* ctor, Il2CppClass* klass, int argStart, int argCount);
    };
}
