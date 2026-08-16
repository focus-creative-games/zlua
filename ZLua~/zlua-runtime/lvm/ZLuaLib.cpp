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

#include "ZLuaLib.h"

#include "LuaEnv.h"

#include "../LuaConsts.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../marshal/ObjectRegistry.h"
#include "../marshal/StructMarshal.h"
#include "../mt/MetaBinding.h"
#include "../mt/TypeRegistry.h"
#if ZLUA_FAST_METATABLE
#include "../mt/FastMetatable.h"
#endif
#include "../marshal/DelegateMarshal.h"
#include "../marshal/ArrayMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/OpaqueValueMarshal.h"

#include "vm/Array.h"
#include "vm/Class.h"
#include "vm/MetadataCache.h"
#include "vm/Object.h"
#include "vm/Reflection.h"
#include "vm/Type.h"
#include "metadata/GenericMetadata.h"
#include "metadata/GenericMethod.h"

namespace zlua
{

static Il2CppClass* ResolveTypeArg(lua_State* L, int index)
{
    if (lua_type(L, index) == LUA_TSTRING)
    {
        const char* typeName = lua_tostring(L, index);
        // Same semantics as zlua.get_type_from_name / Type.GetType(name).
        return MetadataUtil::ResolveTypeFromName(typeName);
    }

    if (!lua_istable(L, index))
        return nullptr;

    lua_getfield(L, index, LuaConsts::Klass);
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return klass;
}

static int ZLuaBox(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* klass = ResolveTypeArg(L, 1);
    if (klass == nullptr)
        LuaException::Throw("zlua.box expects type as first argument");

    if (!MetadataUtil::IsValueTypeClass(klass))
        LuaException::ThrowFormat("zlua.box expects value type, got: %s.%s", klass->namespaze, klass->name);

    // alloca: Box copies the value; avoid malloc leak if Pop throws (ZLUA_TRY → longjmp-free C++ catch).
    void* storage = alloca(MetadataUtil::GetValueSize(&klass->byval_arg));
    TypedMarshal::PopByType(L, 2, storage, &klass->byval_arg);
    Il2CppObject* boxed = il2cpp::vm::Object::Box(klass, storage);
    ObjectMarshal::Push(L, boxed, klass);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaUnbox(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppObject* obj = ObjectRegistry::Pop(L, 1);
    if (obj == nullptr)
        LuaException::Throw("zlua.unbox value is not an object");

    Il2CppClass* klass = obj->klass;
    if (!MetadataUtil::IsValueTypeClass(klass))
        LuaException::ThrowFormat("zlua.unbox expects value type, got: %s.%s", klass->namespaze, klass->name);
    void* storage = ZLuaObjectUnbox(obj);
    TypedMarshal::PushByType(L, storage, &klass->byval_arg);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaCast(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppObject* obj = ObjectRegistry::Pop(L, 1);
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }

    Il2CppClass* targetType = ResolveTypeArg(L, 2);
    if (targetType == nullptr)
        LuaException::Throw("zlua.cast expects type as second argument");

    if (!il2cpp::vm::Class::IsAssignableFrom(targetType, obj->klass))
    {
        LuaException::ThrowFormat("zlua.cast failed: %s.%s is not assignable to %s.%s", obj->klass->namespaze, obj->klass->name, targetType->namespaze,
                                  targetType->name);
    }

    ObjectMarshal::Push(L, obj, targetType);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaTypeOf(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (!lua_istable(L, 1))
        LuaException::Throw("zlua.typeof expects a csharp type table");
    Il2CppClass* klass = TypeRegistry::GetClassFromTypeTable(L, 1);
    if (klass == nullptr)
        LuaException::Throw("zlua.typeof expects a csharp type table");
    // System.Type reflection object (same kind as C# typeof(T)).
    Il2CppReflectionType* reflectionType = il2cpp::vm::Reflection::GetTypeObject(&klass->byval_arg);
    ObjectMarshal::Push(L, (Il2CppObject*)reflectionType);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaGetTypeFromName(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 1 || lua_type(L, 1) != LUA_TSTRING)
        LuaException::Throw("zlua.get_type_from_name expects a type name string");

    const char* name = lua_tostring(L, 1);
    if (name == nullptr || name[0] == '\0')
        LuaException::Throw("zlua.get_type_from_name expects a type name string");

    // Distinguish invalid syntax from "not found" for clearer errors.
    std::string str(name);
    il2cpp::vm::TypeNameParseInfo info;
    il2cpp::vm::TypeNameParser parser(str, info, false);
    if (!parser.Parse())
        LuaException::ThrowFormat("zlua.get_type_from_name: invalid type name: %s", name);

    Il2CppClass* klass = MetadataUtil::ResolveTypeFromName(name);
    if (klass == nullptr)
        LuaException::ThrowFormat("zlua.get_type_from_name: type not found: %s", name);

    TypeRegistry::PushInternedTypeTable(L, klass);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaCreateSignature(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const int top = lua_gettop(L);
    std::string signature = "(";
    for (int i = 1; i <= top; ++i)
    {
        if (i > 1)
            signature.push_back(',');
        Il2CppClass* klass = ResolveTypeArg(L, i);
        if (klass == nullptr)
            LuaException::ThrowFormat("zlua.signature arg %d is not a type", i);
        signature += MetadataUtil::GetSignatureTypeName(klass);
    }
    signature.push_back(')');
    lua_pushstring(L, signature.c_str());
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaMakeGenericType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* genericDef = ResolveTypeArg(L, 1);
    if (genericDef == nullptr)
        LuaException::Throw("zlua.make_generic_type expects generic type as first arg");

    if (!ZLuaIsGenericTypeDefinition(genericDef))
        LuaException::ThrowFormat("type is not a generic definition: %s", MetadataUtil::GetTypeFullName(genericDef));

    const Il2CppMetadataGenericContainerHandle container = il2cpp::vm::Class::GetGenericContainer(genericDef);
    const uint32_t expectedCount = il2cpp::vm::MetadataCache::GetGenericContainerCount(container);
    const int argCount = lua_gettop(L) - 1;
    if ((uint32_t)argCount != expectedCount)
        LuaException::ThrowFormat("generic arg count mismatch: expected %u, got %d", expectedCount, argCount);

    const Il2CppType* typeArgs[32];
    IL2CPP_ASSERT(argCount <= 32);
    for (int i = 0; i < argCount; ++i)
    {
        Il2CppClass* argClass = ResolveTypeArg(L, i + 2);
        if (argClass == nullptr)
            LuaException::ThrowFormat("generic arg %d is not a type", i + 1);
        typeArgs[i] = &argClass->byval_arg;
    }

    Il2CppClass* closedType = il2cpp::vm::MetadataCache::GetGenericInstanceType(genericDef, typeArgs, (uint32_t)argCount);
    if (closedType == nullptr)
        LuaException::Throw("zlua make_generic_type error: failed to construct closed type");

    TypeRegistry::PushInternedTypeTable(L, closedType);
    return 1;
    ZLUA_TRY_END()
}

constexpr int maxMdArrayRank = 32;

static int ZLuaMakeSzArrayType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* elementClass = ResolveTypeArg(L, 1);
    if (elementClass == nullptr)
        LuaException::Throw("zlua.make_szarray_type expects element type");

    Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, 1);
    if (arrayClass == nullptr)
        LuaException::Throw("zlua.make_szarray_type failed");

    TypeRegistry::PushInternedTypeTable(L, arrayClass);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaMakeMdArrayType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* elementClass = ResolveTypeArg(L, 1);
    if (elementClass == nullptr)
        LuaException::Throw("zlua.make_mdarray_type expects element type");

    const int rank = (int)luaL_checkinteger(L, 2);
    if (rank < 1 || rank > maxMdArrayRank)
        LuaException::Throw("zlua.make_mdarray_type rank must be >= 1 && <= 32");

    Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, (uint32_t)rank);
    if (arrayClass == nullptr)
        LuaException::Throw("zlua.make_mdarray_type failed");

    TypeRegistry::PushInternedTypeTable(L, arrayClass);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaNewSzArrayByElementType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* elementClass = ResolveTypeArg(L, 1);
    if (elementClass == nullptr)
        LuaException::Throw("zlua.new_szarray_by_element_type expects element type");

    if (!lua_isinteger(L, 2))
        LuaException::Throw("zlua.new_szarray_by_element_type expects integer length");

    const lua_Integer length = lua_tointeger(L, 2);
    if (length < 0)
        LuaException::Throw("zlua.new_szarray_by_element_type length must be >= 0");

    Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, 1);
    Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)length);
    if (array == nullptr)
        LuaException::Throw("zlua: failed to create szarray");

    ObjectMarshal::Push(L, (Il2CppObject*)array);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaNewSzArrayBySzArrayType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* arrayClass = ResolveTypeArg(L, 1);
    if (arrayClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(arrayClass))
        LuaException::Throw("zlua.new_szarray_by_szarray_type expects szarray type table");
    if (arrayClass->rank != 1)
        LuaException::Throw("zlua.new_szarray_by_szarray_type expects rank-1 array type");

    if (!lua_isinteger(L, 2))
        LuaException::Throw("zlua.new_szarray_by_szarray_type expects integer length");

    const lua_Integer length = lua_tointeger(L, 2);
    if (length < 0)
        LuaException::Throw("zlua.new_szarray_by_szarray_type length must be >= 0");

    Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)length);
    if (array == nullptr)
        LuaException::Throw("zlua: failed to create szarray");

    ObjectMarshal::Push(L, (Il2CppObject*)array);
    return 1;
    ZLUA_TRY_END()
}

static Il2CppArray* CreateMdArrayInstance(lua_State* L, Il2CppClass* arrayClass, int lowboundsIndex, int sizesIndex)
{
    int rank = arrayClass->rank;
    int lowboundsLength = 0;
    int sizesLength = 0;
    if (!ArrayMarshal::TryGetConsecutiveTableLength(L, lowboundsIndex, lowboundsLength))
        LuaException::Throw("zlua.new_mdarray_* lowbounds is not a table");
    if (!ArrayMarshal::TryGetConsecutiveTableLength(L, sizesIndex, sizesLength))
        LuaException::Throw("zlua.new_mdarray_* sizes is not a table");
    if (lowboundsLength != rank)
        LuaException::Throw(std::string("zlua.new_mdarray_* lowbounds length must be ") + std::to_string(rank));
    if (sizesLength != rank)
        LuaException::Throw(std::string("zlua.new_mdarray_* sizes length must be ") + std::to_string(rank));

    std::vector<int32_t> lowerBounds;
    std::vector<int32_t> sizes;
    if (!ArrayMarshal::TryReadIntSequence(L, lowboundsIndex, rank, lowerBounds))
        LuaException::Throw("zlua.new_mdarray_* lowbounds is not a sequence of integers");
    if (!ArrayMarshal::TryReadIntSequence(L, sizesIndex, rank, sizes))
        LuaException::Throw("zlua.new_mdarray_* sizes is not a sequence of integers");

    for (int32_t size : sizes)
    {
        if (size < 0)
            LuaException::Throw("zlua.new_mdarray_* sizes must be >= 0");
    }

    std::vector<il2cpp_array_size_t> bounds((size_t)rank);
    std::vector<il2cpp_array_size_t> lengths((size_t)rank);
    for (int i = 0; i < rank; ++i)
    {
        bounds[(size_t)i] = (il2cpp_array_size_t)lowerBounds[(size_t)i];
        lengths[(size_t)i] = (il2cpp_array_size_t)sizes[(size_t)i];
    }

    return il2cpp::vm::Array::NewFull(arrayClass, lengths.data(), bounds.data());
}

static int ZLuaNewMdArrayByMdArrayType(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* arrayClass = ResolveTypeArg(L, 1);
    if (arrayClass == nullptr || arrayClass->byval_arg.type != IL2CPP_TYPE_ARRAY)
        LuaException::Throw("zlua.new_mdarray_by_mdarray_type expects mdarray type table");

    Il2CppArray* array = CreateMdArrayInstance(L, arrayClass, 2, 3);
    if (array == nullptr)
        LuaException::Throw("zlua.new_mdarray_by_mdarray_type failed");

    ObjectMarshal::Push(L, (Il2CppObject*)array);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaNewMdArrayBySpec(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    Il2CppClass* elementClass = ResolveTypeArg(L, 1);
    if (elementClass == nullptr)
        LuaException::Throw("zlua.new_mdarray_by_spec expects element type");

    int rank = 0;
    if (!ArrayMarshal::TryGetConsecutiveTableLength(L, 3, rank))
        LuaException::Throw("zlua.new_mdarray_by_spec sizes is not a table");
    if (rank < 1 || rank > maxMdArrayRank)
        LuaException::Throw("zlua.new_mdarray_by_spec rank must be >= 1 && <= 32");

    Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, (uint32_t)rank);
    Il2CppArray* array = CreateMdArrayInstance(L, arrayClass, 2, 3);
    if (array == nullptr)
        LuaException::Throw("zlua.new_mdarray_by_spec failed");

    ObjectMarshal::Push(L, (Il2CppObject*)array);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaMakeGenericMethod(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    const int kclosureStackIndex = 1;

    if (!lua_iscfunction(L, kclosureStackIndex))
        LuaException::Throw("zlua.register_method expects callable closure");

    if (!MetaBinding::IsDirectMethodClosure(L, kclosureStackIndex))
        LuaException::Throw("zlua.register_method expects direct method closure");

    const MethodMarshalCtx* ctx = MetaBinding::GetMethodMarshalCtxFromClosure(L, kclosureStackIndex);
    IL2CPP_ASSERT(ctx != nullptr);
    if (!ctx->method->is_generic)
        LuaException::Throw("zlua.make_generic_method expects generic method");

    const TypeBinding* binding = MetaBinding::GetTypeBindingFromClosure(L, kclosureStackIndex);
    IL2CPP_ASSERT(binding != nullptr);

    std::vector<const Il2CppType*> typeArgs;
    int32_t genericArgumentCount = MetadataUtil::GetMethodGenericParameterCount(ctx->method);
    IL2CPP_ASSERT(genericArgumentCount > 0);
    int32_t argCount = lua_gettop(L) - kclosureStackIndex;
    if (argCount != genericArgumentCount)
        LuaException::ThrowFormat("zlua.make_generic_method expects %d arguments, got %d", genericArgumentCount, argCount);

    for (int32_t i = 0; i < genericArgumentCount; ++i)
    {
        Il2CppClass* argClass = ResolveTypeArg(L, kclosureStackIndex + i + 1);
        if (argClass == nullptr)
            LuaException::ThrowFormat("zlua.make_generic_method arg %d is not a type", i + 1);
        typeArgs.push_back(&argClass->byval_arg);
    }
    const MethodInfo* method = ctx->method;
    const Il2CppGenericInst* methodInst = il2cpp::vm::MetadataCache::GetGenericInst(typeArgs.data(), (uint32_t)typeArgs.size());

    if (method->is_inflated)
    {
        method = il2cpp::metadata::GenericMethod::GetMethod(method->genericMethod->methodDefinition, method->genericMethod->context.class_inst, methodInst);
    }
    else
    {
        method = il2cpp::metadata::GenericMethod::GetMethod(method, NULL, methodInst);
    }
    std::string methodSignature = MetadataUtil::CreateMethodNameWithParametersAndGenericArguments(method);
    
    NameMetaMap& nmm = MetadataUtil::IsStaticMethod(ctx->method) ? binding->staticMap : (ctx->byVal ? binding->byvalInstanceMap : binding->byobjInstanceMap);
    auto it = nmm.find(methodSignature.c_str());
    if (it != nmm.end())
    {
        LuaUtil::PushRef(L, it->second.closureRef);
        return 1;
    }

    int closureRef = MetaBinding::CreateDirectMethodClosureRef(L, method, binding, MetadataUtil::IsStaticMethod(method), ctx->byVal);
    MetaInfo newMeta = {};
    newMeta.kind = MetaKind::Method;
    newMeta.closureRef = closureRef;
    const char* key = zlua_strdup(methodSignature.c_str());
    nmm.insert({key, newMeta});
    LuaUtil::PushRef(L, closureRef);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaRegisterMethod(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 2)
        LuaException::Throw("zlua.register_method expects (aliasName, methodOrClosure)");

    if (lua_type(L, 1) != LUA_TSTRING)
        LuaException::Throw("zlua.register_method argument mismatch: expects string alias name");

    const char* aliasName = lua_tostring(L, 1);
    if (aliasName[0] == '\0')
        LuaException::Throw("zlua.register_method expects non-empty alias name");

    const int kclosureStackIndex = 2;

    if (!lua_iscfunction(L, kclosureStackIndex))
        LuaException::Throw("zlua.register_method argument mismatch: expects callable closure");

    if (!MetaBinding::IsDirectMethodClosure(L, kclosureStackIndex))
        LuaException::Throw("zlua.register_method argument mismatch: expects direct method closure");

    const MethodMarshalCtx* ctx = MetaBinding::GetMethodMarshalCtxFromClosure(L, kclosureStackIndex);
    IL2CPP_ASSERT(ctx != nullptr);

    const TypeBinding* binding = MetaBinding::GetTypeBindingFromClosure(L, kclosureStackIndex);
    IL2CPP_ASSERT(binding != nullptr);

    const bool isStatic = MetadataUtil::IsStaticMethod(ctx->method);
    NameMetaMap& map = isStatic ? binding->staticMap : (ctx->byVal ? binding->byvalInstanceMap : binding->byobjInstanceMap);
    auto it = map.find(aliasName);
    if (it != map.end())
        LuaException::ThrowFormat("zlua: method alias already exists: %s", aliasName);

#if ZLUA_FAST_METATABLE
    FastMetatable::RawSetMethodOnIndexTable(L, binding->klass, isStatic, ctx->byVal, aliasName, kclosureStackIndex);
#endif

    MetaInfo newMeta = {};
    newMeta.kind = MetaKind::Method;
    newMeta.closureRef = LuaUtil::ToLuaRef(L);
    const char* key = zlua_strdup(aliasName);   
    map.insert({key, newMeta});
    return 0;
    ZLUA_TRY_END()
}

static int ZLuaToDelegate(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (!lua_isfunction(L, 1))
        LuaException::Throw("zlua.to_delegate expects Lua function");

    Il2CppClass* delegateClass = ResolveTypeArg(L, 2);
    if (delegateClass == nullptr)
        LuaException::Throw("zlua.to_delegate expects closed delegate type");

    if (!MetadataUtil::IsDelegateClass(delegateClass))
        LuaException::Throw("zlua.to_delegate expects delegate type");

    Il2CppDelegate* delegate = DelegateMarshal::Pop(L, 1, delegateClass);
    if (delegate == nullptr)
        LuaException::Throw("unsupported delegate signature for Lua callback");

    ObjectMarshal::Push(L, (Il2CppObject*)delegate);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaGetOpaqueValue(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 1)
        LuaException::Throw("zlua.get_opaquevalue expects (opaque_handle)");

    OpaqueValueMarshal::PushValueFromHandle(L, 1);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaSetOpaqueValue(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 2)
        LuaException::Throw("zlua.set_opaquevalue expects (opaque_handle, new_value)");

    OpaqueValueMarshal::SetValueToHandle(L, 1, 2);
    return 0;
    ZLUA_TRY_END()
}

static bool IsBlittablePrimitiveElementType(const Il2CppType* elementType)
{
    switch (elementType->type)
    {
        case IL2CPP_TYPE_BOOLEAN:
        case IL2CPP_TYPE_CHAR:
        case IL2CPP_TYPE_I1:
        case IL2CPP_TYPE_U1:
        case IL2CPP_TYPE_I2:
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_I4:
        case IL2CPP_TYPE_U4:
        case IL2CPP_TYPE_I8:
        case IL2CPP_TYPE_U8:
        case IL2CPP_TYPE_R4:
        case IL2CPP_TYPE_R8:
            return true;
        default:
            return false;
    }
}

static Il2CppArray* PopSzArray(lua_State* L, int index, const char* apiName)
{
    Il2CppObject* obj = ObjectRegistry::Pop(L, index);
    if (obj == nullptr)
        LuaException::ThrowFormat("%s expects szarray userdata", apiName);

    Il2CppClass* klass = obj->klass;
    if (klass == nullptr || klass->byval_arg.type != IL2CPP_TYPE_SZARRAY || klass->rank != 1)
        LuaException::ThrowFormat("%s expects szarray userdata", apiName);

    return (Il2CppArray*)obj;
}

static int ZLuaToBytes(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 1)
        LuaException::Throw("zlua argument mismatch: zlua.to_bytes expects (szarray)");

    if (!lua_isuserdata(L, 1))
        LuaException::Throw("zlua argument mismatch: zlua.to_bytes expects (szarray)");

    Il2CppObject* obj = ObjectRegistry::Pop(L, 1);
    if (obj == nullptr)
        return 0;

    Il2CppClass* arrayKlass = obj->klass;
    if (arrayKlass == nullptr || arrayKlass->byval_arg.type != IL2CPP_TYPE_SZARRAY || arrayKlass->rank != 1)
        LuaException::ThrowFormat("zlua argument mismatch: zlua.to_bytes expects szarray, got: %s.%s",
                                  arrayKlass ? arrayKlass->namespaze : "?", arrayKlass ? arrayKlass->name : "?");

    Il2CppClass* elementKlass = arrayKlass->element_class;
    if (elementKlass == nullptr)
        LuaException::Throw("zlua.to_bytes: missing element class");

    il2cpp::vm::Class::Init(elementKlass);
    if (!il2cpp::vm::Class::IsBlittable(elementKlass))
        LuaException::ThrowFormat("zlua argument mismatch: element type is not blittable: %s",
                                  MetadataUtil::GetTypeFullName(elementKlass));

    Il2CppArray* array = (Il2CppArray*)obj;
    const uint32_t byteLength = il2cpp::vm::Array::GetByteLength(array);
    if (byteLength == 0)
    {
        lua_pushlstring(L, "", 0);
        return 1;
    }

    const char* data = il2cpp::vm::Array::GetFirstElementAddress(array);
    lua_pushlstring(L, data, (size_t)byteLength);
    return 1;
    ZLUA_TRY_END()
}

static int ZLuaToTable(lua_State* L)
{
    ZLUA_TRY_BEGIN()
    if (lua_gettop(L) != 1)
        LuaException::Throw("zlua argument mismatch: zlua.to_table expects (szarray)");

    Il2CppObject* obj = ObjectRegistry::Pop(L, 1);
    if (obj == nullptr)
    {
        return 0;
    }
    if (obj->klass->byval_arg.type != IL2CPP_TYPE_SZARRAY)
    {
        LuaException::ThrowFormat("zlua argument mismatch: expected szarray, got: %s.%s", obj->klass->namespaze, obj->klass->name);
    }
    Il2CppArray* array = (Il2CppArray*)obj;
    ArrayMarshal::PushAsTable(L, array);
    return 1;
    ZLUA_TRY_END()
}

void ZLuaLib::RegisterGlobals(lua_State* L)
{
    lua_pushcfunction(L, ZLuaTypeOf);
    lua_setglobal(L, "__zlua_typeof");
    lua_pushcfunction(L, ZLuaGetTypeFromName);
    lua_setglobal(L, "__zlua_get_type_from_name");
    lua_pushcfunction(L, ZLuaCreateSignature);
    lua_setglobal(L, "__zlua_create_signature");
    lua_pushcfunction(L, ZLuaMakeGenericType);
    lua_setglobal(L, "__zlua_make_generic_type");
    lua_pushcfunction(L, ZLuaMakeSzArrayType);
    lua_setglobal(L, "__zlua_make_szarray_type");
    lua_pushcfunction(L, ZLuaMakeMdArrayType);
    lua_setglobal(L, "__zlua_make_mdarray_type");
    lua_pushcfunction(L, ZLuaNewSzArrayByElementType);
    lua_setglobal(L, "__zlua_new_szarray_by_element_type");
    lua_pushcfunction(L, ZLuaNewSzArrayBySzArrayType);
    lua_setglobal(L, "__zlua_new_szarray_by_szarray_type");
    lua_pushcfunction(L, ZLuaNewMdArrayByMdArrayType);
    lua_setglobal(L, "__zlua_new_mdarray_by_mdarray_type");
    lua_pushcfunction(L, ZLuaNewMdArrayBySpec);
    lua_setglobal(L, "__zlua_new_mdarray_by_spec");
    lua_pushcfunction(L, ZLuaMakeGenericMethod);
    lua_setglobal(L, "__zlua_make_generic_method");
    lua_pushcfunction(L, ZLuaRegisterMethod);
    lua_setglobal(L, "__zlua_register_method");
    lua_pushcfunction(L, ZLuaBox);
    lua_setglobal(L, "__zlua_box");
    lua_pushcfunction(L, ZLuaUnbox);
    lua_setglobal(L, "__zlua_unbox");
    lua_pushcfunction(L, ZLuaCast);
    lua_setglobal(L, "__zlua_cast");
    lua_pushcfunction(L, ZLuaToDelegate);
    lua_setglobal(L, "__zlua_to_delegate");
    lua_pushcfunction(L, ZLuaGetOpaqueValue);
    lua_setglobal(L, "__zlua_get_opaquevalue");
    lua_pushcfunction(L, ZLuaSetOpaqueValue);
    lua_setglobal(L, "__zlua_set_opaquevalue");
    lua_pushcfunction(L, ZLuaToBytes);
    lua_setglobal(L, "__zlua_to_bytes");
    lua_pushcfunction(L, ZLuaToTable);
    lua_setglobal(L, "__zlua_to_table");
}
} // namespace zlua
