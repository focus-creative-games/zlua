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
struct DefaultMarshaling;

template <>
struct DefaultMarshaling<void>
{
    static void Push(lua_State*, void*)
    {
    }

    static void Pop(lua_State*, int)
    {
    }
};

template <>
struct DefaultMarshaling<bool>
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
struct DefaultMarshaling<Il2CppChar>
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
struct DefaultMarshaling<int8_t>
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
struct DefaultMarshaling<uint8_t>
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
struct DefaultMarshaling<int16_t>
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
struct DefaultMarshaling<uint16_t>
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
struct DefaultMarshaling<int32_t>
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
struct DefaultMarshaling<int64_t>
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
struct DefaultMarshaling<uint32_t>
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
struct DefaultMarshaling<uint64_t>
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

template <>
struct DefaultMarshaling<float>
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
struct DefaultMarshaling<double>
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
struct DefaultMarshaling<void*>
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
struct DefaultMarshaling<Il2CppString*>
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

class Marshaling
{
  public:
    static void PushByType(lua_State* L, void* ptr, const Il2CppType* type);
    static void PopByType(lua_State* L, int idx, void* ptr, const Il2CppType* type);
};

} // namespace zlua
