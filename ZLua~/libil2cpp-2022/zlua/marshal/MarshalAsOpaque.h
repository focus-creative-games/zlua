#pragma once

struct Il2CppObject;
struct Il2CppType;

namespace zlua
{
    class MarshalAsOpaque
    {
    public:
        static int Push(lua_State* L, const Il2CppType* type, Il2CppObject* retObj);
    };
}
