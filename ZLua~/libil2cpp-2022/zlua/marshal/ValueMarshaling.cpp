#include "ValueMarshaling.h"
#include "MarshalAsRegistry.h"
#include "MarshalAsUserData.h"
#include "MarshalAsBytes.h"
#include "MarshalAsOpaque.h"
#include "../StructOpaqueScope.h"
#include "../MetadataUtil.h"
#include "../LuaUtil.h"
#include "../delegate/LuaDelegateBinder.h"
#include "../mt/MetaTableCache.h"
#include "../mt/MetaBinding.h"
#include "../mt/TypeRegistry.h"
#include "Marshaling.h"
#include "ObjectMarshal.h"
#include "PointerMarshal.h"
#include "PrimitiveMarshal.h"
#include "StringMarshal.h"
#include "StructMarshal.h"
#include "il2cpp-tabledefs.h"
#include "lua/lauxlib.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Method.h"
#include "vm/Type.h"
#include "vm/Array.h"
#include <climits>
#include <cstring>
#include <vector>

namespace zlua
{
bool ValueMarshaling::IsStructClass(Il2CppClass* klass)
{
    return klass->byval_arg.valuetype && !klass->enumtype;
}

bool ValueMarshaling::IsEnumClass(Il2CppClass* klass)
{
    return klass->enumtype;
}

size_t ValueMarshaling::GetValueTypeInstanceSize(Il2CppClass* klass)
{
    return il2cpp::vm::Class::GetValueSize(klass, nullptr);
}

static int ConversionKindRank(ConversionKind kind)
{
    switch (kind)
    {
    case ConversionKind::Identity:
        return 0;
    case ConversionKind::ImplicitNumeric:
        return 1;
    case ConversionKind::ImplicitEnum:
        return 2;
    case ConversionKind::NullLiteral:
        return 3;
    case ConversionKind::ImplicitReference:
        return 4;
    case ConversionKind::ImplicitBoxing:
        return 5;
    default:
        return 999;
    }
}

static bool IsConversionKindBetter(ConversionKind a, ConversionKind b)
{
    if (a == ConversionKind::None || b == ConversionKind::None)
        return false;
    return ConversionKindRank(a) < ConversionKindRank(b);
}

static bool IsBetterFunctionMember(
    const ConversionKind* candidateKinds,
    const ConversionKind* currentBestKinds,
    int parameterCount)
{
    bool candidateBetter = false;
    bool currentBetter = false;
    for (int i = 0; i < parameterCount; ++i)
    {
        if (IsConversionKindBetter(candidateKinds[i], currentBestKinds[i]))
            candidateBetter = true;
        if (IsConversionKindBetter(currentBestKinds[i], candidateKinds[i]))
            currentBetter = true;
    }
    return candidateBetter && !currentBetter;
}

static bool IsClassLikeType(const Il2CppType* type)
{
    return type->type == IL2CPP_TYPE_CLASS
        || type->type == IL2CPP_TYPE_VALUETYPE
        || type->type == IL2CPP_TYPE_GENERICINST;
}

static Il2CppClass* GetTypeClass(const Il2CppType* type)
{
    return il2cpp::vm::Class::FromIl2CppType(type, false);
}

static bool TryPopNullable(lua_State* L, int index, Il2CppClass* nullableClass, void* dest, size_t destSize)
{
    il2cpp::vm::Class::Init(nullableClass);
    const size_t sz = ValueMarshaling::GetValueTypeInstanceSize(nullableClass);
    if (destSize < sz)
        return false;

    if (lua_type(L, index) == LUA_TNIL)
    {
        std::memset(dest, 0, sz);
        return true;
    }

    Il2CppClass* underlying = il2cpp::vm::Class::GetNullableArgument(nullableClass);
    const int32_t valueOffset = nullableClass->fields[1].offset - (int32_t)sizeof(Il2CppObject);
    uint8_t* hasValuePtr = static_cast<uint8_t*>(dest) + nullableClass->fields[0].offset - sizeof(Il2CppObject);
    uint8_t* valuePtr = static_cast<uint8_t*>(dest) + valueOffset;
    const size_t underlyingSize = ValueMarshaling::GetValueTypeInstanceSize(underlying);

    if (!ValueMarshaling::TryPop(L, index, &underlying->byval_arg, valuePtr, underlyingSize))
        return false;

    *hasValuePtr = 1;
    return true;
}

static ConversionKind GetPrimitiveConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    if (type == nullptr || !PrimitiveMarshal::CanConvert(L, index, type))
        return ConversionKind::None;

    if (type->type == IL2CPP_TYPE_BOOLEAN)
        return ConversionKind::Identity;

    if (LuaUtil::IsStrictLuaInteger(L, index))
    {
        switch (type->type)
        {
        case IL2CPP_TYPE_I1:
        case IL2CPP_TYPE_U1:
        case IL2CPP_TYPE_I2:
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
        case IL2CPP_TYPE_I4:
        case IL2CPP_TYPE_U4:
        case IL2CPP_TYPE_I:
        case IL2CPP_TYPE_U:
            return ConversionKind::Identity;
        case IL2CPP_TYPE_I8:
        case IL2CPP_TYPE_U8:
        case IL2CPP_TYPE_R4:
        case IL2CPP_TYPE_R8:
            return ConversionKind::ImplicitNumeric;
        default:
            break;
        }
    }
    else if (lua_isnumber(L, index))
    {
        switch (type->type)
        {
        case IL2CPP_TYPE_R8:
            return ConversionKind::Identity;
        case IL2CPP_TYPE_R4:
        case IL2CPP_TYPE_I:
        case IL2CPP_TYPE_U:
            return ConversionKind::ImplicitNumeric;
        default:
            return ConversionKind::None;
        }
    }

    return ConversionKind::None;
}

static ConversionKind GetReferenceConversionKind(Il2CppClass* paramClass, Il2CppClass* runtimeClass)
{
    if (paramClass == nullptr || runtimeClass == nullptr)
        return ConversionKind::None;
    il2cpp::vm::Class::Init(paramClass);
    il2cpp::vm::Class::Init(runtimeClass);
    if (!il2cpp::vm::Class::IsAssignableFrom(paramClass, runtimeClass))
        return ConversionKind::None;
    return runtimeClass == paramClass ? ConversionKind::Identity : ConversionKind::ImplicitReference;
}

static ConversionKind GetArrayConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    if (type == nullptr || (type->type != IL2CPP_TYPE_SZARRAY && type->type != IL2CPP_TYPE_ARRAY))
        return ConversionKind::None;

    if (lua_type(L, index) == LUA_TNIL)
        return ConversionKind::NullLiteral;

    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr)
        return ConversionKind::None;

    Il2CppClass* expectedClass = il2cpp::vm::Class::FromIl2CppType(type);
    if (expectedClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(expectedClass))
        return ConversionKind::None;

    return GetReferenceConversionKind(expectedClass, obj->klass);
}

static ConversionKind GetEnumConversionKind(lua_State* L, int index, Il2CppClass* enumClass)
{
    if (enumClass == nullptr || !enumClass->enumtype)
        return ConversionKind::None;

    const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(enumClass);
    if (baseType == nullptr)
        return ConversionKind::None;

    if (PrimitiveMarshal::CanConvert(L, index, baseType))
        return ConversionKind::ImplicitEnum;

    if (StructMarshal::IsByValUserData(L, index))
    {
        ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
        if (header == nullptr || header->klass == nullptr)
            return ConversionKind::None;
        il2cpp::vm::Class::Init(enumClass);
        il2cpp::vm::Class::Init(header->klass);
        if (!il2cpp::vm::Class::IsAssignableFrom(enumClass, header->klass))
            return ConversionKind::None;
        return header->klass == enumClass ? ConversionKind::Identity : ConversionKind::None;
    }

    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr)
        return ConversionKind::None;
    return GetReferenceConversionKind(enumClass, obj->klass);
}

static ConversionKind GetObjectParameterConversionKind(lua_State* L, int index)
{
    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
        return ConversionKind::NullLiteral;
    if (luaType == LUA_TBOOLEAN || luaType == LUA_TNUMBER)
        return ConversionKind::ImplicitBoxing;
    if (luaType == LUA_TSTRING)
        return ConversionKind::ImplicitReference;
    if (luaType == LUA_TUSERDATA)
    {
        if (StructMarshal::IsByValUserData(L, index))
            return ConversionKind::ImplicitBoxing;
        return ObjectMarshal::Pop(L, index) != nullptr ? ConversionKind::Identity : ConversionKind::None;
    }
    return ConversionKind::None;
}

static ConversionKind GetDelegateConversionKind(lua_State* L, int index, Il2CppClass* delegateClass)
{
    if (delegateClass == nullptr || !MetadataUtil::IsDelegateClass(delegateClass))
        return ConversionKind::None;

    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
        return ConversionKind::NullLiteral;
    if (luaType == LUA_TFUNCTION)
        return ConversionKind::Identity;

    if (luaType == LUA_TUSERDATA)
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return ConversionKind::None;
        return GetReferenceConversionKind(delegateClass, obj->klass);
    }

    return ConversionKind::None;
}

static ConversionKind GetClassLikeConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    Il2CppClass* paramClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (paramClass == nullptr)
        return ConversionKind::None;

    if (MetadataUtil::IsDelegateClass(paramClass))
        return GetDelegateConversionKind(L, index, paramClass);

    if (paramClass->enumtype)
        return GetEnumConversionKind(L, index, paramClass);

    if (paramClass->byval_arg.valuetype)
    {
        if (PointerMarshal::IsIntPtrClass(paramClass) || PointerMarshal::IsUIntPtrClass(paramClass))
            return PointerMarshal::CanConvert(L, index, type) ? ConversionKind::Identity : ConversionKind::None;

        if (StructMarshal::IsByValUserData(L, index))
        {
            ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
            if (header == nullptr || header->klass == nullptr)
                return ConversionKind::None;
            il2cpp::vm::Class::Init(paramClass);
            il2cpp::vm::Class::Init(header->klass);
            if (!il2cpp::vm::Class::IsAssignableFrom(paramClass, header->klass))
                return ConversionKind::None;
            return header->klass == paramClass ? ConversionKind::Identity : ConversionKind::None;
        }

        if (lua_istable(L, index) && StructMarshal::CanPopFromTable(L, index, paramClass))
            return ConversionKind::Identity;

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return ConversionKind::None;
        il2cpp::vm::Class::Init(paramClass);
        if (!il2cpp::vm::Class::IsAssignableFrom(paramClass, obj->klass))
            return ConversionKind::None;
        return obj->klass == paramClass ? ConversionKind::Identity : ConversionKind::ImplicitReference;
    }

    if (lua_type(L, index) == LUA_TNIL)
        return ConversionKind::NullLiteral;

    if (StructMarshal::IsByValUserData(L, index))
    {
        ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
        if (header == nullptr || header->klass == nullptr)
            return ConversionKind::None;
        il2cpp::vm::Class::Init(paramClass);
        il2cpp::vm::Class::Init(header->klass);
        if (!il2cpp::vm::Class::IsAssignableFrom(paramClass, header->klass))
            return ConversionKind::None;
        return ConversionKind::ImplicitBoxing;
    }

    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr)
        return ConversionKind::None;
    return GetReferenceConversionKind(paramClass, obj->klass);
}

ConversionKind ValueMarshaling::GetConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    return GetConversionKind(L, index, type, nullptr, -1);
}

ConversionKind ValueMarshaling::GetConversionKind(lua_State* L, int index, const Il2CppType* type, const MethodInfo* method, int paramIndex)
{
    if (method != nullptr && paramIndex >= 0)
    {
        const LuaMarshalType marshalType = MarshalAsRegistry::GetParameterMarshalType(method, paramIndex);
        if (marshalType == LuaMarshalType::UserData)
            return MarshalAsUserData::GetConversionKind(L, index, type);
        if (marshalType == LuaMarshalType::Bytes)
            return MarshalAsBytes::GetConversionKind(L, index, type);
    }

    if (type == nullptr)
        return ConversionKind::None;

    if (PointerMarshal::IsPointerType(type))
    {
        const int luaType = lua_type(L, index);
        if (luaType == LUA_TNIL)
            return ConversionKind::NullLiteral;
        return luaType == LUA_TLIGHTUSERDATA ? ConversionKind::Identity : ConversionKind::None;
    }

    Il2CppClass* typeClass = GetTypeClass(type);
    if (typeClass != nullptr)
    {
        il2cpp::vm::Class::Init(typeClass);
        if (PointerMarshal::IsUnsupportedMarshalType(typeClass))
            return ConversionKind::None;
    }

    if (lua_type(L, index) == LUA_TNIL)
    {
        if (typeClass != nullptr && il2cpp::vm::Class::IsNullable(typeClass))
            return ConversionKind::NullLiteral;
        return il2cpp::vm::Type::IsReference(type) ? ConversionKind::NullLiteral : ConversionKind::None;
    }

    if (typeClass != nullptr && il2cpp::vm::Class::IsNullable(typeClass))
    {
        Il2CppClass* underlying = il2cpp::vm::Class::GetNullableArgument(typeClass);
        return GetConversionKind(L, index, &underlying->byval_arg);
    }

    if (type->type == IL2CPP_TYPE_I || type->type == IL2CPP_TYPE_U
        || (typeClass != nullptr && (PointerMarshal::IsIntPtrClass(typeClass) || PointerMarshal::IsUIntPtrClass(typeClass))))
    {
        return PointerMarshal::CanConvert(L, index, type) ? ConversionKind::Identity : ConversionKind::None;
    }

    ConversionKind primitiveKind = GetPrimitiveConversionKind(L, index, type);
    if (primitiveKind != ConversionKind::None)
        return primitiveKind;

    if (type->type == IL2CPP_TYPE_STRING)
        return StringMarshal::CanConvert(L, index) ? ConversionKind::Identity : ConversionKind::None;

    if (type->type == IL2CPP_TYPE_OBJECT)
        return GetObjectParameterConversionKind(L, index);

    if (IsClassLikeType(type))
        return GetClassLikeConversionKind(L, index, type);

    ConversionKind arrayKind = GetArrayConversionKind(L, index, type);
    if (arrayKind != ConversionKind::None)
        return arrayKind;

    return ConversionKind::None;
}

static Il2CppObject* BoxLuaValueToObject(lua_State* L, int index)
{
    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
        return nullptr;

    if (luaType == LUA_TBOOLEAN)
    {
        Il2CppClass* klass = MetadataUtil::ResolveCorlibType("System.Boolean");
        if (klass == nullptr)
            return nullptr;
        il2cpp::vm::Class::Init(klass);
        uint8_t value = PrimitiveMarshal::PopBool(L, index) ? 1 : 0;
        return il2cpp::vm::Object::Box(klass, &value);
    }

    if (luaType == LUA_TNUMBER)
    {
        if (LuaUtil::IsStrictLuaInteger(L, index))
        {
            const lua_Integer integral = lua_tointeger(L, index);
            if (integral >= INT32_MIN && integral <= INT32_MAX)
            {
                Il2CppClass* klass = MetadataUtil::ResolveCorlibType("System.Int32");
                if (klass == nullptr)
                    return nullptr;
                il2cpp::vm::Class::Init(klass);
                int32_t value = (int32_t)integral;
                return il2cpp::vm::Object::Box(klass, &value);
            }

            Il2CppClass* klass = MetadataUtil::ResolveCorlibType("System.Int64");
            if (klass == nullptr)
                return nullptr;
            il2cpp::vm::Class::Init(klass);
            int64_t value = (int64_t)integral;
            return il2cpp::vm::Object::Box(klass, &value);
        }

        Il2CppClass* klass = MetadataUtil::ResolveCorlibType("System.Double");
        if (klass == nullptr)
            return nullptr;
        il2cpp::vm::Class::Init(klass);
        double value = PrimitiveMarshal::PopDouble(L, index);
        return il2cpp::vm::Object::Box(klass, &value);
    }

    if (luaType == LUA_TSTRING)
    {
        Il2CppString* str = StringMarshal::Pop(L, index);
        return reinterpret_cast<Il2CppObject*>(str);
    }

    if (StructMarshal::IsByValUserData(L, index))
    {
        ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
        if (header == nullptr || header->klass == nullptr)
            return nullptr;
        il2cpp::vm::Class::Init(header->klass);
        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(header->klass);
        std::vector<uint8_t> storage(payloadSize, 0);
        std::memcpy(storage.data(), header->Payload(), payloadSize);
        return il2cpp::vm::Object::Box(header->klass, static_cast<void*>(storage.data()));
    }

    return ObjectMarshal::Pop(L, index);
}

static bool TryPopEnum(lua_State* L, int index, Il2CppClass* enumClass, void* dest, size_t destSize)
{
    if (GetEnumConversionKind(L, index, enumClass) == ConversionKind::None)
        return false;
    const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(enumClass);
    if (baseType == nullptr)
        return false;
    if (PrimitiveMarshal::CanConvert(L, index, baseType))
        return PrimitiveMarshal::TryPop(L, index, baseType, dest, destSize);
    if (StructMarshal::PopValue(L, index, enumClass, dest))
        return true;
    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr)
        return false;
    const size_t sz = ValueMarshaling::GetValueTypeInstanceSize(enumClass);
    if (sz > destSize)
        return false;
    std::memcpy(dest, il2cpp::vm::Object::Unbox(obj), sz);
    return true;
}

static bool TryPopObject(lua_State* L, int index, Il2CppObject** dest)
{
    if (dest == nullptr)
        return false;

    const ConversionKind kind = GetObjectParameterConversionKind(L, index);
    switch (kind)
    {
    case ConversionKind::NullLiteral:
        *dest = nullptr;
        return true;
    case ConversionKind::ImplicitBoxing:
        *dest = BoxLuaValueToObject(L, index);
        return true;
    case ConversionKind::ImplicitReference:
        if (lua_type(L, index) == LUA_TSTRING)
        {
            Il2CppString* str = StringMarshal::Pop(L, index);
            *dest = reinterpret_cast<Il2CppObject*>(str);
            return true;
        }
        *dest = ObjectMarshal::Pop(L, index);
        return *dest != nullptr;
    case ConversionKind::Identity:
        *dest = ObjectMarshal::Pop(L, index);
        if (*dest != nullptr)
            return true;
        if (lua_type(L, index) == LUA_TSTRING)
        {
            Il2CppString* str = StringMarshal::Pop(L, index);
            *dest = reinterpret_cast<Il2CppObject*>(str);
            return true;
        }
        return false;
    default:
        return false;
    }
}

static bool TryPopDelegate(lua_State* L, int index, Il2CppClass* delegateClass, Il2CppObject** dest)
{
    if (dest == nullptr || delegateClass == nullptr)
        return false;

    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
    {
        *dest = nullptr;
        return true;
    }

    if (luaType == LUA_TFUNCTION)
    {
        Il2CppDelegate* delegate = LuaDelegateBinder::CreateFromStack(L, index, delegateClass);
        *dest = reinterpret_cast<Il2CppObject*>(delegate);
        return delegate != nullptr;
    }

    if (luaType == LUA_TUSERDATA)
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return false;
        il2cpp::vm::Class::Init(delegateClass);
        if (!il2cpp::vm::Class::IsAssignableFrom(delegateClass, obj->klass))
            return false;
        *dest = obj;
        return true;
    }

    return false;
}

static bool TryPopClassLike(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
{
    Il2CppClass* paramClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (paramClass == nullptr)
        return false;
    if (MetadataUtil::IsDelegateClass(paramClass))
        return TryPopDelegate(L, index, paramClass, reinterpret_cast<Il2CppObject**>(dest));
    if (paramClass->enumtype)
        return TryPopEnum(L, index, paramClass, dest, destSize);
    if (paramClass->byval_arg.valuetype)
    {
        if (PointerMarshal::IsIntPtrClass(paramClass) || PointerMarshal::IsUIntPtrClass(paramClass))
            return PointerMarshal::TryPop(L, index, type, dest, destSize);
        if (StructMarshal::PopValue(L, index, paramClass, dest))
            return true;
        if (StructMarshal::TryPopFromTable(L, index, paramClass, dest))
            return true;
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return false;
        const size_t sz = ValueMarshaling::GetValueTypeInstanceSize(paramClass);
        if (sz > destSize)
            return false;
        std::memcpy(dest, il2cpp::vm::Object::Unbox(obj), sz);
        return true;
    }

    const ConversionKind kind = ValueMarshaling::GetConversionKind(L, index, type);
    if (kind == ConversionKind::NullLiteral)
    {
        *reinterpret_cast<Il2CppObject**>(dest) = nullptr;
        return true;
    }
    if (kind == ConversionKind::ImplicitBoxing)
    {
        *reinterpret_cast<Il2CppObject**>(dest) = BoxLuaValueToObject(L, index);
        return true;
    }
    if (kind == ConversionKind::Identity || kind == ConversionKind::ImplicitReference)
    {
        *reinterpret_cast<Il2CppObject**>(dest) = ObjectMarshal::Pop(L, index);
        return true;
    }
    return false;
}

bool ValueMarshaling::CanConvert(lua_State* L, int index, const Il2CppType* type)
{
    return GetConversionKind(L, index, type) != ConversionKind::None;
}

bool ValueMarshaling::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
{
    return TryPop(L, index, type, dest, destSize, nullptr, -1);
}

bool ValueMarshaling::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize, const MethodInfo* method, int paramIndex)
{
    if (method != nullptr && paramIndex >= 0)
    {
        const LuaMarshalType marshalType = MarshalAsRegistry::GetParameterMarshalType(method, paramIndex);
        if (marshalType == LuaMarshalType::UserData)
            return MarshalAsUserData::TryPop(L, index, type, dest, destSize);
        if (marshalType == LuaMarshalType::Bytes)
            return MarshalAsBytes::TryPop(L, index, type, dest, destSize);
    }

    if (!CanConvert(L, index, type))
        return false;

    Il2CppClass* typeClass = GetTypeClass(type);
    if (typeClass != nullptr && il2cpp::vm::Class::IsNullable(typeClass))
        return TryPopNullable(L, index, typeClass, dest, destSize);

    if (PointerMarshal::IsPointerType(type)
        || (typeClass != nullptr && (PointerMarshal::IsIntPtrClass(typeClass) || PointerMarshal::IsUIntPtrClass(typeClass))))
    {
        return PointerMarshal::TryPop(L, index, type, dest, destSize);
    }

    if (PrimitiveMarshal::TryPop(L, index, type, dest, destSize))
        return true;
    if (type->type == IL2CPP_TYPE_STRING)
        return StringMarshal::TryPop(L, index, reinterpret_cast<Il2CppString**>(dest));
    if (type->type == IL2CPP_TYPE_OBJECT)
        return TryPopObject(L, index, reinterpret_cast<Il2CppObject**>(dest));
    if (IsClassLikeType(type))
        return TryPopClassLike(L, index, type, dest, destSize);
    if (type->type == IL2CPP_TYPE_SZARRAY || type->type == IL2CPP_TYPE_ARRAY)
    {
        *reinterpret_cast<Il2CppObject**>(dest) = ObjectMarshal::Pop(L, index);
        return true;
    }
    return false;
}

bool ValueMarshaling::AttachInstanceMetatable(lua_State* L, Il2CppClass* klass)
{
    if (!MetaTableCache::TryPushReferenceMetatable(L, klass))
    {
        TypeRegistry::PushInternedTypeTable(L, klass);
        lua_pop(L, 1);
        if (!MetaTableCache::TryPushReferenceMetatable(L, klass))
            return false;
    }
    lua_setmetatable(L, -2);
    return true;
}

bool ValueMarshaling::AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass)
{
    if (!MetaTableCache::TryPushByObjMetatable(L, klass))
        return false;
    lua_setmetatable(L, -2);
    return true;
}

void ValueMarshaling::PushStructInstance(lua_State* L, Il2CppClass* klass, Il2CppObject* boxed)
{
    if (klass == nullptr || boxed == nullptr)
        luaL_error(L, "zlua: invalid struct instance");
    void* payload = il2cpp::vm::Object::Unbox(boxed);
    StructMarshal::PushValue(L, payload, klass);
}

int ValueMarshaling::PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj)
{
    return PushReturn(L, returnType, retObj, nullptr);
}

int ValueMarshaling::PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj, const MethodInfo* method)
{
    if (method != nullptr)
    {
        const LuaMarshalType marshalType = MarshalAsRegistry::GetReturnMarshalType(method);
        if (marshalType == LuaMarshalType::UserData)
            return MarshalAsUserData::Push(L, returnType, retObj);
        if (marshalType == LuaMarshalType::Bytes)
            return MarshalAsBytes::Push(L, returnType, retObj);
        if (marshalType == LuaMarshalType::OpaqueLightUserData)
            return MarshalAsOpaque::Push(L, returnType, retObj);
    }

    if (returnType == nullptr || returnType->type == IL2CPP_TYPE_VOID)
        return 0;

    if (PointerMarshal::IsPointerType(returnType))
        return PointerMarshal::PushReturn(L, returnType, retObj);

    if (returnType->type == IL2CPP_TYPE_STRING)
    {
        Marshaling::PushCString(L, (Il2CppString*)retObj);
        return 1;
    }

    Il2CppClass* returnClass = il2cpp::vm::Class::FromIl2CppType(returnType, false);
    if (returnClass != nullptr)
    {
        il2cpp::vm::Class::Init(returnClass);
        if (PointerMarshal::IsIntPtrClass(returnClass) || PointerMarshal::IsUIntPtrClass(returnClass))
            return PointerMarshal::PushReturn(L, returnType, retObj);
    }

    if (returnType->type == IL2CPP_TYPE_VALUETYPE || returnType->type == IL2CPP_TYPE_GENERICINST)
    {
        if (returnClass != nullptr && il2cpp::vm::Class::IsNullable(returnClass))
        {
            if (retObj == nullptr)
            {
                lua_pushnil(L);
                return 1;
            }

            void* data = il2cpp::vm::Object::Unbox(retObj);
            if (!il2cpp::vm::Object::NullableHasValue(returnClass, data))
            {
                lua_pushnil(L);
                return 1;
            }

            Il2CppClass* underlying = il2cpp::vm::Class::GetNullableArgument(returnClass);
            void* valuePtr = data;

            if (underlying->enumtype)
            {
                const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(underlying);
                if (baseType == nullptr)
                    return luaL_error(L, "zlua: enum base type missing");
                const int pushed = PrimitiveMarshal::PushBoxedReturn(L, baseType, valuePtr);
                if (pushed == 0)
                    return luaL_error(L, "zlua: unsupported nullable return type");
                return pushed;
            }

            const int pushed = PrimitiveMarshal::PushBoxedReturn(
                L,
                &underlying->byval_arg,
                valuePtr);
            if (pushed == 0)
                return luaL_error(L, "zlua: unsupported nullable return type");
            return pushed;
        }
    }

    if (returnType->type == IL2CPP_TYPE_CLASS || returnType->type == IL2CPP_TYPE_OBJECT
        || returnType->type == IL2CPP_TYPE_SZARRAY || returnType->type == IL2CPP_TYPE_ARRAY)
    {
        ObjectMarshal::Push(L, retObj);
        if (retObj != nullptr)
            AttachInstanceMetatable(L, retObj->klass);
        return 1;
    }
    if (returnType->type == IL2CPP_TYPE_VALUETYPE || returnType->type == IL2CPP_TYPE_GENERICINST)
    {
        if (retObj == nullptr)
            return luaL_error(L, "zlua: null value type return");
        Il2CppClass* retClass = retObj->klass;
        if (retClass != nullptr && retClass->enumtype)
        {
            const Il2CppType* baseType = il2cpp::vm::Class::GetEnumBaseType(retClass);
            if (baseType == nullptr)
                return luaL_error(L, "zlua: enum base type missing");
            const int pushed = PrimitiveMarshal::PushBoxedReturn(L, baseType, il2cpp::vm::Object::Unbox(retObj));
            if (pushed == 0)
                return luaL_error(L, "zlua: unsupported enum return type");
            return pushed;
        }
        if (retClass != nullptr && retClass->byval_arg.valuetype)
        {
            PushStructInstance(L, retClass, retObj);
            return 1;
        }
        ObjectMarshal::Push(L, retObj);
        if (retObj != nullptr)
            AttachInstanceMetatable(L, retObj->klass);
        return 1;
    }
    if (retObj == nullptr)
        return luaL_error(L, "zlua: null primitive return");
    const int pushed = PrimitiveMarshal::PushBoxedReturn(L, returnType, il2cpp::vm::Object::Unbox(retObj));
    if (pushed == 0)
        return luaL_error(L, "zlua: unsupported return type");
    return pushed;
}

static bool TryGetParameterConversionKinds(
    lua_State* L,
    int argStart,
    int argCount,
    const MethodInfo* method,
    std::vector<ConversionKind>& kindsOut)
{
    kindsOut.clear();
    if (method == nullptr)
        return false;

    const int paramsIndex = ValueMarshaling::FindParamsParameterIndex(method);
    if (paramsIndex >= 0)
    {
        const int fixedCount = paramsIndex;
        if (argCount < fixedCount)
            return false;

        kindsOut.reserve((size_t)argCount);
        for (int i = 0; i < fixedCount; ++i)
        {
            ConversionKind kind = ValueMarshaling::GetConversionKind(L, argStart + i, method->parameters[i], method, i);
            if (kind == ConversionKind::None)
                return false;
            kindsOut.push_back(kind);
        }

        const Il2CppType* paramsType = method->parameters[paramsIndex];
        const int varCount = argCount - fixedCount;
        if (varCount == 1 && (paramsType->type == IL2CPP_TYPE_SZARRAY || paramsType->type == IL2CPP_TYPE_ARRAY))
        {
            ConversionKind kind = ValueMarshaling::GetConversionKind(L, argStart + fixedCount, paramsType);
            if (kind == ConversionKind::None)
                return false;
            kindsOut.push_back(kind);
            return true;
        }

        Il2CppClass* arrayClass = il2cpp::vm::Class::FromIl2CppType(paramsType);
        if (arrayClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(arrayClass))
            return false;
        Il2CppClass* elementClass = arrayClass->element_class;
        if (elementClass == nullptr)
            return false;

        for (int i = 0; i < varCount; ++i)
        {
            ConversionKind kind = ValueMarshaling::GetConversionKind(L, argStart + fixedCount + i, &elementClass->byval_arg, method, paramsIndex);
            if (kind == ConversionKind::None)
                return false;
            kindsOut.push_back(kind);
        }
        return true;
    }

    if (method->parameters_count != argCount)
        return false;

    kindsOut.reserve((size_t)argCount);
    for (int i = 0; i < argCount; ++i)
    {
        ConversionKind kind = ValueMarshaling::GetConversionKind(L, argStart + i, method->parameters[i], method, i);
        if (kind == ConversionKind::None)
            return false;
        kindsOut.push_back(kind);
    }
    return true;
}

bool ValueMarshaling::TryMatchParameters(lua_State* L, int argStart, int argCount, const MethodInfo* method)
{
    std::vector<ConversionKind> kinds;
    return TryGetParameterConversionKinds(L, argStart, argCount, method, kinds);
}

int ValueMarshaling::FindParamsParameterIndex(const MethodInfo* method)
{
    if (method == nullptr)
        return -1;
    for (int i = 0; i < method->parameters_count; ++i)
    {
        if (MetadataUtil::MethodParameterHasParamArrayAttribute(method, i))
            return i;
    }
    return -1;
}

static const MethodInfo* SelectBestMatchingMethod(
    Il2CppClass* klass,
    const char* name,
    bool isStatic,
    lua_State* L,
    int argStart,
    int argCount)
{
    const MethodInfo* bestMethod = nullptr;
    std::vector<ConversionKind> bestKinds;

    il2cpp::vm::Class::Init(klass);
    for (Il2CppClass* cursor = klass; cursor != nullptr; cursor = cursor->parent)
    {
        for (uint16_t i = 0; i < cursor->method_count; ++i)
        {
            const MethodInfo* method = cursor->methods[i];
            if (strcmp(method->name, name) != 0)
                continue;
            const bool methodIsStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
            if (methodIsStatic != isStatic)
                continue;

            std::vector<ConversionKind> candidateKinds;
            if (!TryGetParameterConversionKinds(L, argStart, argCount, method, candidateKinds))
                continue;

            if (bestMethod == nullptr)
            {
                bestMethod = method;
                bestKinds = std::move(candidateKinds);
                continue;
            }

            if (IsBetterFunctionMember(candidateKinds.data(), bestKinds.data(), (int)candidateKinds.size()))
            {
                bestMethod = method;
                bestKinds = std::move(candidateKinds);
            }
        }
    }

    return bestMethod;
}

const MethodInfo* ValueMarshaling::FindMatchingMethod(
    Il2CppClass* klass, const char* name, bool isStatic, lua_State* L, int argStart, int argCount)
{
    return SelectBestMatchingMethod(klass, name, isStatic, L, argStart, argCount);
}

const MethodInfo* ValueMarshaling::FindMatchingConstructor(Il2CppClass* klass, lua_State* L, int argStart, int argCount)
{
    const MethodInfo* bestMethod = nullptr;
    std::vector<ConversionKind> bestKinds;

    il2cpp::vm::Class::Init(klass);
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const MethodInfo* method = klass->methods[i];
        if (strcmp(method->name, ".ctor") != 0 || method->parameters_count != argCount)
            continue;

        std::vector<ConversionKind> candidateKinds;
        if (!TryGetParameterConversionKinds(L, argStart, argCount, method, candidateKinds))
            continue;

        if (bestMethod == nullptr)
        {
            bestMethod = method;
            bestKinds = std::move(candidateKinds);
            continue;
        }

        if (IsBetterFunctionMember(candidateKinds.data(), bestKinds.data(), (int)candidateKinds.size()))
        {
            bestMethod = method;
            bestKinds = std::move(candidateKinds);
        }
    }

    return bestMethod;
}

static bool IsPrimitiveIl2CppType(const Il2CppType* type)
{
    if (type == nullptr)
        return false;

    switch (type->type)
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
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U:
        return true;
    default:
        return false;
    }
}

static Il2CppClass* ResolvePrimitiveBoxClass(const Il2CppType* type)
{
    if (type == nullptr)
        return nullptr;

    const char* typeName = nullptr;
    switch (type->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        typeName = "System.Boolean";
        break;
    case IL2CPP_TYPE_CHAR:
        typeName = "System.Char";
        break;
    case IL2CPP_TYPE_I1:
        typeName = "System.SByte";
        break;
    case IL2CPP_TYPE_U1:
        typeName = "System.Byte";
        break;
    case IL2CPP_TYPE_I2:
        typeName = "System.Int16";
        break;
    case IL2CPP_TYPE_U2:
        typeName = "System.UInt16";
        break;
    case IL2CPP_TYPE_I4:
        typeName = "System.Int32";
        break;
    case IL2CPP_TYPE_U4:
        typeName = "System.UInt32";
        break;
    case IL2CPP_TYPE_I8:
        typeName = "System.Int64";
        break;
    case IL2CPP_TYPE_U8:
        typeName = "System.UInt64";
        break;
    case IL2CPP_TYPE_R4:
        typeName = "System.Single";
        break;
    case IL2CPP_TYPE_R8:
        typeName = "System.Double";
        break;
    case IL2CPP_TYPE_I:
        typeName = "System.IntPtr";
        break;
    case IL2CPP_TYPE_U:
        typeName = "System.UIntPtr";
        break;
    default:
        return nullptr;
    }

    return MetadataUtil::ResolveCorlibType(typeName);
}

static int PushBoxedByObjInstance(lua_State* L, Il2CppClass* klass, Il2CppObject* boxed)
{
    ObjectMarshal::Push(L, boxed);
    if (boxed == nullptr)
        return 1;

    if (ValueMarshaling::IsStructClass(klass) || ValueMarshaling::IsEnumClass(klass))
    {
        if (ValueMarshaling::AttachByObjInstanceMetatable(L, klass))
            return 1;
    }
    else if (ValueMarshaling::AttachInstanceMetatable(L, klass))
    {
        return 1;
    }

    TypeRegistry::PushInternedTypeTable(L, klass);
    const int typeTableIndex = lua_gettop(L);
    const int boxedIndex = typeTableIndex - 1;

    if (ValueMarshaling::IsStructClass(klass) || ValueMarshaling::IsEnumClass(klass))
        MetaBinding::PushByObjInstanceMetatable(L, klass, typeTableIndex);
    else
        MetaBinding::PushReferenceInstanceMetatable(L, klass, typeTableIndex);

    lua_setmetatable(L, boxedIndex);
    lua_settop(L, boxedIndex);
    return 1;
}

static bool TryPopBoxedObjectValue(
    lua_State* L,
    int index,
    Il2CppClass* expectedClass,
    void* dest,
    size_t destSize)
{
    if (expectedClass == nullptr || dest == nullptr)
        return false;

    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    if (obj == nullptr)
        return false;

    il2cpp::vm::Class::Init(expectedClass);
    il2cpp::vm::Class::Init(obj->klass);
    if (!il2cpp::vm::Class::IsAssignableFrom(expectedClass, obj->klass))
        return false;

    if (expectedClass->byval_arg.valuetype)
    {
        const size_t sz = ValueMarshaling::GetValueTypeInstanceSize(expectedClass);
        if (sz > destSize)
            return false;
        std::memcpy(dest, il2cpp::vm::Object::Unbox(obj), sz);
        return true;
    }

    if (destSize < sizeof(Il2CppObject*))
        return false;

    *reinterpret_cast<Il2CppObject**>(dest) = obj;
    return true;
}

ConversionKind MarshalAsUserData::GetConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    return CanConvert(L, index, type) ? ConversionKind::Identity : ConversionKind::None;
}

bool MarshalAsUserData::CanConvert(lua_State* L, int index, const Il2CppType* type)
{
    if (type == nullptr || lua_type(L, index) != LUA_TUSERDATA)
        return false;

    if (PointerMarshal::IsPointerType(type))
        return false;

    Il2CppClass* typeClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (typeClass != nullptr)
    {
        il2cpp::vm::Class::Init(typeClass);
        if (PointerMarshal::IsUnsupportedMarshalType(typeClass))
            return false;
    }

    if (type->type == IL2CPP_TYPE_STRING)
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        return obj != nullptr && obj->klass->byval_arg.type == IL2CPP_TYPE_STRING;
    }

    if (IsPrimitiveIl2CppType(type))
    {
        Il2CppClass* boxClass = ResolvePrimitiveBoxClass(type);
        if (boxClass == nullptr)
            return false;

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        return obj != nullptr && il2cpp::vm::Class::IsAssignableFrom(boxClass, obj->klass);
    }

    if (typeClass == nullptr)
        return false;

    if (typeClass->enumtype)
    {
        if (StructMarshal::IsByValUserData(L, index))
        {
            ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
            return header != nullptr && header->klass == typeClass;
        }

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        return obj != nullptr && obj->klass == typeClass;
    }

    if (typeClass->byval_arg.valuetype)
    {
        if (StructMarshal::IsByValUserData(L, index))
        {
            ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
            return header != nullptr && header->klass == typeClass;
        }

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        return obj != nullptr && obj->klass == typeClass;
    }

    if (type->type == IL2CPP_TYPE_OBJECT)
    {
        return ObjectMarshal::Pop(L, index) != nullptr;
    }

    if (type->type == IL2CPP_TYPE_SZARRAY || type->type == IL2CPP_TYPE_ARRAY)
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return false;
        Il2CppClass* expectedClass = il2cpp::vm::Class::FromIl2CppType(type);
        return expectedClass != nullptr && il2cpp::vm::Class::IsAssignableFrom(expectedClass, obj->klass);
    }

    if (MetadataUtil::IsDelegateClass(typeClass))
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        return obj != nullptr && il2cpp::vm::Class::IsAssignableFrom(typeClass, obj->klass);
    }

    Il2CppObject* obj = ObjectMarshal::Pop(L, index);
    return obj != nullptr && il2cpp::vm::Class::IsAssignableFrom(typeClass, obj->klass);
}

bool MarshalAsUserData::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
{
    if (!CanConvert(L, index, type))
        return false;

    if (type->type == IL2CPP_TYPE_STRING)
    {
        if (destSize < sizeof(Il2CppString*))
            return false;
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        *reinterpret_cast<Il2CppString**>(dest) = reinterpret_cast<Il2CppString*>(obj);
        return true;
    }

    if (IsPrimitiveIl2CppType(type))
    {
        Il2CppClass* boxClass = ResolvePrimitiveBoxClass(type);
        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (boxClass == nullptr || obj == nullptr || !il2cpp::vm::Class::IsAssignableFrom(boxClass, obj->klass))
            return false;

        const size_t sz = ValueMarshaling::GetValueTypeInstanceSize(boxClass);
        if (sz > destSize)
            return false;
        std::memcpy(dest, il2cpp::vm::Object::Unbox(obj), sz);
        return true;
    }

    Il2CppClass* typeClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (typeClass == nullptr)
        return false;

    if (typeClass->enumtype || typeClass->byval_arg.valuetype)
    {
        if (StructMarshal::PopValue(L, index, typeClass, dest))
            return true;
        return TryPopBoxedObjectValue(L, index, typeClass, dest, destSize);
    }

    if (type->type == IL2CPP_TYPE_OBJECT || type->type == IL2CPP_TYPE_CLASS
        || type->type == IL2CPP_TYPE_SZARRAY || type->type == IL2CPP_TYPE_ARRAY
        || MetadataUtil::IsDelegateClass(typeClass))
    {
        return TryPopBoxedObjectValue(L, index, typeClass, dest, destSize);
    }

    return false;
}

int MarshalAsUserData::Push(lua_State* L, const Il2CppType* type, Il2CppObject* retObj)
{
    if (type == nullptr)
        return 0;

    if (retObj == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }

    if (type->type == IL2CPP_TYPE_STRING)
    {
        ObjectMarshal::Push(L, retObj);
        Il2CppClass* stringClass = il2cpp_defaults.string_class;
        if (stringClass != nullptr)
            ValueMarshaling::AttachInstanceMetatable(L, stringClass);
        return 1;
    }

    if (IsPrimitiveIl2CppType(type))
    {
        Il2CppClass* boxClass = ResolvePrimitiveBoxClass(type);
        if (boxClass == nullptr || !il2cpp::vm::Class::IsAssignableFrom(boxClass, retObj->klass))
            return luaL_error(L, "zlua: unsupported userdata return type");
        return PushBoxedByObjInstance(L, boxClass, retObj);
    }

    Il2CppClass* typeClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (typeClass == nullptr)
        return luaL_error(L, "zlua: unsupported userdata return type");

    il2cpp::vm::Class::Init(typeClass);
    if (typeClass->enumtype || IsPrimitiveIl2CppType(&typeClass->byval_arg))
    {
        if (retObj->klass != typeClass)
            return luaL_error(L, "zlua: userdata return type mismatch");
        return PushBoxedByObjInstance(L, typeClass, retObj);
    }

    if (typeClass->byval_arg.valuetype)
    {
        if (retObj->klass != typeClass)
            return luaL_error(L, "zlua: userdata return type mismatch");
        return PushBoxedByObjInstance(L, typeClass, retObj);
    }

    ObjectMarshal::Push(L, retObj);
    ValueMarshaling::AttachInstanceMetatable(L, retObj->klass);
    return 1;
}

static bool IsByteArrayType(const Il2CppType* type)
{
    if (type == nullptr || type->type != IL2CPP_TYPE_SZARRAY)
        return false;

    Il2CppClass* arrayClass = il2cpp::vm::Class::FromIl2CppType(type);
    if (arrayClass == nullptr || arrayClass->element_class == nullptr)
        return false;

    return arrayClass->element_class->byval_arg.type == IL2CPP_TYPE_U1;
}

ConversionKind MarshalAsBytes::GetConversionKind(lua_State* L, int index, const Il2CppType* type)
{
    if (type == nullptr)
        return ConversionKind::None;

    if (!IsByteArrayType(type) && type->type != IL2CPP_TYPE_STRING)
        return ConversionKind::None;

    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
        return ConversionKind::NullLiteral;
    if (luaType == LUA_TSTRING)
        return ConversionKind::Identity;
    return ConversionKind::None;
}

bool MarshalAsBytes::CanConvert(lua_State* L, int index, const Il2CppType* type)
{
    return GetConversionKind(L, index, type) != ConversionKind::None;
}

static Il2CppArray* CreateByteArrayFromLuaString(lua_State* L, int index, Il2CppClass* arrayClass)
{
    size_t length = 0;
    const char* bytes = lua_tolstring(L, index, &length);
    if (bytes == nullptr && length != 0)
        return nullptr;

    Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)length);
    if (array == nullptr)
        return nullptr;

    if (length > 0)
    {
        uint8_t* dest = reinterpret_cast<uint8_t*>(il2cpp::vm::Array::GetFirstElementAddress(array));
        std::memcpy(dest, bytes, length);
    }

    return array;
}

bool MarshalAsBytes::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
{
    if (dest == nullptr || destSize < sizeof(Il2CppObject*))
        return false;

    const int luaType = lua_type(L, index);
    if (luaType == LUA_TNIL)
    {
        *reinterpret_cast<Il2CppObject**>(dest) = nullptr;
        return true;
    }

    if (luaType != LUA_TSTRING)
        return false;

    if (IsByteArrayType(type))
    {
        Il2CppClass* arrayClass = il2cpp::vm::Class::FromIl2CppType(type);
        if (arrayClass == nullptr)
            return false;

        Il2CppArray* array = CreateByteArrayFromLuaString(L, index, arrayClass);
        if (array == nullptr)
            return false;

        *reinterpret_cast<Il2CppObject**>(dest) = reinterpret_cast<Il2CppObject*>(array);
        return true;
    }

    if (type->type == IL2CPP_TYPE_STRING)
        return StringMarshal::TryPop(L, index, reinterpret_cast<Il2CppString**>(dest));

    return false;
}

static void PushByteArrayAsLuaString(lua_State* L, Il2CppArray* array)
{
    if (array == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    const il2cpp_array_size_t length = il2cpp::vm::Array::GetLength(array);
    if (length == 0)
    {
        lua_pushlstring(L, "", 0);
        return;
    }

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(il2cpp::vm::Array::GetFirstElementAddress(array));
    lua_pushlstring(L, reinterpret_cast<const char*>(bytes), (size_t)length);
}

int MarshalAsBytes::Push(lua_State* L, const Il2CppType* type, Il2CppObject* retObj)
{
    if (IsByteArrayType(type))
    {
        PushByteArrayAsLuaString(L, retObj != nullptr ? reinterpret_cast<Il2CppArray*>(retObj) : nullptr);
        return 1;
    }

    if (type->type == IL2CPP_TYPE_STRING)
    {
        StringMarshal::Push(L, reinterpret_cast<Il2CppString*>(retObj));
        return 1;
    }

    return luaL_error(L, "zlua: unsupported bytes return type");
}

int MarshalAsOpaque::Push(lua_State* L, const Il2CppType* type, Il2CppObject* retObj)
{
    if (type == nullptr)
        return 0;

    if (retObj == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }

    Il2CppClass* structClass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (structClass == nullptr || !ValueMarshaling::IsStructClass(structClass))
        return luaL_error(L, "zlua: unsupported opaque return type");

    if (retObj->klass != structClass)
        return luaL_error(L, "zlua: opaque return type mismatch");

    const intptr_t handle = StructOpaqueScope::RegisterStruct(retObj, structClass);
    if (handle == 0)
    {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, reinterpret_cast<void*>(handle));
    return 1;
}
} // namespace zlua
