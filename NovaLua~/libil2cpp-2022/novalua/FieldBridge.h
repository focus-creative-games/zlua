#pragma once

#include "NovaLuaCommon.h"

struct FieldInfo;
struct Il2CppClass;
struct Il2CppObject;

namespace novalua
{
    class FieldBridge
    {
    public:
        static FieldInfo* FindPublicField(Il2CppClass* klass, const char* name, bool requireStatic);
        static void* GetFieldAddress(FieldInfo* field, Il2CppObject* instance);
        static int PushField(lua_State* L, FieldInfo* field, void* fieldPtr);
        static int SetField(lua_State* L, FieldInfo* field, void* fieldPtr, int valueIndex);
    };
}
