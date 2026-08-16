// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <vector>

#include "TypeRegistryArray.h"

#include "MetaBinding.h"
#include "MetaTableCache.h"
#include "TypeRegistryCommon.h"

#include "../LuaConsts.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaUtil.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../marshal/StringMarshal.h"

#include "vm/Array.h"
#include "vm/Class.h"
#include "vm/Type.h"
#include "gc/GarbageCollector.h"

namespace zlua
{
static int ArrayInstanceLen(lua_State* L)
{
    Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, 1);
    IL2CPP_ASSERT(obj != nullptr);
    IL2CPP_ASSERT(IL2CPP_CLASS_IS_ARRAY(obj->klass));

    Il2CppArray* array = (Il2CppArray*)obj;

    lua_pushinteger(L, (lua_Integer)il2cpp::vm::Array::GetLength(array));
    return 1;
}

static bool TryReadIntIndex(lua_State* L, int keyIndex, int32_t* outIndex)
{
    lua_Integer value;
    if (LuaUtil::IsStrictLuaInteger(L, keyIndex, value))
    {
        *outIndex = (int32_t)value;
        return true;
    }
    return false;
}

static il2cpp_array_size_t ResolveFlatIndex(lua_State* L, Il2CppArray* array, int indexStart, int indexCount)
{
    Il2CppClass* arrayClass = array->klass;
    const int rank = arrayClass->rank;
    if (indexCount != rank)
        luaL_error(L, "zlua: expected %d index argument(s)", rank);

    if (rank == 1)
    {
        int32_t index = 0;
        if (!TryReadIntIndex(L, indexStart, &index))
            luaL_error(L, "zlua: expected integer index");
        return (il2cpp_array_size_t)index;
    }

    std::vector<int32_t> indices((size_t)rank);
    for (int i = 0; i < rank; ++i)
    {
        if (!TryReadIntIndex(L, indexStart + i, &indices[(size_t)i]))
            luaL_error(L, "zlua: expected integer indices");
    }
    return ArrayIndexFromIndices(array, indices.data());
}

static int PushArrayElement(lua_State* L, Il2CppArray* array, il2cpp_array_size_t arrayIndex)
{
    Il2CppClass* arrayClass = array->klass;
    Il2CppClass* elementClass = arrayClass->element_class;

    const il2cpp_array_size_t length = il2cpp::vm::Array::GetLength(array);
    if (arrayIndex >= length)
        return luaL_error(L, "zlua: array index out of range: %d", (int)arrayIndex);

    const int elementSize = il2cpp::vm::Array::GetElementSize(arrayClass);
    // il2cpp-object-internals.h: il2cpp_array_addr_with_size(arr, idx, size) — not (arr, size, idx).
    // (Unity vm/Array.h helpers often pass size/index swapped; multiply is commutative today.)
    void* elementAddress = il2cpp_array_addr_with_size(array, arrayIndex, elementSize);
    const Il2CppType* elementType = &elementClass->byval_arg;

    switch (elementType->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        PrimitiveMarshal::PushBool(L, *(bool*)elementAddress);
        return 1;
    case IL2CPP_TYPE_I1:
        PrimitiveMarshal::PushInt8(L, *(int8_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_U1:
        PrimitiveMarshal::PushUInt8(L, *(uint8_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_I2:
        PrimitiveMarshal::PushInt16(L, *(int16_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        PrimitiveMarshal::PushUInt16(L, *(uint16_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_I4:
        PrimitiveMarshal::PushInt32(L, *(int32_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_U4:
        PrimitiveMarshal::PushUInt32(L, *(uint32_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_I8:
        PrimitiveMarshal::PushInt64(L, *(int64_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_U8:
        PrimitiveMarshal::PushUInt64(L, *(uint64_t*)elementAddress);
        return 1;
    case IL2CPP_TYPE_R4:
        PrimitiveMarshal::PushFloat(L, *(float*)elementAddress);
        return 1;
    case IL2CPP_TYPE_R8:
        PrimitiveMarshal::PushDouble(L, *(double*)elementAddress);
        return 1;
    case IL2CPP_TYPE_STRING:
        StringMarshal::Push(L, *(Il2CppString**)elementAddress);
        return 1;
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_SZARRAY:
    case IL2CPP_TYPE_ARRAY:
        ObjectMarshal::Push(L, *(Il2CppObject**)elementAddress, elementClass);
        return 1;
    default:
        TypedMarshal::PushByType(L, elementAddress, elementType);
        return 1;
    }
}

static int SetArrayElement(lua_State* L, Il2CppArray* array, il2cpp_array_size_t arrayIndex, int valueIndex)
{
    Il2CppClass* arrayClass = array->klass;
    Il2CppClass* elementClass = arrayClass->element_class;
    if (!elementClass->initialized)
        il2cpp::vm::Class::Init(elementClass);

    const il2cpp_array_size_t length = il2cpp::vm::Array::GetLength(array);
    if (arrayIndex >= length)
        return luaL_error(L, "zlua: array index out of range: %d", (int)arrayIndex);

    const int elementSize = il2cpp::vm::Array::GetElementSize(arrayClass);
    // il2cpp-object-internals.h: il2cpp_array_addr_with_size(arr, idx, size)
    void* elementAddress = il2cpp_array_addr_with_size(array, arrayIndex, elementSize);
    const Il2CppType* elementType = &elementClass->byval_arg;

    switch (elementType->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        *(bool*)elementAddress = PrimitiveMarshal::PopBool(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_I1:
        *(int8_t*)elementAddress = PrimitiveMarshal::PopInt8(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_U1:
        *(uint8_t*)elementAddress = PrimitiveMarshal::PopUInt8(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_I2:
        *(int16_t*)elementAddress = PrimitiveMarshal::PopInt16(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        *(uint16_t*)elementAddress = PrimitiveMarshal::PopUInt16(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_I4:
        *(int32_t*)elementAddress = PrimitiveMarshal::PopInt32(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_U4:
        *(uint32_t*)elementAddress = PrimitiveMarshal::PopUInt32(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_I8:
        *(int64_t*)elementAddress = PrimitiveMarshal::PopInt64(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_U8:
        *(uint64_t*)elementAddress = PrimitiveMarshal::PopUInt64(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_R4:
        *(float*)elementAddress = PrimitiveMarshal::PopFloat(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_R8:
        *(double*)elementAddress = PrimitiveMarshal::PopDouble(L, valueIndex);
        return 0;
    case IL2CPP_TYPE_STRING:
        *(Il2CppString**)elementAddress = StringMarshal::Pop(L, valueIndex);
        il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)elementAddress);
        return 0;
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_SZARRAY:
    case IL2CPP_TYPE_ARRAY:
        *(Il2CppObject**)elementAddress = ObjectMarshal::Pop(L, valueIndex, elementClass);
        il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)elementAddress);
        return 0;
    default:
        TypedMarshal::PopByType(L, valueIndex, elementAddress, elementType);
        if (il2cpp::vm::Type::IsReference(elementType))
            il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)elementAddress);
        return 0;
    }
}

static Il2CppArray* GetArrayThis(lua_State* L, int index)
{
    return (Il2CppArray*)TypeRegistryCommon::GetByObjThis(L, index);
}

static int ArrayInstanceGet(lua_State* L)
{
    Il2CppArray* array = GetArrayThis(L, 1);

    const int rank = array->klass->rank;
    const int argCount = lua_gettop(L) - 1;
    if (argCount != rank)
        return luaL_error(L, "zlua: get expects %d index argument(s)", rank);
    const il2cpp_array_size_t flatIndex = ResolveFlatIndex(L, array, 2, argCount);
    return PushArrayElement(L, array, flatIndex);
}

static int ArrayInstanceSet(lua_State* L)
{
    Il2CppArray* array = GetArrayThis(L, 1);

    const int rank = array->klass->rank;
    const int argCount = lua_gettop(L) - 1;
    if (argCount != rank + 1)
        return luaL_error(L, "zlua: set expects %d index argument(s) and a value", rank);

    const int valueIndex = lua_gettop(L);
    const il2cpp_array_size_t flatIndex = ResolveFlatIndex(L, array, 2, rank);
    SetArrayElement(L, array, flatIndex, valueIndex);
    return 0;
}

static void RegisterArrayElementAccessMethods(lua_State* L, Il2CppClass* klass)
{
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    TypeRegistryCommon::RegisterNativeInstanceMethod(L, binding, "get", ArrayInstanceGet);
    TypeRegistryCommon::RegisterNativeInstanceMethod(L, binding, "set", ArrayInstanceSet);
}

void TypeRegistryArray::CreateTypeTable(lua_State* L, Il2CppClass* klass)
{
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int typeTableIndex = stackGuard.GetTop();
    TypeRegistryCommon::WriteCommonTypeFields(L, klass, typeTableIndex, binding);
    RegisterArrayElementAccessMethods(L, klass);

    TypeRegistryCommon::AttachReferenceInstanceMetatable(L, klass, typeTableIndex, binding);
    // both szarray and mdarray support __len
    lua_getfield(L, typeTableIndex, LuaConsts::ByObjInstanceMt);
    lua_pushcfunction(L, ArrayInstanceLen);
    lua_setfield(L, -2, LuaConsts::MetaLen);
    lua_pop(L, 1);

    TypeRegistryCommon::AttachStaticTypeMetatable(L, klass, typeTableIndex, nullptr, nullptr, nullptr, binding);
}
} // namespace zlua
