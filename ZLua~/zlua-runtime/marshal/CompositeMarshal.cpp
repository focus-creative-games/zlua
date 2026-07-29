#include "CompositeMarshal.h"

#include "../bridge/FieldBridge.h"
#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"

#include "gc/GarbageCollector.h"
#include "vm/Class.h"
#include "vm/Object.h"

#include <cstring>

namespace zlua
{
namespace
{

size_t StructPayloadSize(Il2CppClass* klass)
{
    return MetadataUtil::GetInstanceSizeWithoutHeader(klass);
}

void ZeroInitStruct(void* address, Il2CppClass* klass)
{
    std::memset(address, 0, StructPayloadSize(klass));
}

void* MemberAddress(void* structAddress, const CompositeMember& member)
{
    IL2CPP_ASSERT(member.isField);
    return reinterpret_cast<uint8_t*>(structAddress) + member.fieldOffset;
}

void WriteFieldMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    void* dest = MemberAddress(structAddress, member);
    const MarshalMetaInfo* memberMeta = member.memberMeta;
    if (memberMeta->passByValue)
    {
        *reinterpret_cast<void**>(dest) = *reinterpret_cast<void**>(valueStorage);
        il2cpp::gc::GarbageCollector::SetWriteBarrier(reinterpret_cast<void**>(dest));
    }
    else
    {
        std::memcpy(dest, valueStorage, static_cast<size_t>(memberMeta->size));
        if (!member.field->type->valuetype)
        {
            il2cpp::gc::GarbageCollector::SetWriteBarrier(reinterpret_cast<void**>(dest), static_cast<size_t>(memberMeta->size));
        }
        else if (!memberMeta->typeKlass->is_blittable)
        {
            il2cpp::gc::GarbageCollector::SetWriteBarrier(reinterpret_cast<void**>(dest), static_cast<size_t>(memberMeta->size));
        }
    }
}

void ReadFieldMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    void* src = MemberAddress(structAddress, member);
    const MarshalMetaInfo* memberMeta = member.memberMeta;
    if (memberMeta->passByValue)
    {
        *reinterpret_cast<void**>(valueStorage) = *reinterpret_cast<void**>(src);
    }
    else
    {
        std::memcpy(valueStorage, src, static_cast<size_t>(memberMeta->size));
    }
}

void WritePropertyMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    const PropertyInfo* property = member.property;
    IL2CPP_ASSERT(property->set != nullptr);
    const MethodInfo* setter = property->set;
    void* args[1];
    if (member.memberMeta->passByValue)
        args[0] = *reinterpret_cast<void**>(valueStorage);
    else
        args[0] = valueStorage;
    setter->invoker_method(setter->methodPointer, setter, structAddress, args, nullptr);
}

void ReadPropertyMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    const PropertyInfo* property = member.property;
    IL2CPP_ASSERT(property->get != nullptr);
    const MethodInfo* getter = property->get;
    getter->invoker_method(getter->methodPointer, getter, structAddress, nullptr, valueStorage);
}

void WriteMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    if (member.isField)
        WriteFieldMember(structAddress, member, valueStorage);
    else
        WritePropertyMember(structAddress, member, valueStorage);
}

void ReadMember(void* structAddress, const CompositeMember& member, void* valueStorage)
{
    if (member.isField)
        ReadFieldMember(structAddress, member, valueStorage);
    else
        ReadPropertyMember(structAddress, member, valueStorage);
}

void PopOneMember(lua_State* L, int valueIdx, void* structAddress, const CompositeMember& member)
{
    const MarshalMetaInfo* memberMeta = member.memberMeta;
    void* tempStorage = nullptr;
    void* storage = memberMeta->passByValue ? &tempStorage : alloca(static_cast<size_t>(memberMeta->size));
    memberMeta->lua2csWriter(L, valueIdx, storage, memberMeta);
    void* valuePtr = memberMeta->passByValue ? &tempStorage : storage;
    WriteMember(structAddress, member, valuePtr);
}

void PushOneMember(lua_State* L, void* structAddress, const CompositeMember& member)
{
    const MarshalMetaInfo* memberMeta = member.memberMeta;
    void* tempStorage = nullptr;
    void* storage = memberMeta->passByValue ? &tempStorage : alloca(static_cast<size_t>(memberMeta->size));
    ReadMember(structAddress, member, memberMeta->passByValue ? &tempStorage : storage);
    memberMeta->cs2luaWriter(L, memberMeta->passByValue ? &tempStorage : storage, memberMeta);
}

void PopTableIntoStruct(lua_State* L, int valueIdx, void* address, Il2CppClass* structKlass, const MarshalMetaInfo* meta)
{
    if (!lua_istable(L, valueIdx))
    {
        LuaException::ThrowFormat(
            "zlua: expected table for LuaMarshalType.Table (%s.%s), got %s",
            structKlass->namespaze,
            structKlass->name,
            lua_typename(L, lua_type(L, valueIdx)));
    }

    ZeroInitStruct(address, structKlass);
    for (uint16_t i = 0; i < meta->memberCount; ++i)
    {
        const CompositeMember& member = meta->members[i];
        const int fieldType = lua_getfield(L, valueIdx, member.clrName);
        if (fieldType == LUA_TNIL)
        {
            lua_pop(L, 1);
            if (member.optional)
                continue;
            LuaException::ThrowFormat(
                "zlua: missing table key '%s' for %s.%s",
                member.clrName,
                structKlass->namespaze,
                structKlass->name);
        }

        PopOneMember(L, -1, address, member);
        lua_pop(L, 1);
    }
}

void PushTableFromStruct(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    lua_createtable(L, 0, meta->memberCount);
    const int tableIdx = lua_gettop(L);
    for (uint16_t i = 0; i < meta->memberCount; ++i)
    {
        const CompositeMember& member = meta->members[i];
        PushOneMember(L, address, member);
        lua_setfield(L, tableIdx, member.clrName);
    }
}

} // namespace

void CompositeMarshal::PopTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    if (klass->nullabletype)
    {
        if (lua_isnil(L, valueIdx))
        {
            MetadataUtil::InitNullableValue(address, klass);
            return;
        }

        void* valueAddr = MetadataUtil::GetNullableValue(address, klass);
        PopTableIntoStruct(L, valueIdx, valueAddr, klass->element_class, meta);
        MetadataUtil::NullableSetHasValue(address, klass);
        return;
    }

    PopTableIntoStruct(L, valueIdx, address, klass, meta);
}

void CompositeMarshal::PushTable(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    if (klass->nullabletype)
    {
        if (!il2cpp::vm::Object::NullableHasValue(klass, address))
        {
            lua_pushnil(L);
            return;
        }

        void* valueAddr = MetadataUtil::GetNullableValue(address, klass);
        PushTableFromStruct(L, valueAddr, meta);
        return;
    }

    PushTableFromStruct(L, address, meta);
}

void CompositeMarshal::PopUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    IL2CPP_ASSERT(!klass->nullabletype);
    ZeroInitStruct(address, klass);
    for (uint16_t i = 0; i < meta->memberCount; ++i)
    {
        PopOneMember(L, valueIdx + static_cast<int>(i), address, meta->members[i]);
    }
}

void CompositeMarshal::PushUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    Il2CppClass* klass = meta->typeKlass;
    IL2CPP_ASSERT(!klass->nullabletype);
    for (uint16_t i = 0; i < meta->memberCount; ++i)
    {
        PushOneMember(L, address, meta->members[i]);
    }
}

void CompositeMarshal::Lua2CSMarshalTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)
{
    PopTable(L, valueIdx, address, meta);
}

void CompositeMarshal::CS2LuaMarshalTable(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    PushTable(L, address, meta);
}

void CompositeMarshal::Lua2CSMarshalUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)
{
    PopUnpacked(L, valueIdx, address, meta);
}

void CompositeMarshal::CS2LuaMarshalUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta)
{
    PushUnpacked(L, address, meta);
}

} // namespace zlua
