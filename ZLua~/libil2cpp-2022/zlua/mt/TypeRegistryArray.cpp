#include <vector>

#include "TypeRegistryArray.h"

#include "MetaBinding.h"
#include "MetaTableCache.h"
#include "TypeRegistryCommon.h"

#include "../LuaConsts.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaUtil.h"
#include "../marshal/Marshaling.h"
#include "../marshal/ObjectMarshal.h"

#include "vm/Array.h"
#include "vm/Class.h"
#include "vm/Type.h"

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
    return il2cpp::vm::Array::IndexFromIndices(array, indices.data());
}

static int PushArrayElement(lua_State* L, Il2CppArray* array, il2cpp_array_size_t arrayIndex)
{
    Il2CppClass* arrayClass = array->klass;
    Il2CppClass* elementClass = arrayClass->element_class;
    il2cpp::vm::Class::Init(elementClass);

    const il2cpp_array_size_t length = il2cpp::vm::Array::GetLength(array);
    if (arrayIndex >= length)
        return luaL_error(L, "zlua: array index out of range: %d", (int)arrayIndex);

    const int elementSize = il2cpp::vm::Array::GetElementSize(arrayClass);
    void* elementAddress = il2cpp_array_addr_with_size(array, elementSize, arrayIndex);
    const Il2CppType* elementType = &elementClass->byval_arg;
    Marshaling::PushByType(L, elementAddress, elementType);
    return 1;
}

static int SetArrayElement(lua_State* L, Il2CppArray* array, il2cpp_array_size_t arrayIndex, int valueIndex)
{
    Il2CppClass* arrayClass = array->klass;
    Il2CppClass* elementClass = arrayClass->element_class;
    il2cpp::vm::Class::Init(elementClass);

    const il2cpp_array_size_t length = il2cpp::vm::Array::GetLength(array);
    if (arrayIndex >= length)
        return luaL_error(L, "zlua: array index out of range: %d", (int)arrayIndex);

    const int elementSize = il2cpp::vm::Array::GetElementSize(arrayClass);
    void* elementAddress = il2cpp_array_addr_with_size(array, elementSize, arrayIndex);
    const Il2CppType* elementType = &elementClass->byval_arg;

    Marshaling::PopByType(L, valueIndex, elementAddress, elementType);

    if (il2cpp::vm::Type::IsReference(elementType))
        il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)elementAddress);

    return 0;
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
