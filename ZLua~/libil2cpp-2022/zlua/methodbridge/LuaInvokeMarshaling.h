#pragma once

#include "../marshal/MarshalAsTypes.h"

#include "../ZLuaCommon.h"

struct Il2CppArray;
struct Il2CppClass;
struct Il2CppObject;
struct Il2CppString;
struct Il2CppType;

namespace zlua
{
    class LuaInvokeMarshaling
    {
    public:
        static void EnterCall();

        static Il2CppClass* ResolveClass(const char* assemblyName, const char* typeFullName);

        static void PushDefaultBool(lua_State* L, bool value);
        static void PushDefaultInt32(lua_State* L, int32_t value);
        static void PushDefaultUInt32(lua_State* L, uint32_t value);
        static void PushDefaultInt64(lua_State* L, int64_t value);
        static void PushDefaultUInt64(lua_State* L, uint64_t value);
        static void PushDefaultFloat(lua_State* L, float value);
        static void PushDefaultDouble(lua_State* L, double value);
        static void PushDefaultString(lua_State* L, Il2CppString* value);

        static void PushInt32UserData(lua_State* L, int32_t value);
        static void PushStringUserData(lua_State* L, Il2CppString* value);
        static void PushEnumUserData(lua_State* L, Il2CppClass* enumClass, int32_t value);
        static void PushByteArrayBytes(lua_State* L, Il2CppArray* value);
        static void PushStructOpaque(lua_State* L, Il2CppClass* structClass, const void* valuePtr);

        static bool PopDefaultBool(lua_State* L, int index);
        static int32_t PopDefaultInt32(lua_State* L, int index);
        static Il2CppString* PopDefaultString(lua_State* L, int index);

        static int32_t PopInt32UserData(lua_State* L, int index);
        static Il2CppString* PopStringUserData(lua_State* L, int index);
        static int32_t PopEnumUserData(lua_State* L, int index, Il2CppClass* enumClass);
        static Il2CppArray* PopByteArrayBytes(lua_State* L, int index);
    };
} // namespace zlua
