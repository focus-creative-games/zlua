#include "MarshalMeta.h"

#include "ArrayMarshal.h"
#include "IntrinsicTypes.h"
#include "ObjectMarshal.h"
#include "OpaqueValueMarshal.h"
#include "StringMarshal.h"
#include "StructMarshal.h"
#include "PrimitiveMarshal.h"

#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../mt/MetaTableCache.h"

#include "gc/GarbageCollector.h"
#include "utils/StringUtils.h"
#include "vm/Exception.h"
#include "vm/Class.h"
#include "vm/GenericClass.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Array.h"
#include "vm/MetadataCache.h"
#include "vm/GlobalMetadata.h"
#include "vm/Reflection.h"

#include "../utils/Collection.h"

#include <string>
#include <vector>

namespace zlua
{
static void Lua2CSMarshalVoid(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)L;
    (void)valueIdx;
    (void)address;
    (void)ctx;
}

static void CS2LuaMarshalVoid(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)L;
    (void)address;
    (void)ctx;
}

static void Lua2CSMarshalBool(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(bool*)address = PrimitiveMarshal::PopBool(L, valueIdx);
}

static void CS2LuaMarshalBool(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    PrimitiveMarshal::PushBool(L, *(bool*)address);
}

static void Lua2CSMarshalSByte(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(int8_t*)address = PrimitiveMarshal::PopInt8(L, valueIdx);
}

static void CS2LuaMarshalSByte(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    PrimitiveMarshal::PushInt8(L, *(int8_t*)address);
}

static void Lua2CSMarshalByte(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(uint8_t*)address = PrimitiveMarshal::PopUInt8(L, valueIdx);
}

static void CS2LuaMarshalByte(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushUInt8(L, *(uint8_t*)address);
}

static void Lua2CSMarshalShort(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(int16_t*)address = PrimitiveMarshal::PopInt16(L, valueIdx);
}

static void CS2LuaMarshalShort(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushInt16(L, *(int16_t*)address);
}

static void Lua2CSMarshalUShort(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(uint16_t*)address = PrimitiveMarshal::PopUInt16(L, valueIdx);
}

static void CS2LuaMarshalUShort(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushUInt16(L, *(uint16_t*)address);
}

static void Lua2CSMarshalInt(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(int32_t*)address = PrimitiveMarshal::PopInt32(L, valueIdx);
}

static void CS2LuaMarshalInt(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushInt32(L, *(int32_t*)address);
}

static void Lua2CSMarshalUInt(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(uint32_t*)address = PrimitiveMarshal::PopUInt32(L, valueIdx);
}

static void CS2LuaMarshalUInt(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushUInt32(L, *(uint32_t*)address);
}

static void Lua2CSMarshalLong(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(int64_t*)address = PrimitiveMarshal::PopInt64(L, valueIdx);
}

static void CS2LuaMarshalLong(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushInt64(L, *(int64_t*)address);
}

static void Lua2CSMarshalULong(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(uint64_t*)address = PrimitiveMarshal::PopUInt64(L, valueIdx);
}

static void CS2LuaMarshalULong(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushUInt64(L, *(uint64_t*)address);
}

static void Lua2CSMarshalFloat(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(float*)address = PrimitiveMarshal::PopFloat(L, valueIdx);
}

static void CS2LuaMarshalFloat(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushFloat(L, *(float*)address);
}

static void Lua2CSMarshalDouble(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(double*)address = PrimitiveMarshal::PopDouble(L, valueIdx);
}

static void CS2LuaMarshalDouble(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushDouble(L, *(double*)address);
}

static void Lua2CSMarshalIntPtr(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(intptr_t*)address = PrimitiveMarshal::PopIntPtr(L, valueIdx);
}

static void CS2LuaMarshalIntPtr(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    PrimitiveMarshal::PushIntPtr(L, *(intptr_t*)address);
}

static void Lua2CSMarshalUIntPtr(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(uintptr_t*)address = PrimitiveMarshal::PopUIntPtr(L, valueIdx);
}

static void CS2LuaMarshalUIntPtr(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    PrimitiveMarshal::PushUIntPtr(L, *(uintptr_t*)address);
}

static void Lua2CSMarshalPointer(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    *(void**)address = PrimitiveMarshal::PopPointer(L, valueIdx);
}

static void CS2LuaMarshalPointer(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    PrimitiveMarshal::PushPointer(L, *(void**)address);
}

static void Lua2CSMarshalObject(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    Il2CppObject* value = ObjectMarshal::Pop(L, valueIdx, ctx->typeKlass);
    *(Il2CppObject**)address = value;
    // il2cpp::gc::WriteBarrier::GenericStore((Il2CppObject**)address, value);
}

static void CS2LuaMarshalObject(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    ObjectMarshal::Push(L, *(Il2CppObject**)address, ctx);
}

static void Lua2CSMarshalNullable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PopNullableValue(L, valueIdx, ctx->typeKlass, address);
}

static void CS2LuaMarshalNullable(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PushNullableValue(L, address, ctx->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx));
}

static void Lua2CSMarshalString(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    Il2CppString* value = StringMarshal::Pop(L, valueIdx);
    *(Il2CppString**)address = value;
    // il2cpp::gc::WriteBarrier::GenericStore((Il2CppString**)address, value);
}

static void CS2LuaMarshalString(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    StringMarshal::Push(L, *(Il2CppString**)address);
}

static void Lua2CSMarshalStruct(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PopValue(L, valueIdx, ctx->typeKlass, address);
}

static void CS2LuaMarshalStruct(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PushValue(L, address, ctx->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx));
}

static void Lua2CSMarshalUnityVector2(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, valueIdx);
    if (header == nullptr)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert non-by-val userdata to struct: %s.%s", ctx->typeKlass->namespaze,
                                  ctx->typeKlass->name);
    if (header->klass != ctx->typeKlass)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert userdata %s.%s to struct: %s.%s", header->klass->namespaze, header->klass->name,
                                  ctx->typeKlass->namespaze, ctx->typeKlass->name);
    *reinterpret_cast<UnityVector2*>(address) = *reinterpret_cast<const UnityVector2*>(header->Payload());
}

static void CS2LuaMarshalUnityVector2(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PushValue(L, address, ctx->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx));
}

static void Lua2CSMarshalUnityVector3(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, valueIdx);
    if (header == nullptr)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert non-by-val userdata to struct: %s.%s", ctx->typeKlass->namespaze,
                                  ctx->typeKlass->name);
    if (header->klass != ctx->typeKlass)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert userdata %s.%s to struct: %s.%s", header->klass->namespaze, header->klass->name,
                                  ctx->typeKlass->namespaze, ctx->typeKlass->name);
    *reinterpret_cast<UnityVector3*>(address) = *reinterpret_cast<const UnityVector3*>(header->Payload());
}

static void CS2LuaMarshalUnityVector3(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PushValue(L, address, ctx->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx));
}

static void Lua2CSMarshalUnityVector4(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, valueIdx);
    if (header == nullptr)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert non-by-val userdata to struct: %s.%s", ctx->typeKlass->namespaze,
                                  ctx->typeKlass->name);
    if (header->klass != ctx->typeKlass)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert userdata %s.%s to struct: %s.%s", header->klass->namespaze, header->klass->name,
                                  ctx->typeKlass->namespaze, ctx->typeKlass->name);
    *reinterpret_cast<UnityVector4*>(address) = *reinterpret_cast<const UnityVector4*>(header->Payload());
}

static void CS2LuaMarshalUnityVector4(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    StructMarshal::PushValue(L, address, ctx->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx));
}

static void Lua2CSMarshalVar(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetNotSupportedException("can't marshal VAR or MVAR type"));
}

static void CS2LuaMarshalVar(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetNotSupportedException("can't marshal VAR or MVAR type"));
}

static void Lua2CsMarshalTypedReference(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    LuaException::Throw("zlua: can't marshal typed reference type");
}

static void CS2LuaMarshalTypedReference(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    LuaException::Throw("zlua: can't marshal typed reference type");
}

static Il2CppClass* GetMetatableKlass(const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    if (klass != nullptr && klass->nullabletype)
        return klass->element_class;
    return klass;
}

int MarshalMeta::EnsureByValMetatableRefSlow(lua_State* L, MarshalMetaInfo* meta)
{
    Il2CppClass* klass = GetMetatableKlass(meta);
    IL2CPP_ASSERT(klass != nullptr);
    meta->luaByValRefIndex = MetaTableCache::GetOrCreateByValMetatableRef(L, klass);
    return meta->luaByValRefIndex;
}

int MarshalMeta::EnsureByObjMetatableRefSlow(lua_State* L, MarshalMetaInfo* meta)
{
    Il2CppClass* klass = GetMetatableKlass(meta);
    IL2CPP_ASSERT(klass != nullptr);
    meta->luaByObjRefIndex = MetaTableCache::GetOrCreateByObjMetatableRef(L, klass);
    return meta->luaByObjRefIndex;
}

struct LuaMarshalAsData
{
    LuaMarshalType marshalType = LuaMarshalType::Default;
    std::vector<std::string> fieldOrPropertyNames;
};

static void FillLuaMarshalAsDataFromAttribute(Il2CppObject* attr, LuaMarshalAsData& data)
{
    data.marshalType = LuaMarshalType::Default;
    data.fieldOrPropertyNames.clear();

    const PropertyInfo* marshalTypeProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "LuaMarshalType");
    IL2CPP_ASSERT(marshalTypeProperty != nullptr && marshalTypeProperty->get != nullptr);

    Il2CppException* exc = nullptr;
    Il2CppObject* enumValue = il2cpp::vm::Runtime::Invoke(marshalTypeProperty->get, attr, nullptr, &exc);
    IL2CPP_ASSERT(exc == nullptr && enumValue != nullptr && enumValue->klass->enumtype);

    const int32_t rawValue = *reinterpret_cast<int32_t*>(il2cpp::vm::Object::Unbox(enumValue));
    IL2CPP_ASSERT(rawValue >= 0 && rawValue <= static_cast<int32_t>(LuaMarshalType::ParamsTable));
    data.marshalType = static_cast<LuaMarshalType>(rawValue);

    const PropertyInfo* namesProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "FieldOrPropertyNames");
    if (namesProperty != nullptr && namesProperty->get != nullptr)
    {
        exc = nullptr;
        Il2CppObject* namesObj = il2cpp::vm::Runtime::Invoke(namesProperty->get, attr, nullptr, &exc);
        IL2CPP_ASSERT(exc == nullptr);
        if (namesObj != nullptr)
        {
            Il2CppArray* names = reinterpret_cast<Il2CppArray*>(namesObj);
            data.fieldOrPropertyNames.reserve(static_cast<size_t>(names->max_length));
            for (il2cpp_array_size_t i = 0; i < names->max_length; ++i)
            {
                Il2CppString* nameStr = il2cpp_array_get(names, Il2CppString*, i);
                if (nameStr == nullptr)
                    continue;
                data.fieldOrPropertyNames.emplace_back(
                    il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(nameStr), il2cpp::utils::StringUtils::GetLength(nameStr)));
            }
        }
    }
}

static bool TryParseLuaMarshalAsData(const Il2CppImage* image, uint32_t token, LuaMarshalAsData& data)
{
    if (token == 0)
        return false;
    if (!MetadataUtil::HasParameterMarshalAsAttribute(image, token))
        return false;

    Il2CppMetadataCustomAttributeHandle customAttributeHandle = il2cpp::vm::GlobalMetadata::GetCustomAttributeTypeToken(image, token);
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(customAttributeHandle, MetadataUtil::GetLuaMarshalAsAttributeClass());
    IL2CPP_ASSERT(attr != nullptr);
    FillLuaMarshalAsDataFromAttribute(attr, data);
    return true;
}

static Il2CppClass* GetLuaMarshalAsAttributeOwnerClass(Il2CppClass* klass)
{
    il2cpp::vm::Class::Init(klass);
    if (klass->nullabletype)
        klass = klass->element_class;
    if (klass->generic_class != nullptr)
        klass = il2cpp::vm::GenericClass::GetTypeDefinition(klass->generic_class);
    return klass;
}

// Cache parse results for types that declare [LuaMarshalAs] (keyed by attribute owner klass).
static AppendOnlyRawPointerHashMap<Il2CppClass, LuaMarshalAsData> s_typeLuaMarshalAsCache;

static bool TryParseLuaMarshalAsDataFromType(Il2CppClass* klass, LuaMarshalAsData& data)
{
    Il2CppClass* owner = GetLuaMarshalAsAttributeOwnerClass(klass);

    auto it = s_typeLuaMarshalAsCache.find(owner);
    if (it != s_typeLuaMarshalAsCache.end())
    {
        data = it->second;
        return true;
    }

    Il2CppClass* attrClass = MetadataUtil::GetLuaMarshalAsAttributeClass();
    if (!il2cpp::vm::Class::HasAttribute(owner, attrClass))
        return false;

    Il2CppMetadataCustomAttributeHandle handle = il2cpp::vm::GlobalMetadata::GetCustomAttributeTypeToken(owner->image, owner->token);
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, attrClass);
    IL2CPP_ASSERT(attr != nullptr);
    FillLuaMarshalAsDataFromAttribute(attr, data);
    s_typeLuaMarshalAsCache.insert({klass, data});
    return true;
}

static bool TryResolveLuaMarshalAsData(const Il2CppImage* image, uint32_t token, Il2CppClass* typeKlass, LuaMarshalAsData& data)
{
    if (TryParseLuaMarshalAsData(image, token, data))
        return true;

    return TryParseLuaMarshalAsDataFromType(typeKlass, data);
}

static void Lua2CSMarshalByteArrayAsBytes(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    *(Il2CppArray**)address = ArrayMarshal::PopFromBytes(L, valueIdx, ctx->typeKlass);
}

static void CS2LuaMarshalByteArrayAsBytes(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    (void)ctx;
    Il2CppArray* array = *(Il2CppArray**)address;
    if (array == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    ArrayMarshal::PushAsBytes(L, array);
}

static void Lua2CsMarshalArrayAsTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    *(Il2CppArray**)address = ArrayMarshal::PopFromTable(L, valueIdx, ctx->typeKlass);
}

static void CS2LuaMarshalArrayAsTable(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    Il2CppArray* array = *(Il2CppArray**)address;
    ArrayMarshal::PushAsTable(L, array);
}

static void Lua2CSMarshalOpaque(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx)
{
    OpaqueValueMarshal::Pop(L, valueIdx, address, ctx->type);
}

static void CS2LuaMarshalOpaque(lua_State* L, void* address, const MarshalMetaInfo* ctx)
{
    OpaqueValueMarshal::Push(L, address, ctx->type);
}

static void ApplySizeAndPassByValueAndKlass(MarshalMetaInfo* meta, const Il2CppType* type, Il2CppClass* klass)
{
    IL2CPP_ASSERT(!type->byref);
    meta->typeKlass = klass;
    // meta->type = type;

    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
    case IL2CPP_TYPE_STRING:
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
    {
        meta->size = sizeof(Il2CppObject*);
        meta->passByValue = true;
        break;
    }
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
    {
        meta->size = 0;
        meta->passByValue = false;
        break;
    }
    case IL2CPP_TYPE_TYPEDBYREF:
    {
        meta->size = sizeof(Il2CppTypedRef);
        meta->passByValue = false;
        break;
    }
    case IL2CPP_TYPE_GENERICINST:
    {
        if (MetadataUtil::IsReferenceType(type))
        {
            meta->size = sizeof(Il2CppObject*);
            meta->passByValue = true;
        }
        else
        {
            meta->size = MetadataUtil::GetValueSize(type);
            meta->passByValue = false;
        }
        break;
    }
    default:
    {
        IL2CPP_ASSERT(!MetadataUtil::IsReferenceType(type));
        meta->size = MetadataUtil::GetValueSize(type);
        meta->passByValue = false;
        break;
    }
    }
}

static void ApplyDefaultMarshalWriters(MarshalMetaInfo* meta, const Il2CppType* type, Il2CppClass* klass)
{
restart:
    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
        meta->lua2csWriter = Lua2CSMarshalVoid;
        meta->cs2luaWriter = CS2LuaMarshalVoid;
        break;
    case IL2CPP_TYPE_BOOLEAN:
        meta->lua2csWriter = Lua2CSMarshalBool;
        meta->cs2luaWriter = CS2LuaMarshalBool;
        break;
    case IL2CPP_TYPE_I1:
        meta->lua2csWriter = Lua2CSMarshalSByte;
        meta->cs2luaWriter = CS2LuaMarshalSByte;
        break;
    case IL2CPP_TYPE_U1:
        meta->lua2csWriter = Lua2CSMarshalByte;
        meta->cs2luaWriter = CS2LuaMarshalByte;
        break;
    case IL2CPP_TYPE_I2:
        meta->lua2csWriter = Lua2CSMarshalShort;
        meta->cs2luaWriter = CS2LuaMarshalShort;
        break;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        meta->lua2csWriter = Lua2CSMarshalUShort;
        meta->cs2luaWriter = CS2LuaMarshalUShort;
        break;
    case IL2CPP_TYPE_I4:
        meta->lua2csWriter = Lua2CSMarshalInt;
        meta->cs2luaWriter = CS2LuaMarshalInt;
        break;
    case IL2CPP_TYPE_U4:
        meta->lua2csWriter = Lua2CSMarshalUInt;
        meta->cs2luaWriter = CS2LuaMarshalUInt;
        break;
    case IL2CPP_TYPE_I8:
        meta->lua2csWriter = Lua2CSMarshalLong;
        meta->cs2luaWriter = CS2LuaMarshalLong;
        break;
    case IL2CPP_TYPE_U8:
        meta->lua2csWriter = Lua2CSMarshalULong;
        meta->cs2luaWriter = CS2LuaMarshalULong;
        break;
    case IL2CPP_TYPE_R4:
        meta->lua2csWriter = Lua2CSMarshalFloat;
        meta->cs2luaWriter = CS2LuaMarshalFloat;
        break;
    case IL2CPP_TYPE_R8:
        meta->lua2csWriter = Lua2CSMarshalDouble;
        meta->cs2luaWriter = CS2LuaMarshalDouble;
        break;
    case IL2CPP_TYPE_I:
        meta->lua2csWriter = Lua2CSMarshalIntPtr;
        meta->cs2luaWriter = CS2LuaMarshalIntPtr;
        break;
    case IL2CPP_TYPE_U:
        meta->lua2csWriter = Lua2CSMarshalUIntPtr;
        meta->cs2luaWriter = CS2LuaMarshalUIntPtr;
        break;
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        meta->lua2csWriter = Lua2CSMarshalPointer;
        meta->cs2luaWriter = CS2LuaMarshalPointer;
        break;
    case IL2CPP_TYPE_STRING:
        meta->lua2csWriter = Lua2CSMarshalString;
        meta->cs2luaWriter = CS2LuaMarshalString;
        break;
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        meta->lua2csWriter = Lua2CSMarshalObject;
        meta->cs2luaWriter = CS2LuaMarshalObject;
        break;
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        if (MetadataUtil::IsReferenceType(type))
        {
            meta->lua2csWriter = Lua2CSMarshalObject;
            meta->cs2luaWriter = CS2LuaMarshalObject;
        }
        else if (klass->enumtype)
        {
            type = il2cpp::vm::Class::GetEnumBaseType(klass);
            goto restart;
        }
        else if (klass->nullabletype)
        {
            meta->lua2csWriter = Lua2CSMarshalNullable;
            meta->cs2luaWriter = CS2LuaMarshalNullable;
        }
        else
        {
            switch (GetIntrinsicType(klass->namespaze, klass->name))
            {
            case IntrinsicTypeKind::Vector2:
                meta->lua2csWriter = Lua2CSMarshalUnityVector2;
                meta->cs2luaWriter = CS2LuaMarshalUnityVector2;
                break;
            case IntrinsicTypeKind::Vector3:
                meta->lua2csWriter = Lua2CSMarshalUnityVector3;
                meta->cs2luaWriter = CS2LuaMarshalUnityVector3;
                break;
            case IntrinsicTypeKind::Vector4:
                meta->lua2csWriter = Lua2CSMarshalUnityVector4;
                meta->cs2luaWriter = CS2LuaMarshalUnityVector4;
                break;
            default:
                meta->lua2csWriter = Lua2CSMarshalStruct;
                meta->cs2luaWriter = CS2LuaMarshalStruct;
                break;
            }
        }
        break;
    }
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
        meta->lua2csWriter = Lua2CSMarshalVar;
        meta->cs2luaWriter = CS2LuaMarshalVar;
        break;
    case IL2CPP_TYPE_TYPEDBYREF:
        meta->lua2csWriter = Lua2CsMarshalTypedReference;
        meta->cs2luaWriter = CS2LuaMarshalTypedReference;
        break;
    default:
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentException("type", "Invalid type"));
        break;
    }
}

static bool TryApplyDeclaredMarshalWriters(MarshalMetaInfo* meta, const Il2CppType* type, Il2CppClass* klass, const LuaMarshalAsData& data)
{
    LuaMarshalType marshalType = data.marshalType;

    switch (marshalType)
    {
    case LuaMarshalType::Default:
        return false;
    case LuaMarshalType::UserData:
    {
        // Meaningful override: string Default is Lua string; UserData forces ByObj userdata.
        if (type->type == IL2CPP_TYPE_STRING)
        {
            meta->lua2csWriter = Lua2CSMarshalObject;
            meta->cs2luaWriter = CS2LuaMarshalObject;
            return true;
        }
        // class / array / struct / object already default to userdata.
        return false;
    }
    case LuaMarshalType::Bytes:
    {
        if (MetadataUtil::IsByteArrayClass(klass))
        {
            meta->lua2csWriter = Lua2CSMarshalByteArrayAsBytes;
            meta->cs2luaWriter = CS2LuaMarshalByteArrayAsBytes;
            return true;
        }
        return false;
    }
    case LuaMarshalType::OpaqueLightUserData:
    {
        meta->lua2csWriter = Lua2CSMarshalOpaque;
        meta->cs2luaWriter = CS2LuaMarshalOpaque;
        return true;
    }
    case LuaMarshalType::UnpackedValues:
    {
        // TODO: Implement UnpackedValues push/pop writers.
        return false;
    }
    case LuaMarshalType::Table:
    {
        if (MetadataUtil::IsSzArrayClass(klass))
        {
            meta->lua2csWriter = Lua2CsMarshalArrayAsTable;
            meta->cs2luaWriter = CS2LuaMarshalArrayAsTable;
            return true;
        }
        // TODO: Implement Table push/pop writers.
        return false;
    }
    case LuaMarshalType::ParamsTable:
    {
        if (MetadataUtil::IsSzArrayClass(klass))
        {
            meta->lua2csWriter = Lua2CsMarshalArrayAsTable;
            meta->cs2luaWriter = CS2LuaMarshalArrayAsTable;
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

static MarshalMetaInfo* CreateByType(lua_State* L, const Il2CppType* type, const Il2CppImage* image, uint32_t token)
{
    (void)L;
    MarshalMetaInfo* meta = LuaMetadataAlloc::MallocAnyZeroed<MarshalMetaInfo>();
    meta->type = type;
    meta->luaByValRefIndex = LUA_NOREF;
    meta->luaByObjRefIndex = LUA_NOREF;
    if (type->byref)
    {
        meta->lua2csWriter = Lua2CSMarshalOpaque;
        meta->cs2luaWriter = CS2LuaMarshalOpaque;
        meta->size = sizeof(uintptr_t);
        meta->passByValue = true;
        return meta;
    }

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
    il2cpp::vm::Class::Init(klass);

    ApplySizeAndPassByValueAndKlass(meta, type, klass);

    LuaMarshalAsData marshalAs;

    if (!TryResolveLuaMarshalAsData(image, token, klass, marshalAs) || !TryApplyDeclaredMarshalWriters(meta, type, klass, marshalAs))
        ApplyDefaultMarshalWriters(meta, type, klass);

    return meta;
}

MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const MethodInfo* method, int argIndex)
{
    uint32_t token = MetadataUtil::GetParameterToken(method, argIndex);
    const Il2CppType* type = argIndex >= 0 ? method->parameters[argIndex] : method->return_type;
    return CreateByType(L, type, method->klass->image, token);
}

MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const FieldInfo* field)
{
    return CreateByType(L, field->type, field->parent->image, field->token);
}

MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const PropertyInfo* property)
{
    const Il2CppType* type = property->get != nullptr ? property->get->return_type : property->set->parameters[0];
    return CreateByType(L, type, property->parent->image, property->token);
}
} // namespace zlua