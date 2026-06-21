#pragma once

#include "NovaLuaCommon.h"

struct MethodInfo;
struct Il2CppObject;

namespace novalua
{
    class MethodBridge
    {
    public:
        static int InvokeStaticIntIntInt(lua_State* L, const MethodInfo* method, int argStart);
        static int InvokeStaticInt(lua_State* L, const MethodInfo* method);
        static int InvokeStaticVoidInt(lua_State* L, const MethodInfo* method, int argStart);
        static int InvokeInstanceInt(lua_State* L, const MethodInfo* method, Il2CppObject* instance, int argStart);
        static int InvokeInstanceVoidInt(lua_State* L, const MethodInfo* method, Il2CppObject* instance, int argStart);
        static int InvokeConstructorNoArgs(lua_State* L, const MethodInfo* ctor, Il2CppClass* klass);
    };
}
