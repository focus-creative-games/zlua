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

#include "MarshalMeta.h"

#include "ArrayMarshal.h"
#include "CompositeMarshal.h"
#include "CompositeSpecializedTable.h"
#include "MarshalAsXmlTable.h"
#include "IntrinsicTypes.h"
#include "ObjectMarshal.h"
#include "OpaqueValueMarshal.h"
#include "StringMarshal.h"
#include "StructMarshal.h"
#include "PrimitiveMarshal.h"

#include "../bridge/FieldBridge.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../mt/MetaTableCache.h"

#include "gc/GarbageCollector.h"
#include "utils/StringUtils.h"
#include "vm/Exception.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/GenericClass.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Array.h"
#include "vm/MetadataCache.h"
#include "vm/GlobalMetadata.h"
#include "vm/Reflection.h"

#include <string>
#include <unordered_map>
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

    const int32_t rawValue = *reinterpret_cast<int32_t*>(ZLuaObjectUnbox(enumValue));
    IL2CPP_ASSERT(rawValue >= 0 && rawValue <= static_cast<int32_t>(LuaMarshalType::Table));
    data.marshalType = static_cast<LuaMarshalType>(rawValue);

    const PropertyInfo* namesProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "Members");
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

    // Type-level [LuaMarshalAs] is illegal on generic definitions and does not
    // apply to closed generic instances (spec §1.1) — do not walk to type def.
    if (klass->is_generic || klass->generic_class != nullptr)
        return nullptr;

    return klass;
}

// Cache parse results for types that declare [LuaMarshalAs] (keyed by attribute owner klass).
static std::unordered_map<const Il2CppClass*, LuaMarshalAsData> s_typeLuaMarshalAsCache;

static bool TryParseLuaMarshalAsDataFromType(Il2CppClass* klass, LuaMarshalAsData& data)
{
    Il2CppClass* owner = GetLuaMarshalAsAttributeOwnerClass(klass);
    if (owner == nullptr)
        return false;

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
    s_typeLuaMarshalAsCache.insert({owner, data});
    return true;
}

static bool CopyXmlData(const LuaMarshalAsResolvedData& xml, LuaMarshalAsData& data)
{
    data.marshalType = xml.marshalType;
    data.fieldOrPropertyNames = xml.members;
    return data.marshalType != LuaMarshalType::Default;
}

enum class LuaMarshalAsResolveKind : uint8_t
{
    None = 0,
    DeclaredOnMember = 1, // param/field/property attribute or XML slot
    TypeLevel = 2,
};

static bool TryResolveTypeLevelLuaMarshalAsData(Il2CppClass* typeKlass, LuaMarshalAsData& data)
{
    if (TryParseLuaMarshalAsDataFromType(typeKlass, data))
        return true;

    LuaMarshalAsResolvedData xml;
    return MarshalAsXmlTable::TryGetForType(typeKlass, xml) && CopyXmlData(xml, data);
}

static LuaMarshalAsResolveKind TryResolveLuaMarshalAsDataForMethodSlot(
    const Il2CppImage* image,
    uint32_t paramOrReturnToken,
    const MethodInfo* method,
    int argIndex,
    Il2CppClass* typeKlass,
    LuaMarshalAsData& data)
{
    if (TryParseLuaMarshalAsData(image, paramOrReturnToken, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    LuaMarshalAsResolvedData xml;
    if (MarshalAsXmlTable::TryGetForMethodSlot(method, argIndex, xml) && CopyXmlData(xml, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    if (TryResolveTypeLevelLuaMarshalAsData(typeKlass, data))
        return LuaMarshalAsResolveKind::TypeLevel;
    return LuaMarshalAsResolveKind::None;
}

static LuaMarshalAsResolveKind TryResolveLuaMarshalAsDataForField(const FieldInfo* field, Il2CppClass* typeKlass, LuaMarshalAsData& data)
{
    if (TryParseLuaMarshalAsData(field->parent->image, field->token, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    LuaMarshalAsResolvedData xml;
    if (MarshalAsXmlTable::TryGet(field->parent->image, field->token, xml) && CopyXmlData(xml, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    if (TryResolveTypeLevelLuaMarshalAsData(typeKlass, data))
        return LuaMarshalAsResolveKind::TypeLevel;
    return LuaMarshalAsResolveKind::None;
}

static LuaMarshalAsResolveKind TryResolveLuaMarshalAsDataForProperty(const PropertyInfo* property, Il2CppClass* typeKlass, LuaMarshalAsData& data)
{
    if (TryParseLuaMarshalAsData(property->parent->image, property->token, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    LuaMarshalAsResolvedData xml;
    if (MarshalAsXmlTable::TryGet(property->parent->image, property->token, xml) && CopyXmlData(xml, data))
        return LuaMarshalAsResolveKind::DeclaredOnMember;

    if (TryResolveTypeLevelLuaMarshalAsData(typeKlass, data))
        return LuaMarshalAsResolveKind::TypeLevel;
    return LuaMarshalAsResolveKind::None;
}

static void SpecializedNullableTable_Lua2Cs(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    if (lua_isnil(L, valueIdx))
    {
        MetadataUtil::InitNullableValue(address, klass);
        return;
    }

    void* valueAddr = MetadataUtil::GetNullableValue(address, klass);
    FnMarshalLua2Cs inner = nullptr;
    if (!CompositeSpecializedTable::TryGet(klass->element_class, LuaMarshalType::Table, &inner, nullptr, nullptr) || inner == nullptr)
    {
        LuaException::ThrowFormat(
            "zlua: missing specialized Table writer for Nullable<%s.%s>",
            klass->element_class->namespaze,
            klass->element_class->name);
    }
    inner(L, valueIdx, valueAddr, meta);
    MetadataUtil::NullableSetHasValue(address, klass);
}

static void SpecializedNullableTable_Cs2Lua(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    if (!il2cpp::vm::Object::NullableHasValue(klass, address))
    {
        lua_pushnil(L);
        return;
    }

    void* valueAddr = MetadataUtil::GetNullableValue(address, klass);
    FnMarshalCs2Lua inner = nullptr;
    if (!CompositeSpecializedTable::TryGet(klass->element_class, LuaMarshalType::Table, nullptr, &inner, nullptr) || inner == nullptr)
    {
        LuaException::ThrowFormat(
            "zlua: missing specialized Table writer for Nullable<%s.%s>",
            klass->element_class->namespaze,
            klass->element_class->name);
    }
    inner(L, valueAddr, meta);
}

static bool TryAttachCompositeSpecializedWriters(
    MarshalMetaInfo* meta,
    Il2CppClass* klass,
    LuaMarshalType marshalType,
    bool fromTypeLevel)
{
    if (!fromTypeLevel)
        return false;

    Il2CppClass* specializedKlass = klass;
    if (klass->nullabletype)
    {
        if (marshalType != LuaMarshalType::Table)
            return false;
        specializedKlass = klass->element_class;
    }

    FnMarshalLua2Cs lua2cs = nullptr;
    FnMarshalCs2Lua cs2lua = nullptr;
    uint16_t stackSlots = 1;
    if (!CompositeSpecializedTable::TryGet(specializedKlass, marshalType, &lua2cs, &cs2lua, &stackSlots))
        return false;

    if (klass->nullabletype)
    {
        meta->lua2csWriter = SpecializedNullableTable_Lua2Cs;
        meta->cs2luaWriter = SpecializedNullableTable_Cs2Lua;
    }
    else
    {
        meta->lua2csWriter = lua2cs;
        meta->cs2luaWriter = cs2lua;
    }
    meta->stackSlots = stackSlots;
    return true;
}

static MarshalMetaInfo* AllocMarshalMeta(const Il2CppType* type)
{
    MarshalMetaInfo* meta = LuaMetadataAlloc::MallocAnyZeroed<MarshalMetaInfo>();
    meta->type = type;
    meta->luaByValRefIndex = LUA_NOREF;
    meta->luaByObjRefIndex = LUA_NOREF;
    meta->marshalType = LuaMarshalType::Default;
    meta->stackSlots = 1;
    meta->memberCount = 0;
    meta->members = nullptr;
    return meta;
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

static bool IsCompositeTargetValid(LuaMarshalType marshalType, Il2CppClass* klass)
{
    if (klass == nullptr)
        return false;
    if (marshalType == LuaMarshalType::Table)
    {
        if (klass->nullabletype)
        {
            Il2CppClass* arg = klass->element_class;
            return arg != nullptr && arg->byval_arg.valuetype && !arg->enumtype && !il2cpp::vm::Class::IsInterface(arg);
        }
        if (il2cpp::vm::Class::IsInterface(klass))
            return false;
        return klass->byval_arg.valuetype && !klass->enumtype;
    }
    if (marshalType == LuaMarshalType::UnpackedValues)
    {
        if (klass->nullabletype)
            return false;
        if (il2cpp::vm::Class::IsInterface(klass))
            return false;
        return klass->byval_arg.valuetype && !klass->enumtype;
    }
    return false;
}

static MarshalMetaInfo* CreateByRefOpaqueMeta(const Il2CppType* type)
{
    MarshalMetaInfo* meta = AllocMarshalMeta(type);
    meta->lua2csWriter = Lua2CSMarshalOpaque;
    meta->cs2luaWriter = CS2LuaMarshalOpaque;
    meta->size = sizeof(uintptr_t);
    meta->passByValue = true;
    return meta;
}

/// Shared Default (no Members / no non-Default LuaMarshalAs) metas keyed by Il2CppType*.
/// Hot path keeps const MarshalMetaInfo*; no runtime lookup.
static std::unordered_map<const Il2CppType*, MarshalMetaInfo*> s_defaultMarshalMetaByType;

static MarshalMetaInfo* GetOrCreateDefaultMarshalMeta(const Il2CppType* type)
{
    auto it = s_defaultMarshalMetaByType.find(type);
    if (it != s_defaultMarshalMetaByType.end())
        return it->second;

    MarshalMetaInfo* meta;
    if (type->byref)
    {
        meta = CreateByRefOpaqueMeta(type);
    }
    else
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        il2cpp::vm::Class::Init(klass);
        meta = AllocMarshalMeta(type);
        ApplySizeAndPassByValueAndKlass(meta, type, klass);
        ApplyDefaultMarshalWriters(meta, type, klass);
    }

    s_defaultMarshalMetaByType.insert({type, meta});
    return meta;
}

static MarshalMetaInfo* CreateDefaultOnlyMemberMeta(const Il2CppType* type)
{
    return GetOrCreateDefaultMarshalMeta(type);
}

static bool IsPublicInstanceField(const FieldInfo* field)
{
    if (field == nullptr || !il2cpp::vm::Field::IsInstance(const_cast<FieldInfo*>(field)))
        return false;
    return (il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) == FIELD_ATTRIBUTE_PUBLIC;
}

static bool IsPublicInstanceProperty(const PropertyInfo* property, bool requireGetter, bool requireSetter)
{
    if (property == nullptr)
        return false;
    if (requireGetter)
    {
        if (property->get == nullptr || !MetadataUtil::IsPublicMethod(property->get) || MetadataUtil::IsStaticMethod(property->get))
            return false;
        if (property->get->parameters_count != 0)
            return false;
    }
    if (requireSetter)
    {
        if (property->set == nullptr || !MetadataUtil::IsPublicMethod(property->set) || MetadataUtil::IsStaticMethod(property->set))
            return false;
        if (property->set->parameters_count != 1)
            return false;
    }
    return true;
}

static Il2CppClass* GetCompositeMemberOwnerClass(Il2CppClass* declaredKlass, LuaMarshalType marshalType)
{
    if (declaredKlass->nullabletype)
    {
        if (marshalType != LuaMarshalType::Table)
            LuaException::ThrowFormat("zlua: Nullable is only allowed with LuaMarshalType.Table (%s.%s)", declaredKlass->namespaze, declaredKlass->name);
        return declaredKlass->element_class;
    }
    return declaredKlass;
}

static void ResolveCompositeMembers(
    Il2CppClass* memberOwner,
    LuaMarshalType marshalType,
    const std::vector<std::string>& names,
    bool requireWrite,
    bool requireRead,
    CompositeMember*& outMembers,
    uint16_t& outCount)
{
    if (names.empty())
        LuaException::ThrowFormat("zlua: LuaMarshalType.%s requires non-empty Members (%s.%s)",
                                  marshalType == LuaMarshalType::Table ? "Table" : "UnpackedValues", memberOwner->namespaze, memberOwner->name);

    CompositeMember* members = LuaMetadataAlloc::CallocArray<CompositeMember>(names.size());
    for (size_t i = 0; i < names.size(); ++i)
    {
        const std::string& raw = names[i];
        if (raw.empty())
            LuaException::ThrowFormat("zlua: empty Members entry on %s.%s", memberOwner->namespaze, memberOwner->name);

        bool optional = false;
        std::string clrName = raw;
        if (!raw.empty() && raw.back() == '?')
        {
            if (marshalType != LuaMarshalType::Table)
                LuaException::ThrowFormat("zlua: UnpackedValues does not support optional member suffix '?' (%s.%s)", memberOwner->namespaze, memberOwner->name);
            optional = true;
            clrName = raw.substr(0, raw.size() - 1);
            if (clrName.empty())
                LuaException::ThrowFormat("zlua: invalid optional Members entry on %s.%s", memberOwner->namespaze, memberOwner->name);
        }

        CompositeMember& entry = members[i];
        entry.optional = optional;

        FieldInfo* field = il2cpp::vm::Class::GetFieldFromName(memberOwner, clrName.c_str());
        const PropertyInfo* property = nullptr;
        if (field == nullptr)
            property = il2cpp::vm::Class::GetPropertyFromName(memberOwner, clrName.c_str());

        if (field != nullptr)
        {
            if (!IsPublicInstanceField(field))
                LuaException::ThrowFormat("zlua: composite member '%s' is not a public instance field (%s.%s)", clrName.c_str(), memberOwner->namespaze,
                                          memberOwner->name);
            entry.isField = true;
            entry.field = field;
            entry.fieldOffset = FieldBridge::ComputeInstanceFieldOffset(field);
            entry.clrName = field->name;
            entry.memberMeta = CreateDefaultOnlyMemberMeta(field->type);
        }
        else if (property != nullptr)
        {
            if (!IsPublicInstanceProperty(property, requireRead, requireWrite))
                LuaException::ThrowFormat("zlua: composite member '%s' is not a usable public instance property (%s.%s)", clrName.c_str(),
                                          memberOwner->namespaze, memberOwner->name);
            entry.isField = false;
            entry.property = property;
            entry.fieldOffset = 0;
            entry.clrName = property->name;
            const Il2CppType* propType = property->get != nullptr ? property->get->return_type : property->set->parameters[0];
            entry.memberMeta = CreateDefaultOnlyMemberMeta(propType);
        }
        else
        {
            LuaException::ThrowFormat("zlua: composite member '%s' not found on %s.%s", clrName.c_str(), memberOwner->namespaze, memberOwner->name);
        }
    }

    outMembers = members;
    outCount = static_cast<uint16_t>(names.size());
}

static bool TryApplyDeclaredMarshalWriters(MarshalMetaInfo* meta, const Il2CppType* type, Il2CppClass* klass, const LuaMarshalAsData& data,
                                           bool requireWrite, bool requireRead, bool allowUnpacked, bool fromTypeLevel)
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
            meta->marshalType = LuaMarshalType::UserData;
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
            meta->marshalType = LuaMarshalType::Bytes;
            meta->lua2csWriter = Lua2CSMarshalByteArrayAsBytes;
            meta->cs2luaWriter = CS2LuaMarshalByteArrayAsBytes;
            return true;
        }
        return false;
    }
    case LuaMarshalType::OpaqueValue:
    {
        meta->marshalType = LuaMarshalType::OpaqueValue;
        meta->lua2csWriter = Lua2CSMarshalOpaque;
        meta->cs2luaWriter = CS2LuaMarshalOpaque;
        return true;
    }
    case LuaMarshalType::UnpackedValues:
    {
        if (!allowUnpacked)
            LuaException::ThrowFormat("zlua: UnpackedValues is not supported on fields/properties (%s.%s)", klass->namespaze, klass->name);
        if (!IsCompositeTargetValid(LuaMarshalType::UnpackedValues, klass))
            LuaException::ThrowFormat("zlua: invalid LuaMarshalType.UnpackedValues for %s.%s", klass->namespaze, klass->name);

        Il2CppClass* owner = GetCompositeMemberOwnerClass(klass, LuaMarshalType::UnpackedValues);
        CompositeMember* members = nullptr;
        uint16_t memberCount = 0;
        ResolveCompositeMembers(owner, LuaMarshalType::UnpackedValues, data.fieldOrPropertyNames, requireWrite, requireRead, members, memberCount);
        meta->marshalType = LuaMarshalType::UnpackedValues;
        meta->stackSlots = memberCount;
        meta->memberCount = memberCount;
        meta->members = members;
        if (!TryAttachCompositeSpecializedWriters(meta, klass, LuaMarshalType::UnpackedValues, fromTypeLevel))
        {
            meta->lua2csWriter = CompositeMarshal::Lua2CSMarshalUnpacked;
            meta->cs2luaWriter = CompositeMarshal::CS2LuaMarshalUnpacked;
        }
        return true;
    }
    case LuaMarshalType::Table:
    {
        if (!IsCompositeTargetValid(LuaMarshalType::Table, klass))
            LuaException::ThrowFormat("zlua: invalid LuaMarshalType.Table for %s.%s", klass->namespaze, klass->name);

        Il2CppClass* owner = GetCompositeMemberOwnerClass(klass, LuaMarshalType::Table);
        CompositeMember* members = nullptr;
        uint16_t memberCount = 0;
        ResolveCompositeMembers(owner, LuaMarshalType::Table, data.fieldOrPropertyNames, requireWrite, requireRead, members, memberCount);
        meta->marshalType = LuaMarshalType::Table;
        meta->stackSlots = 1;
        meta->memberCount = memberCount;
        meta->members = members;
        if (!TryAttachCompositeSpecializedWriters(meta, klass, LuaMarshalType::Table, fromTypeLevel))
        {
            meta->lua2csWriter = CompositeMarshal::Lua2CSMarshalTable;
            meta->cs2luaWriter = CompositeMarshal::CS2LuaMarshalTable;
        }
        return true;
    }
    default:
        return false;
    }
}

static void ApplyResolvedOrDefaultWriters(
    MarshalMetaInfo* meta,
    const Il2CppType* type,
    Il2CppClass* klass,
    LuaMarshalAsResolveKind resolveKind,
    const LuaMarshalAsData& marshalAs,
    bool requireWrite,
    bool requireRead,
    bool allowUnpacked)
{
    const bool fromTypeLevel = resolveKind == LuaMarshalAsResolveKind::TypeLevel;
    if (resolveKind == LuaMarshalAsResolveKind::None
        || !TryApplyDeclaredMarshalWriters(meta, type, klass, marshalAs, requireWrite, requireRead, allowUnpacked, fromTypeLevel))
        ApplyDefaultMarshalWriters(meta, type, klass);
}

static MarshalMetaInfo* CreateForValueType(const Il2CppType* type, Il2CppClass** outKlass)
{
    if (type->byref)
    {
        *outKlass = nullptr;
        return nullptr; // caller uses GetOrCreateDefaultMarshalMeta for byref
    }

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
    il2cpp::vm::Class::Init(klass);

    MarshalMetaInfo* meta = AllocMarshalMeta(type);
    ApplySizeAndPassByValueAndKlass(meta, type, klass);
    *outKlass = klass;
    return meta;
}


MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const MethodInfo* method, int argIndex)
{
    (void)L;
    const Il2CppType* type = argIndex >= 0 ? method->parameters[argIndex] : method->return_type;
    if (type->byref)
        return GetOrCreateDefaultMarshalMeta(type);

    Il2CppClass* klass = nullptr;
    LuaMarshalAsData marshalAs;
    uint32_t token = MetadataUtil::GetParameterToken(method, argIndex);
    // Resolve klass early for marshal-as; CreateForValueType also inits klass.
    klass = il2cpp::vm::Class::FromIl2CppType(type);
    il2cpp::vm::Class::Init(klass);

    LuaMarshalAsResolveKind resolveKind = TryResolveLuaMarshalAsDataForMethodSlot(
        method->klass->image, token, method, argIndex, klass, marshalAs);
    if (resolveKind == LuaMarshalAsResolveKind::None)
        return GetOrCreateDefaultMarshalMeta(type);

    MarshalMetaInfo* meta = CreateForValueType(type, &klass);
    IL2CPP_ASSERT(meta != nullptr);
    const bool requireWrite = argIndex >= 0;
    const bool requireRead = argIndex < 0;
    ApplyResolvedOrDefaultWriters(meta, type, klass, resolveKind, marshalAs, requireWrite, requireRead, /*allowUnpacked*/ true);
    // Declared/XML slot: always exclusive. Never intern — Bytes/UserData/Opaque used to keep
    // marshalType==Default while changing writers; sharing would poison Default for that type.
    return meta;
}

MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const FieldInfo* field)
{
    (void)L;
    const Il2CppType* type = field->type;
    if (type->byref)
        return GetOrCreateDefaultMarshalMeta(type);

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
    il2cpp::vm::Class::Init(klass);

    LuaMarshalAsData marshalAs;
    LuaMarshalAsResolveKind resolveKind = TryResolveLuaMarshalAsDataForField(field, klass, marshalAs);
    if (resolveKind == LuaMarshalAsResolveKind::None)
        return GetOrCreateDefaultMarshalMeta(type);

    MarshalMetaInfo* meta = CreateForValueType(type, &klass);
    IL2CPP_ASSERT(meta != nullptr);
    ApplyResolvedOrDefaultWriters(meta, type, klass, resolveKind, marshalAs, /*requireWrite*/ true, /*requireRead*/ true, /*allowUnpacked*/ false);
    return meta;
}

MarshalMetaInfo* MarshalMeta::Create(lua_State* L, const PropertyInfo* property)
{
    (void)L;
    const Il2CppType* type = property->get != nullptr ? property->get->return_type : property->set->parameters[0];
    if (type->byref)
        return GetOrCreateDefaultMarshalMeta(type);

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
    il2cpp::vm::Class::Init(klass);

    LuaMarshalAsData marshalAs;
    LuaMarshalAsResolveKind resolveKind = TryResolveLuaMarshalAsDataForProperty(property, klass, marshalAs);
    if (resolveKind == LuaMarshalAsResolveKind::None)
        return GetOrCreateDefaultMarshalMeta(type);

    MarshalMetaInfo* meta = CreateForValueType(type, &klass);
    IL2CPP_ASSERT(meta != nullptr);
    ApplyResolvedOrDefaultWriters(meta, type, klass, resolveKind, marshalAs, /*requireWrite*/ true, /*requireRead*/ true, /*allowUnpacked*/ false);
    return meta;
}
} // namespace zlua
