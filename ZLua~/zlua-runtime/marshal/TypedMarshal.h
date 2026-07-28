#pragma once

#include "../ZLuaCommon.h"

#include "PrimitiveMarshal.h"
#include "StringMarshal.h"

namespace zlua
{

template <typename T>
class MarshalHelper
{
  public:
};

template <typename T>
struct DefaultTypedMarshal;

template <>
struct DefaultTypedMarshal<void>
{
    static void Push(lua_State*, void*)
    {
    }

    static void Pop(lua_State*, int)
    {
    }
};

template <>
struct DefaultTypedMarshal<bool>
{
    static void Push(lua_State* L, bool v)
    {
        PrimitiveMarshal::PushBool(L, v);
    }

    static bool Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopBool(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<Il2CppChar>
{
    static void Push(lua_State* L, Il2CppChar v)
    {
        PrimitiveMarshal::PushUInt16(L, (uint16_t)v);
    }

    static Il2CppChar Pop(lua_State* L, int idx)
    {
        return (Il2CppChar)PrimitiveMarshal::PopUInt16(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<int8_t>
{
    static void Push(lua_State* L, int8_t v)
    {
        PrimitiveMarshal::PushInt8(L, v);
    }

    static int8_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopInt8(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<uint8_t>
{
    static void Push(lua_State* L, uint8_t v)
    {
        PrimitiveMarshal::PushUInt8(L, v);
    }

    static uint8_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopUInt8(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<int16_t>
{
    static void Push(lua_State* L, int16_t v)
    {
        PrimitiveMarshal::PushInt16(L, v);
    }

    static int16_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopInt16(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<uint16_t>
{
    static void Push(lua_State* L, uint16_t v)
    {
        PrimitiveMarshal::PushUInt16(L, v);
    }

    static uint16_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopUInt16(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<int32_t>
{
    static void Push(lua_State* L, int32_t v)
    {
        PrimitiveMarshal::PushInt32(L, v);
    }

    static int32_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopInt32(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<int64_t>
{
    static void Push(lua_State* L, int64_t v)
    {
        PrimitiveMarshal::PushInt64(L, v);
    }

    static int64_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopInt64(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<uint32_t>
{
    static void Push(lua_State* L, uint32_t v)
    {
        PrimitiveMarshal::PushUInt32(L, v);
    }

    static uint32_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopUInt32(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<uint64_t>
{
    static void Push(lua_State* L, uint64_t v)
    {
        PrimitiveMarshal::PushUInt64(L, v);
    }

    static uint64_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopUInt64(L, idx);
    }
};

// 64-bit LP64 (Apple/Android/Linux): intptr_t is long, distinct from int64_t (long long).
// Windows LLP64 and 32-bit: intptr_t aliases int64_t / int32_t — already specialized above.
#if INTPTR_MAX == INT64_MAX && !defined(_WIN32)
template <>
struct DefaultTypedMarshal<intptr_t>
{
    static void Push(lua_State* L, intptr_t v)
    {
        PrimitiveMarshal::PushIntPtr(L, v);
    }

    static intptr_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopIntPtr(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<uintptr_t>
{
    static void Push(lua_State* L, uintptr_t v)
    {
        PrimitiveMarshal::PushUIntPtr(L, v);
    }

    static uintptr_t Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopUIntPtr(L, idx);
    }
};
#endif

template <>
struct DefaultTypedMarshal<float>
{
    static void Push(lua_State* L, float v)
    {
        PrimitiveMarshal::PushFloat(L, v);
    }

    static float Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopFloat(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<double>
{
    static void Push(lua_State* L, double v)
    {
        PrimitiveMarshal::PushDouble(L, v);
    }

    static double Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopDouble(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<void*>
{
    static void Push(lua_State* L, void* v)
    {
        PrimitiveMarshal::PushPointer(L, v);
    }

    static void* Pop(lua_State* L, int idx)
    {
        return PrimitiveMarshal::PopPointer(L, idx);
    }
};

template <>
struct DefaultTypedMarshal<Il2CppString*>
{
    static void Push(lua_State* L, Il2CppString* v)
    {
        StringMarshal::Push(L, v);
    }

    static Il2CppString* Pop(lua_State* L, int idx)
    {
        return StringMarshal::Pop(L, idx);
    }
};

class TypedMarshal
{
  public:
    static void PushByType(lua_State* L, void* ptr, const Il2CppType* type);
    static void PopByType(lua_State* L, int idx, void* ptr, const Il2CppType* type);
};

} // namespace zlua
