#include "PrimitiveMarshal.h"

#include "../LuaUtil.h"
#include "il2cpp-tabledefs.h"

namespace zlua
{
    bool PrimitiveMarshal::CanConvert(lua_State* L, int index, const Il2CppType* type)    {
        if (type == nullptr)
            return false;

        switch (type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            return lua_isboolean(L, index);
        case IL2CPP_TYPE_I1:
        case IL2CPP_TYPE_U1:
        case IL2CPP_TYPE_I2:
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
        case IL2CPP_TYPE_I4:
        case IL2CPP_TYPE_U4:
        case IL2CPP_TYPE_I8:
        case IL2CPP_TYPE_U8:
        case IL2CPP_TYPE_I:
        case IL2CPP_TYPE_U:
            return LuaUtil::IsStrictLuaInteger(L, index) || lua_isnumber(L, index);
        case IL2CPP_TYPE_R4:
        case IL2CPP_TYPE_R8:
            return lua_isnumber(L, index) || lua_isinteger(L, index);
        default:
            return false;
        }
    }

    bool PrimitiveMarshal::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
    {
        if (!CanConvert(L, index, type))
            return false;

        switch (type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            if (destSize < sizeof(uint8_t))
                return false;
            *reinterpret_cast<uint8_t*>(dest) = PopBool(L, index) ? 1 : 0;
            return true;
        case IL2CPP_TYPE_I1:
            if (destSize < sizeof(int8_t))
                return false;
            *reinterpret_cast<int8_t*>(dest) = (int8_t)PopInt32(L, index);
            return true;
        case IL2CPP_TYPE_U1:
            if (destSize < sizeof(uint8_t))
                return false;
            *reinterpret_cast<uint8_t*>(dest) = (uint8_t)PopInt32(L, index);
            return true;
        case IL2CPP_TYPE_I2:
            if (destSize < sizeof(int16_t))
                return false;
            *reinterpret_cast<int16_t*>(dest) = (int16_t)PopInt32(L, index);
            return true;
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
            if (destSize < sizeof(uint16_t))
                return false;
            *reinterpret_cast<uint16_t*>(dest) = (uint16_t)PopInt32(L, index);
            return true;
        case IL2CPP_TYPE_I4:
            if (destSize < sizeof(int32_t))
                return false;
            *reinterpret_cast<int32_t*>(dest) = PopInt32(L, index);
            return true;
        case IL2CPP_TYPE_U4:
            if (destSize < sizeof(uint32_t))
                return false;
            *reinterpret_cast<uint32_t*>(dest) = PopUInt32(L, index);
            return true;
        case IL2CPP_TYPE_I8:
            if (destSize < sizeof(int64_t))
                return false;
            *reinterpret_cast<int64_t*>(dest) = PopInt64(L, index);
            return true;
        case IL2CPP_TYPE_U8:
            if (destSize < sizeof(uint64_t))
                return false;
            *reinterpret_cast<uint64_t*>(dest) = PopUInt64(L, index);
            return true;
        case IL2CPP_TYPE_I:
            if (destSize < sizeof(intptr_t))
                return false;
            *reinterpret_cast<intptr_t*>(dest) = (intptr_t)PopInt64(L, index);
            return true;
        case IL2CPP_TYPE_U:
            if (destSize < sizeof(uintptr_t))
                return false;
            *reinterpret_cast<uintptr_t*>(dest) = (uintptr_t)PopUInt64(L, index);
            return true;
        case IL2CPP_TYPE_R4:
            if (destSize < sizeof(float))
                return false;
            *reinterpret_cast<float*>(dest) = PopFloat(L, index);
            return true;
        case IL2CPP_TYPE_R8:
            if (destSize < sizeof(double))
                return false;
            *reinterpret_cast<double*>(dest) = PopDouble(L, index);
            return true;
        default:
            return false;
        }
    }

    int PrimitiveMarshal::PushBoxedReturn(lua_State* L, const Il2CppType* returnType, void* boxedData)
    {
        if (returnType == nullptr || boxedData == nullptr)
            return 0;

        switch (returnType->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            PushBool(L, *reinterpret_cast<uint8_t*>(boxedData) != 0);
            return 1;
        case IL2CPP_TYPE_I1:
            PushInt32(L, *reinterpret_cast<int8_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_U1:
            PushInt32(L, *reinterpret_cast<uint8_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_I2:
            PushInt32(L, *reinterpret_cast<int16_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
            PushInt32(L, *reinterpret_cast<uint16_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_I4:
            PushInt32(L, *reinterpret_cast<int32_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_U4:
            PushUInt32(L, *reinterpret_cast<uint32_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_I8:
        case IL2CPP_TYPE_U8:
            PushInt64(L, (int64_t)*reinterpret_cast<int64_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_I:
            PushInt64(L, (int64_t)*reinterpret_cast<intptr_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_U:
            PushInt64(L, (int64_t)*reinterpret_cast<uintptr_t*>(boxedData));
            return 1;
        case IL2CPP_TYPE_R4:
            PushFloat(L, *reinterpret_cast<float*>(boxedData));
            return 1;
        case IL2CPP_TYPE_R8:
            PushDouble(L, *reinterpret_cast<double*>(boxedData));
            return 1;
        default:
            return 0;
        }
    }
}
