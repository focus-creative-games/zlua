#include "StructMarshal.h"

#include "ValueMarshaling.h"
#include "../mt/FieldBridge.h"
#include "../mt/MetaTableCache.h"

#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Type.h"

#include <cstring>

namespace zlua
{
    size_t StructMarshal::GetByValUserDataSize(Il2CppClass* klass)
    {
        return sizeof(ByValUserDataHeader) + ValueMarshaling::GetValueTypeInstanceSize(klass);
    }

    bool StructMarshal::IsByValUserData(lua_State* L, int index)
    {
        if (!lua_isuserdata(L, index))
            return false;

        if (lua_getmetatable(L, index) == 0)
            return false;

        lua_getfield(L, -1, "__zlua_ud_kind");
        const bool isByVal = lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "byval") == 0;
        lua_pop(L, 2);
        return isByVal;
    }

    ByValUserDataHeader* StructMarshal::GetByValHeader(lua_State* L, int index)
    {
        if (!IsByValUserData(L, index))
            return nullptr;
        return static_cast<ByValUserDataHeader*>(lua_touserdata(L, index));
    }

    void StructMarshal::PushValue(lua_State* L, void* dataAddr, Il2CppClass* klass)
    {
        if (klass == nullptr)
            luaL_error(L, "zlua: invalid struct type");

        il2cpp::vm::Class::Init(klass);
        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(klass);
        ByValUserDataHeader* header = static_cast<ByValUserDataHeader*>(lua_newuserdatauv(L, sizeof(ByValUserDataHeader) + payloadSize, 0));
        header->klass = klass;
        if (payloadSize > 0 && dataAddr != nullptr)
            std::memcpy(header->Payload(), dataAddr, payloadSize);

        MetaTableCache::PushByValMetatable(L, klass);
        lua_setmetatable(L, -2);
    }

    bool StructMarshal::PopValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr)
    {
        if (klass == nullptr || outDataAddr == nullptr)
            return false;

        ByValUserDataHeader* header = GetByValHeader(L, index);
        if (header == nullptr || header->klass == nullptr)
            return false;

        il2cpp::vm::Class::Init(klass);
        il2cpp::vm::Class::Init(header->klass);
        if (!il2cpp::vm::Class::IsAssignableFrom(klass, header->klass))
            return false;

        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(klass);
        std::memcpy(outDataAddr, header->Payload(), payloadSize);
        return true;
    }

    static bool IsPublicInstanceField(const FieldInfo* field)
    {
        return field != nullptr
            && field->name != nullptr
            && (il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_STATIC) == 0
            && (il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) == FIELD_ATTRIBUTE_PUBLIC;
    }

    static bool TryReadTableField(lua_State* L, int tableIndex, const FieldInfo* field, void* structData)
    {
        if (field == nullptr || field->name == nullptr || structData == nullptr)
            return false;

        const int absIndex = lua_absindex(L, tableIndex);
        lua_pushstring(L, field->name);
        lua_rawget(L, absIndex);
        const int fieldValueIndex = lua_absindex(L, -1);
        if (lua_isnil(L, fieldValueIndex))
        {
            lua_pop(L, 1);
            return false;
        }

        const int32_t offset = FieldBridge::ComputeInstanceFieldOffset(field);
        Il2CppClass* fieldClass = il2cpp::vm::Class::FromIl2CppType(field->type, false);
        size_t fieldSize = sizeof(void*);
        if (fieldClass != nullptr && fieldClass->byval_arg.valuetype)
            fieldSize = ValueMarshaling::GetValueTypeInstanceSize(fieldClass);
        else if (il2cpp::vm::Type::IsReference(field->type))
            fieldSize = sizeof(Il2CppObject*);
        else if (field->type->type == IL2CPP_TYPE_R8)
            fieldSize = sizeof(double);
        else if (field->type->type == IL2CPP_TYPE_R4)
            fieldSize = sizeof(float);
        else
            fieldSize = sizeof(int64_t);

        const bool ok = ValueMarshaling::TryPop(
            L,
            fieldValueIndex,
            field->type,
            static_cast<uint8_t*>(structData) + offset,
            fieldSize);
        lua_pop(L, 1);
        return ok;
    }

    static bool ValidateStructTable(lua_State* L, int index, Il2CppClass* klass)
    {
        if (!lua_istable(L, index) || klass == nullptr)
            return false;

        il2cpp::vm::Class::Init(klass);
        if (!klass->byval_arg.valuetype || klass->enumtype)
            return false;

        const int absIndex = lua_absindex(L, index);
        void* iter = nullptr;
        const FieldInfo* field = nullptr;
        while ((field = il2cpp::vm::Class::GetFields(klass, &iter)) != nullptr)
        {
            if (!IsPublicInstanceField(field))
                continue;

            lua_pushstring(L, field->name);
            lua_rawget(L, absIndex);
            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                return false;
            }

            if (!ValueMarshaling::CanConvert(L, -1, field->type))
            {
                lua_pop(L, 1);
                return false;
            }
            lua_pop(L, 1);
        }
        return true;
    }

    bool StructMarshal::CanPopFromTable(lua_State* L, int index, Il2CppClass* klass)
    {
        return ValidateStructTable(L, index, klass);
    }

    bool StructMarshal::TryPopFromTable(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr)
    {
        if (outDataAddr == nullptr || !ValidateStructTable(L, index, klass))
            return false;

        il2cpp::vm::Class::Init(klass);
        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(klass);
        std::memset(outDataAddr, 0, payloadSize);

        void* iter = nullptr;
        const FieldInfo* field = nullptr;
        while ((field = il2cpp::vm::Class::GetFields(klass, &iter)) != nullptr)
        {
            if (!IsPublicInstanceField(field))
                continue;
            if (!TryReadTableField(L, index, field, outDataAddr))
                return false;
        }
        return true;
    }
}
