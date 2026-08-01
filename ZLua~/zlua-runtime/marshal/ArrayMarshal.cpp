#include <cstring>
#include <vector>

#include "ArrayMarshal.h"

#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"
#include "ObjectRegistry.h"
#include "ObjectMarshal.h"
#include "TypedMarshal.h"
#include "StringMarshal.h"

#include "gc/GarbageCollector.h"
#include "vm/Array.h"

namespace zlua
{

bool ArrayMarshal::TryGetConsecutiveTableLength(lua_State* L, int index, int& length)
{
    length = 0;
    if (!lua_istable(L, index))
    {
        return false;
    }

    for (int i = 1;; ++i)
    {
        lua_rawgeti(L, index, i);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            break;
        }
        lua_pop(L, 1);
        length = i;
    }
    return true;
}

bool ArrayMarshal::TryReadIntSequence(lua_State* L, int index, int expectedCount, std::vector<int32_t>& values)
{
    values.clear();
    if (!lua_istable(L, index))
    {
        return false;
    }

    values.resize((size_t)expectedCount);
    for (int i = 0; i < expectedCount; ++i)
    {
        lua_rawgeti(L, index, i + 1);
        if (!lua_isinteger(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        values[(size_t)i] = (int32_t)lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    return true;
}

void ArrayMarshal::PushAsArrayObject(lua_State* L, Il2CppArray* array, const MarshalMetaInfo* meta)
{
    ObjectMarshal::Push(L, array, meta);
}

Il2CppArray* ArrayMarshal::PopFromArrayObject(lua_State* L, int arrayIndex, Il2CppClass* klass)
{
    Il2CppArray* array = (Il2CppArray*)ObjectRegistry::Pop(L, arrayIndex);
    if (array != nullptr && array->klass != klass)
    {
        LuaException::ThrowFormat("zlua argument mismatch: array type mismatch");
    }
    return array;
}

void ArrayMarshal::PushAsBytes(lua_State* L, Il2CppArray* array)
{
    IL2CPP_ASSERT(array->klass == MetadataUtil::GetByteArrayClass());
    lua_pushlstring(L, (const char*)il2cpp_array_addr(array, uint8_t, 0), array->max_length);
}

Il2CppArray* ArrayMarshal::PopFromBytes(lua_State* L, int arrayIndex, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass == MetadataUtil::GetByteArrayClass());
    if (lua_isnil(L, arrayIndex))
    {
        return nullptr;
    }
    if (lua_type(L, arrayIndex) != LUA_TSTRING)
    {
        LuaException::ThrowFormat("zlua argument mismatch: expected string, got: %s", lua_typename(L, lua_type(L, arrayIndex)));
    }
    size_t length;
    const char* data = lua_tolstring(L, arrayIndex, &length);
    Il2CppArray* newArr = (Il2CppArray*)il2cpp::vm::Array::New(klass, length);
    if (length > 0)
        std::memcpy(il2cpp_array_addr(newArr, uint8_t, 0), data, length);
    return newArr;
}

template <typename T>
void PushAsPrimitiveTable(lua_State* L, Il2CppArray* array)
{
    IL2CPP_ASSERT(array->klass->rank == 1);
    lua_createtable(L, (int)array->max_length, 0);
    T* startAddr = (T*)il2cpp_array_addr(array, T, 0);
    for (il2cpp_array_size_t i = 0; i < array->max_length; ++i)
    {
        T value = startAddr[i];
        DefaultTypedMarshal<T>::Push(L, value);
        lua_rawseti(L, -2, i + 1);
    }
}

/// Length known: Pop each element directly into the managed array (GC-safe; no native staging of refs).
template <typename T, bool kNeedsWriteBarrier>
Il2CppArray* PopFromPrimitiveTable(lua_State* L, int arrayIndex, Il2CppClass* klass, int32_t length)
{
    Il2CppArray* newArray = (Il2CppArray*)il2cpp::vm::Array::NewSpecific(klass, (il2cpp_array_size_t)length);
    T* startAddr = (T*)il2cpp_array_addr(newArray, T, 0);
    for (int32_t i = 0; i < length; ++i)
    {
        lua_rawgeti(L, arrayIndex, i + 1);
        startAddr[i] = DefaultTypedMarshal<T>::Pop(L, -1);
        lua_pop(L, 1);
    }
    if (kNeedsWriteBarrier && length > 0)
    {
        il2cpp::gc::GarbageCollector::SetWriteBarrier(
            reinterpret_cast<void**>(startAddr), (size_t)length * sizeof(T));
    }
    return newArray;
}

void ArrayMarshal::PushAsTable(lua_State* L, Il2CppArray* array)
{
    if (array == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    if (array->klass->byval_arg.type == IL2CPP_TYPE_ARRAY)
    {
        LuaException::ThrowFormat("zlua not supported: mdarray cannot be pushed as a table");
    }
    Il2CppClass* elementKlass = array->klass->element_class;
    const Il2CppType* elementType = &elementKlass->byval_arg;
    switch (elementType->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        PushAsPrimitiveTable<bool>(L, array);
        break;
    case IL2CPP_TYPE_CHAR:
        PushAsPrimitiveTable<uint16_t>(L, array);
        break;
    case IL2CPP_TYPE_I1:
        PushAsPrimitiveTable<int8_t>(L, array);
        break;
    case IL2CPP_TYPE_U1:
        PushAsPrimitiveTable<uint8_t>(L, array);
        break;
    case IL2CPP_TYPE_I2:
        PushAsPrimitiveTable<int16_t>(L, array);
        break;
    case IL2CPP_TYPE_U2:
        PushAsPrimitiveTable<uint16_t>(L, array);
        break;
    case IL2CPP_TYPE_I4:
        PushAsPrimitiveTable<int32_t>(L, array);
        break;
    case IL2CPP_TYPE_U4:
        PushAsPrimitiveTable<uint32_t>(L, array);
        break;
    case IL2CPP_TYPE_I8:
        PushAsPrimitiveTable<int64_t>(L, array);
        break;
    case IL2CPP_TYPE_U8:
        PushAsPrimitiveTable<uint64_t>(L, array);
        break;
    case IL2CPP_TYPE_R4:
        PushAsPrimitiveTable<float>(L, array);
        break;
    case IL2CPP_TYPE_R8:
        PushAsPrimitiveTable<double>(L, array);
        break;
    case IL2CPP_TYPE_STRING:
        PushAsPrimitiveTable<Il2CppString*>(L, array);
        break;
    default:
    {
        lua_createtable(L, (int)array->max_length, 0);
        int32_t elementSize = il2cpp_array_element_size(array->klass);
        for (il2cpp_array_size_t i = 0; i < array->max_length; ++i)
        {
            void* valueAddr = il2cpp_array_addr_with_size(array, i, elementSize);
            TypedMarshal::PushByType(L, valueAddr, elementType);
            lua_rawseti(L, -2, i + 1);
        }
        break;
    }
    }
}

Il2CppArray* ArrayMarshal::PopFromTable(lua_State* L, int arrayIndex, Il2CppClass* klass)
{
    if (klass->byval_arg.type == IL2CPP_TYPE_ARRAY)
    {
        LuaException::ThrowFormat("zlua not supported: mdarray cannot be popped from a table");
    }
    IL2CPP_ASSERT(klass->rank == 1);
    IL2CPP_ASSERT(klass->byval_arg.type == IL2CPP_TYPE_SZARRAY);
    if (lua_isnil(L, arrayIndex))
    {
        return nullptr;
    }

    int32_t length = 0;
    if (!TryGetConsecutiveTableLength(L, arrayIndex, length))
    {
        LuaException::ThrowFormat("zlua argument mismatch: table value must be a table");
    }

    Il2CppClass* elementKlass = klass->element_class;
    const Il2CppType* elementType = &elementKlass->byval_arg;

    switch (elementType->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        return PopFromPrimitiveTable<bool, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_CHAR:
        return PopFromPrimitiveTable<uint16_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_I1:
        return PopFromPrimitiveTable<int8_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_U1:
        return PopFromPrimitiveTable<uint8_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_I2:
        return PopFromPrimitiveTable<int16_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_U2:
        return PopFromPrimitiveTable<uint16_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_I4:
        return PopFromPrimitiveTable<int32_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_U4:
        return PopFromPrimitiveTable<uint32_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_I8:
        return PopFromPrimitiveTable<int64_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_U8:
        return PopFromPrimitiveTable<uint64_t, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_R4:
        return PopFromPrimitiveTable<float, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_R8:
        return PopFromPrimitiveTable<double, false>(L, arrayIndex, klass, length);
    case IL2CPP_TYPE_STRING:
        return PopFromPrimitiveTable<Il2CppString*, true>(L, arrayIndex, klass, length);
    default:
    {
        const int elementSize = il2cpp_array_element_size(klass);
        Il2CppArray* newArray = (Il2CppArray*)il2cpp::vm::Array::NewSpecific(klass, (il2cpp_array_size_t)length);
        const bool needsWriteBarrier = !elementKlass->byval_arg.valuetype || !elementKlass->is_blittable;
        for (int32_t i = 0; i < length; ++i)
        {
            lua_rawgeti(L, arrayIndex, i + 1);
            void* valueAddr = il2cpp_array_addr_with_size(newArray, i, elementSize);
            TypedMarshal::PopByType(L, -1, valueAddr, elementType);
            lua_pop(L, 1);
            if (needsWriteBarrier)
            {
                il2cpp::gc::GarbageCollector::SetWriteBarrier(
                    reinterpret_cast<void**>(valueAddr), (size_t)elementSize);
            }
        }
        return newArray;
    }
    }
}

Il2CppArray* ArrayMarshal::PopFromArrayObjectOrTable(lua_State* L, int arrayIndex, Il2CppClass* klass)
{
    int type = lua_type(L, arrayIndex);
    switch (type)
    {
    case LUA_TNIL:
        return nullptr;
    case LUA_TTABLE:
        return PopFromTable(L, arrayIndex, klass);
    case LUA_TUSERDATA:
        return PopFromArrayObject(L, arrayIndex, klass);
    default:
        LuaException::ThrowFormat("zlua argument mismatch: expected array or table, got: %s", lua_typename(L, type));
    }
}

} // namespace zlua
