#pragma once

#include "ZLuaCommon.h"

struct FieldInfo;
struct Il2CppClass;
struct Il2CppObject;

namespace zlua
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
        static void* ComputeStaticFieldAddress(const FieldInfo* field);
        static int32_t ComputeInstanceFieldOffset(const FieldInfo* field);
        static FieldAccessor ResolveFieldAccessor(const FieldInfo* field);
        static void WriteBarrierForFieldType(const Il2CppType* type, void** targetAddress);
    };
}
