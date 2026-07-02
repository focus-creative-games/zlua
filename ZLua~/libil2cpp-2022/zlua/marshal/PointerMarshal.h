#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;
struct Il2CppType;
struct Il2CppObject;

namespace zlua
{
    class PointerMarshal
    {
    public:
        static bool IsIntPtrClass(Il2CppClass* klass);
        static bool IsUIntPtrClass(Il2CppClass* klass);
        static bool IsPointerType(const Il2CppType* type);
        static bool IsUnsupportedMarshalType(Il2CppClass* klass);

        static bool CanConvert(lua_State* L, int index, const Il2CppType* type);
        static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize);
        static int PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj);
    };
}
