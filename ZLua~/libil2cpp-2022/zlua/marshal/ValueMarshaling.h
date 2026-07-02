#pragma once

#include "../ZLuaCommon.h"

struct Il2CppClass;
struct Il2CppObject;
struct Il2CppType;
struct MethodInfo;

namespace zlua
{
    // C# implicit conversion category for overload resolution (METHOD_OVERLOAD_SPEC §3.6).
    enum class ConversionKind : uint8_t
    {
        None = 0,
        Identity,
        ImplicitNumeric,
        ImplicitEnum,
        NullLiteral,
        ImplicitReference,
        ImplicitBoxing,
    };

    // Il2CppType-level marshal dispatch for MethodBridge (delegates to *Marshal types).
    class ValueMarshaling
    {
    public:
        static bool IsStructClass(Il2CppClass* klass);
        static bool IsEnumClass(Il2CppClass* klass);
        static size_t GetValueTypeInstanceSize(Il2CppClass* klass);

        static ConversionKind GetConversionKind(lua_State* L, int index, const Il2CppType* type);
        static ConversionKind GetConversionKind(lua_State* L, int index, const Il2CppType* type, const MethodInfo* method, int paramIndex);
        static bool CanConvert(lua_State* L, int index, const Il2CppType* type);
        static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize);
        static bool TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize, const MethodInfo* method, int paramIndex);
        static int PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj);
        static int PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj, const MethodInfo* method);

        static bool TryMatchParameters(lua_State* L, int argStart, int argCount, const MethodInfo* method);
        static const MethodInfo* FindMatchingMethod(
            Il2CppClass* klass, const char* name, bool isStatic, lua_State* L, int argStart, int argCount);
        static const MethodInfo* FindMatchingConstructor(Il2CppClass* klass, lua_State* L, int argStart, int argCount);
        static int FindParamsParameterIndex(const MethodInfo* method);

        static void PushStructInstance(lua_State* L, Il2CppClass* klass, Il2CppObject* boxed);
        static bool AttachInstanceMetatable(lua_State* L, Il2CppClass* klass);
        static bool AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass);
    };
}
