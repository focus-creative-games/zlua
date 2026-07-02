#pragma once

#include "MarshalAsTypes.h"

#include "../ZLuaCommon.h"

struct Il2CppObject;
struct Il2CppType;

namespace zlua
{
    enum class ConversionKind : uint8_t;

    class MarshalAsBytes
    {
    public:
        static ConversionKind GetConversionKind(lua_State* L, int index, const Il2CppType* type);
        static bool CanConvert(lua_State* L, int index, const Il2CppType* type);
        static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize);
        static int Push(lua_State* L, const Il2CppType* type, Il2CppObject* retObj);
    };
}
