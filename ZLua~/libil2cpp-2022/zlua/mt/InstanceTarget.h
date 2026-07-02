#pragma once

#include "../ZLuaCommon.h"

struct FieldInfo;
struct MethodInfo;

namespace zlua
{
    enum class UserDataKind : uint8_t
    {
        Unknown,
        ByObj,
        ByVal,
    };

    class InstanceTarget
    {
    public:
        static UserDataKind GetUserDataKind(lua_State* L, int index);

        static void* ResolveMethodTarget(lua_State* L, int index, const MethodInfo* method);
        static void* ResolveFieldAddress(lua_State* L, int index, int32_t payloadRelativeOffset, const FieldInfo* field);
    };
}
