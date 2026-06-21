#pragma once

#include "NovaLuaCommon.h"

struct FieldInfo;
struct Il2CppClass;
struct Il2CppObject;

namespace novalua
{
    typedef int (*FnFieldGetter)(lua_State* L, void* fieldPtr);
    typedef int (*FnFieldSetter)(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex);

    struct FieldAccessor
    {
        FnFieldGetter getter;
        FnFieldSetter setter;
    };

    class FieldBridge
    {
    public:
        static bool IsPublicField(const FieldInfo* field);
        static void* GetFieldAddress(const FieldInfo* field, Il2CppObject* instance);
        static FieldAccessor ResolveFieldAccessor(const FieldInfo* field);
        static void WriteBarrierForFieldType(const Il2CppType* type, void** targetAddress);
    };
}
