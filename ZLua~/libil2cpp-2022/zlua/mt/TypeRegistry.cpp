#include "TypeRegistry.h"

#include "MetaBinding.h"
#include "MetaTableCache.h"
#include "../MetadataUtil.h"
#include "../methodbridge/MethodBridge.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/ValueMarshaling.h"

#include "il2cpp-class-internals.h"
#include "lua/lauxlib.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Object.h"
#include "vm/Type.h"
#include "vm/Array.h"
#include "gc/GarbageCollector.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace zlua
{
static std::unordered_map<Il2CppClass*, int> s_internedTypeTableRefs;
static int TypeTableToString(lua_State* L)
{
    lua_getfield(L, 1, "__fullname");
    if (!lua_isstring(L, -1))
        lua_pushstring(L, "CSharp.Type");
    return 1;
}

static int InvokeStructDefault(lua_State* L)
{
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
    if (klass == nullptr)
        return luaL_error(L, "zlua: invalid struct type");

    if (lua_gettop(L) != 0)
        return luaL_error(L, "zlua: %s._default expects no arguments", MetadataUtil::GetTypeFullName(klass));

    Il2CppObject* instance = il2cpp::vm::Object::New(klass);
    ValueMarshaling::PushStructInstance(L, klass, instance);
    return 1;
}

static int CreateTypeInstance(lua_State* L)
{
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
    if (klass == nullptr)
        return luaL_error(L, "zlua: invalid type binding");

    int argStartIdx = 2;
    int argCount = lua_gettop(L) - argStartIdx + 1;
    const MethodInfo* ctor = ValueMarshaling::FindMatchingConstructor(klass, L, argStartIdx, argCount);
    if (argCount > 0 && ctor == nullptr)
        return luaL_error(L, "zlua: no constructor found for type: %s", klass->name);

    return MethodBridge::InvokeConstructor(L, ctor, klass, argStartIdx, argCount);
}

static int ArrayInstanceLen(lua_State* L)
{
    Il2CppObject* obj = ObjectMarshal::Pop(L, 1);
    if (obj == nullptr || !IL2CPP_CLASS_IS_ARRAY(obj->klass))
        return luaL_error(L, "zlua: __len expects szarray userdata");

    Il2CppArray* array = (Il2CppArray*)obj;
    if (array->klass->rank != 1)
        return luaL_error(L, "zlua: __len only supported for rank-1 arrays");

    lua_pushinteger(L, (lua_Integer)il2cpp::vm::Array::GetLength(array));
    return 1;
}

static bool TryReadIntIndex(lua_State* L, int keyIndex, int32_t* outIndex)
{
    if (lua_isinteger(L, keyIndex))
    {
        *outIndex = (int32_t)lua_tointeger(L, keyIndex);
        return true;
    }

    if (lua_type(L, keyIndex) == LUA_TNUMBER)
    {
        const lua_Number n = lua_tonumber(L, keyIndex);
        if (n != (lua_Number)(lua_Integer)n)
            return false;
        *outIndex = (int32_t)(lua_Integer)n;
        return true;
    }

    return false;
}

static Il2CppArray* GetArrayFromStack(lua_State* L, int index)
{
    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr || !IL2CPP_CLASS_IS_ARRAY(obj->klass))
        return nullptr;
    return (Il2CppArray*)obj;
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

    if (il2cpp::vm::Type::IsReference(elementType))
    {
        ObjectMarshal::Push(L, *(Il2CppObject**)elementAddress);
        return 1;
    }

    if (elementType->type == IL2CPP_TYPE_VALUETYPE || elementType->type == IL2CPP_TYPE_GENERICINST)
    {
        if (elementClass->enumtype)
        {
            const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(elementClass);
            if (PrimitiveMarshal::PushBoxedReturn(L, baseType, elementAddress) == 0)
                return luaL_error(L, "zlua: failed to read array element");
            return 1;
        }
        if (ValueMarshaling::IsStructClass(elementClass))
        {
            StructMarshal::PushValue(L, elementAddress, elementClass);
            return 1;
        }
    }

    if (PrimitiveMarshal::PushBoxedReturn(L, elementType, elementAddress) == 0)
        return luaL_error(L, "zlua: unsupported array element type");
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

    if (!ValueMarshaling::TryPop(L, valueIndex, elementType, elementAddress, (size_t)elementSize))
        return luaL_error(L, "zlua: argument mismatch");

    if (il2cpp::vm::Type::IsReference(elementType))
        il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)elementAddress);

    return 0;
}

static int ArrayInstanceGet(lua_State* L)
{
    Il2CppArray* array = GetArrayFromStack(L, 1);
    if (array == nullptr)
        return luaL_error(L, "zlua: get expects array userdata");

    const int indexCount = lua_gettop(L) - 1;
    const il2cpp_array_size_t flatIndex = ResolveFlatIndex(L, array, 2, indexCount);
    return PushArrayElement(L, array, flatIndex);
}

static int ArrayInstanceSet(lua_State* L)
{
    Il2CppArray* array = GetArrayFromStack(L, 1);
    if (array == nullptr)
        return luaL_error(L, "zlua: set expects array userdata");

    const int rank = array->klass->rank;
    const int argCount = lua_gettop(L) - 1;
    if (argCount != rank + 1)
        return luaL_error(L, "zlua: set expects %d index argument(s) and a value", rank);

    const int valueIndex = lua_gettop(L);
    const il2cpp_array_size_t flatIndex = ResolveFlatIndex(L, array, 2, rank);
    SetArrayElement(L, array, flatIndex, valueIndex);
    return 0;
}

static void RegisterNativeInstanceMethod(
    lua_State* L,
    TypeBinding* binding,
    const char* name,
    lua_CFunction fn)
{
    if (binding->instanceMap.find(name) != binding->instanceMap.end())
        luaL_error(L, "zlua: duplicate instance method: %s", name);

    lua_pushcfunction(L, fn);
    MetaInfo info = {};
    info.kind = MetaKind::Method;
    info.method.closureRef = luaL_ref(L, LUA_REGISTRYINDEX);
    binding->instanceMap[name] = info;
}

static void RegisterArrayElementAccessMethods(lua_State* L, Il2CppClass* klass)
{
    TypeBinding* binding = MetaBinding::EnsureBinding(L, klass);
    RegisterNativeInstanceMethod(L, binding, "get", ArrayInstanceGet);
    RegisterNativeInstanceMethod(L, binding, "set", ArrayInstanceSet);
}

static void RegisterEnumConstants(lua_State* L, Il2CppClass* klass, int typeTableIndex)
{
    const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(klass);
    if (baseType == nullptr)
        return;

    void* fieldIter = nullptr;
    FieldInfo* field;
    while ((field = il2cpp::vm::Class::GetFields(klass, &fieldIter)) != nullptr)
    {
        const int flags = il2cpp::vm::Field::GetFlags(field);
        if ((flags & FIELD_ATTRIBUTE_LITERAL) == 0)
            continue;
        if ((flags & FIELD_ATTRIBUTE_STATIC) == 0)
            continue;
        if (strcmp(field->name, "value__") == 0)
            continue;

        std::vector<uint8_t> storage(8, 0);
        il2cpp::vm::Field::GetDefaultFieldValue(field, storage.data());
        if (PrimitiveMarshal::PushBoxedReturn(L, baseType, storage.data()) == 0)
            continue;
        lua_setfield(L, typeTableIndex, field->name);
    }
}

static int InvokeEnumCall(lua_State* L)
{
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
    if (klass == nullptr || !klass->enumtype)
        return luaL_error(L, "zlua: invalid enum type");

    if (lua_gettop(L) != 2)
        return luaL_error(L, "zlua: %s expects underlying integer value", MetadataUtil::GetTypeFullName(klass));

    const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(klass);
    const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(klass);

    std::vector<uint8_t> storage(payloadSize, 0);
    if (!PrimitiveMarshal::TryPop(L, 2, baseType, storage.data(), storage.size()))
        return luaL_error(L, "zlua: %s expects underlying integer value", MetadataUtil::GetTypeFullName(klass));

    StructMarshal::PushValue(L, storage.data(), klass);
    return 1;
}

static int EnumInstanceToString(lua_State* L)
{
    ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, 1);
    if (header == nullptr || header->klass == nullptr || !header->klass->enumtype)
        return luaL_error(L, "zlua: invalid enum userdata");

    const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(header->klass);
    if (baseType == nullptr)
        return luaL_error(L, "zlua: enum base type missing");

    int64_t value = 0;
    switch (baseType->type)
    {
    case IL2CPP_TYPE_I1:
        value = *reinterpret_cast<int8_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_U1:
        value = *reinterpret_cast<uint8_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_I2:
        value = *reinterpret_cast<int16_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        value = *reinterpret_cast<uint16_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_I4:
        value = *reinterpret_cast<int32_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_U4:
        value = *reinterpret_cast<uint32_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_I8:
        value = *reinterpret_cast<int64_t*>(header->Payload());
        break;
    case IL2CPP_TYPE_U8:
        value = *reinterpret_cast<uint64_t*>(header->Payload());
        break;
    default:
        return luaL_error(L, "zlua: unsupported enum underlying type");
    }

    const std::string fullName = MetadataUtil::GetLuaFullName(header->klass);
    const std::string text = fullName + "(" + std::to_string(value) + ")";
    lua_pushlstring(L, text.c_str(), text.size());
    return 1;
}

static void WriteCommonTypeFields(lua_State* L, Il2CppClass* klass, int typeTableIndex)
{
    MetaBinding::EnsureBinding(L, klass);
    il2cpp::vm::Class::Init(klass);

    const std::string fullName = MetadataUtil::GetLuaFullName(klass);
    lua_pushstring(L, fullName.c_str());
    lua_setfield(L, typeTableIndex, "__fullname");
    lua_pushlightuserdata(L, klass);
    lua_setfield(L, typeTableIndex, "__klass");
}

static void AttachStaticTypeMetatable(
    lua_State* L,
    Il2CppClass* klass,
    int typeTableIndex,
    lua_CFunction callFn,
    lua_CFunction extraFn,
    const char* extraFieldName)
{
    lua_newtable(L);
    const int smtIndex = lua_gettop(L);

    lua_pushlightuserdata(L, klass);
    lua_pushcclosure(L, callFn, 1);
    lua_setfield(L, smtIndex, "__call");

    if (extraFn != nullptr && extraFieldName != nullptr)
    {
        lua_pushlightuserdata(L, klass);
        lua_pushcclosure(L, extraFn, 1);
        lua_setfield(L, smtIndex, extraFieldName);
    }

    lua_pushcfunction(L, TypeTableToString);
    lua_setfield(L, smtIndex, "__tostring");

    MetaBinding::AttachStaticMetatable(L, klass, smtIndex);
    lua_setmetatable(L, typeTableIndex);
}

static void RegisterInstanceMetatableField(
    lua_State* L,
    Il2CppClass* klass,
    int typeTableIndex,
    const char* fieldName,
    void (*registerFn)(lua_State*, Il2CppClass*, int))
{
    lua_getfield(L, typeTableIndex, fieldName);
    registerFn(L, klass, -1);
    lua_pop(L, 1);
}

static void CreateReferenceTypeTable(lua_State* L, Il2CppClass* klass)
{
    IL2CPP_ASSERT(il2cpp::vm::Type::IsReference(&klass->byval_arg));

    lua_newtable(L);
    const int typeTableIndex = lua_gettop(L);
    WriteCommonTypeFields(L, klass, typeTableIndex);

    MetaBinding::PushReferenceInstanceMetatable(L, klass, typeTableIndex);
    lua_setfield(L, typeTableIndex, "__instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__instance_mt", MetaTableCache::RegisterReferenceMetatable);

    AttachStaticTypeMetatable(L, klass, typeTableIndex, CreateTypeInstance, nullptr, nullptr);
    lua_settop(L, typeTableIndex);
}

static void CreateValueTypeTable(lua_State* L, Il2CppClass* klass)
{
    lua_newtable(L);
    const int typeTableIndex = lua_gettop(L);
    WriteCommonTypeFields(L, klass, typeTableIndex);

    lua_pushboolean(L, 1);
    lua_setfield(L, typeTableIndex, "__struct");

    MetaBinding::PushByValInstanceMetatable(L, klass, typeTableIndex);
    lua_setfield(L, typeTableIndex, "__instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__instance_mt", MetaTableCache::RegisterByValMetatable);

    MetaBinding::PushByObjInstanceMetatable(L, klass, typeTableIndex);
    lua_setfield(L, typeTableIndex, "__byobj_instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__byobj_instance_mt", MetaTableCache::RegisterByObjMetatable);

    AttachStaticTypeMetatable(L, klass, typeTableIndex, CreateTypeInstance, InvokeStructDefault, "_default");
    lua_settop(L, typeTableIndex);
}

static void CreateEnumTypeTable(lua_State* L, Il2CppClass* klass)
{
    lua_newtable(L);
    const int typeTableIndex = lua_gettop(L);
    WriteCommonTypeFields(L, klass, typeTableIndex);

    lua_pushboolean(L, 1);
    lua_setfield(L, typeTableIndex, "__enum");

    MetaBinding::PushByValInstanceMetatable(L, klass, typeTableIndex, EnumInstanceToString);
    lua_setfield(L, typeTableIndex, "__instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__instance_mt", MetaTableCache::RegisterByValMetatable);

    MetaBinding::PushByObjInstanceMetatable(L, klass, typeTableIndex);
    lua_setfield(L, typeTableIndex, "__byobj_instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__byobj_instance_mt", MetaTableCache::RegisterByObjMetatable);

    RegisterEnumConstants(L, klass, typeTableIndex);
    AttachStaticTypeMetatable(L, klass, typeTableIndex, InvokeEnumCall, nullptr, nullptr);
    lua_settop(L, typeTableIndex);
}

static void CreateArrayTypeTable(lua_State* L, Il2CppClass* klass)
{
    lua_newtable(L);
    const int typeTableIndex = lua_gettop(L);
    WriteCommonTypeFields(L, klass, typeTableIndex);

    MetaBinding::PushReferenceInstanceMetatable(L, klass, typeTableIndex);
    RegisterArrayElementAccessMethods(L, klass);
    if (klass->rank == 1)
    {
        lua_pushcfunction(L, ArrayInstanceLen);
        lua_setfield(L, -2, "__len");
    }
    lua_setfield(L, typeTableIndex, "__instance_mt");
    RegisterInstanceMetatableField(L, klass, typeTableIndex, "__instance_mt", MetaTableCache::RegisterReferenceMetatable);

    AttachStaticTypeMetatable(L, klass, typeTableIndex, CreateTypeInstance, nullptr, nullptr);
    lua_settop(L, typeTableIndex);
}

void TypeRegistry::PushTypeTable(lua_State* L, Il2CppClass* klass)
{
    if (klass == nullptr)
        luaL_error(L, "zlua: invalid type");

    il2cpp::vm::Class::Init(klass);

    if (klass->enumtype)
        CreateEnumTypeTable(L, klass);
    else if (ValueMarshaling::IsStructClass(klass))
        CreateValueTypeTable(L, klass);
    else if (IL2CPP_CLASS_IS_ARRAY(klass))
        CreateArrayTypeTable(L, klass);
    else
        CreateReferenceTypeTable(L, klass);
}

void TypeRegistry::PushInternedTypeTable(lua_State* L, Il2CppClass* klass)
{
    if (klass == nullptr)
        luaL_error(L, "zlua: invalid type");

    std::unordered_map<Il2CppClass*, int>::iterator it = s_internedTypeTableRefs.find(klass);
    if (it != s_internedTypeTableRefs.end() && it->second != LUA_NOREF)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
        if (lua_istable(L, -1))
            return;
        lua_pop(L, 1);
    }

    PushTypeTable(L, klass);
    const int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    s_internedTypeTableRefs[klass] = ref;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
}

Il2CppClass* TypeRegistry::GetClassFromTypeTable(lua_State* L, int index)
{
    lua_getfield(L, index, "__klass");
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return klass;
}
} // namespace zlua
