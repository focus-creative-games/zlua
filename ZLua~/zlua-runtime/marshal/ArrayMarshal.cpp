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

// Process-level scratch for leaf primitive/string table→array Pop only.
// Must NOT be used when PopByType can re-enter ArrayMarshal (e.g. T[][]), or nested
// calls will overwrite the outer scan buffer.
static std::vector<uint8_t> s_tablePopScratch;

static void EnsureTablePopScratchBytes(size_t bytes)
{
    if (s_tablePopScratch.size() < bytes)
        s_tablePopScratch.resize(bytes);
}

static size_t TableSequenceLenHint(lua_State* L, int index)
{
#if ZLUA_USE_LUAJIT || (ZLUA_LUA_API_FAMILY < 502)
    return (size_t)lua_objlen(L, index);
#else
    return (size_t)lua_rawlen(L, index);
#endif
}

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

/// Leaf primitive/string only — uses process scratch (DefaultTypedMarshal does not re-enter ArrayMarshal).
template <typename T, bool kNeedsWriteBarrier>
Il2CppArray* PopFromPrimitiveTableSinglePass(lua_State* L, int arrayIndex, Il2CppClass* klass)
{
    if (!lua_istable(L, arrayIndex))
    {
        LuaException::ThrowFormat("zlua argument mismatch: table value must be a table");
    }

    const size_t elemSize = sizeof(T);
    size_t hint = TableSequenceLenHint(L, arrayIndex);
    if (hint < 16)
        hint = 16;
    // Pre-size capacity without changing logical size; avoid per-call heap churn.
    if (s_tablePopScratch.capacity() < hint * elemSize)
        s_tablePopScratch.reserve(hint * elemSize);

    size_t count = 0;
    for (int i = 1;; ++i)
    {
        lua_rawgeti(L, arrayIndex, i);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            break;
        }
        EnsureTablePopScratchBytes((count + 1) * elemSize);
        T* slot = reinterpret_cast<T*>(&s_tablePopScratch[0] + count * elemSize);
        *slot = DefaultTypedMarshal<T>::Pop(L, -1);
        lua_pop(L, 1);
        ++count;
    }

    Il2CppArray* newArray = (Il2CppArray*)il2cpp::vm::Array::NewSpecific(klass, (il2cpp_array_size_t)count);
    if (count > 0)
    {
        T* startAddr = (T*)il2cpp_array_addr(newArray, T, 0);
        std::memcpy(startAddr, &s_tablePopScratch[0], count * elemSize);
        if (kNeedsWriteBarrier)
        {
            il2cpp::gc::GarbageCollector::SetWriteBarrier(
                reinterpret_cast<void**>(startAddr), count * elemSize);
        }
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
    Il2CppClass* elementKlass = klass->element_class;
    const Il2CppType* elementType = &elementKlass->byval_arg;

    switch (elementType->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        return PopFromPrimitiveTableSinglePass<bool, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_CHAR:
        return PopFromPrimitiveTableSinglePass<uint16_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_I1:
        return PopFromPrimitiveTableSinglePass<int8_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_U1:
        return PopFromPrimitiveTableSinglePass<uint8_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_I2:
        return PopFromPrimitiveTableSinglePass<int16_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_U2:
        return PopFromPrimitiveTableSinglePass<uint16_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_I4:
        return PopFromPrimitiveTableSinglePass<int32_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_U4:
        return PopFromPrimitiveTableSinglePass<uint32_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_I8:
        return PopFromPrimitiveTableSinglePass<int64_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_U8:
        return PopFromPrimitiveTableSinglePass<uint64_t, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_R4:
        return PopFromPrimitiveTableSinglePass<float, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_R8:
        return PopFromPrimitiveTableSinglePass<double, false>(L, arrayIndex, klass);
    case IL2CPP_TYPE_STRING:
        return PopFromPrimitiveTableSinglePass<Il2CppString*, true>(L, arrayIndex, klass);
    default:
    {
        if (!lua_istable(L, arrayIndex))
        {
            LuaException::ThrowFormat("zlua argument mismatch: table value must be a table");
        }

        // Local buffer: element Pop may recurse into ArrayMarshal (nested arrays / composites).
        const int elementSize = il2cpp_array_element_size(klass);
        size_t hint = TableSequenceLenHint(L, arrayIndex);
        if (hint < 16)
            hint = 16;

        std::vector<uint8_t> bytes;
        bytes.reserve(hint * (size_t)elementSize);

        int32_t length = 0;
        for (int i = 1;; ++i)
        {
            lua_rawgeti(L, arrayIndex, i);
            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                break;
            }
            bytes.resize((size_t)i * (size_t)elementSize);
            void* dest = &bytes[0] + (size_t)(i - 1) * (size_t)elementSize;
            TypedMarshal::PopByType(L, -1, dest, elementType);
            lua_pop(L, 1);
            length = i;
        }

        Il2CppArray* newArray = (Il2CppArray*)il2cpp::vm::Array::NewSpecific(klass, (il2cpp_array_size_t)length);
        if (length > 0)
        {
            void* dest = il2cpp_array_addr_with_size(newArray, 0, elementSize);
            std::memcpy(dest, &bytes[0], (size_t)length * (size_t)elementSize);
            if (!elementKlass->byval_arg.valuetype || !elementKlass->is_blittable)
            {
                il2cpp::gc::GarbageCollector::SetWriteBarrier(
                    reinterpret_cast<void**>(dest), (size_t)length * (size_t)elementSize);
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
