#include "FieldBridge.h"

#include "Marshaling.h"
#include "ObjectRegistry.h"

#include "gc/GarbageCollector.h"
#include "il2cpp-blob.h"
#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Type.h"

namespace zlua
{
    static int GetBoolean(lua_State* L, void* fieldPtr)
    {
        lua_pushboolean(L, *reinterpret_cast<uint8_t*>(fieldPtr));
        return 1;
    }

    static int SetBoolean(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<uint8_t*>(fieldPtr) = lua_toboolean(L, valueIndex) ? 1 : 0;
        return 0;
    }

    static int GetInt8(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, *reinterpret_cast<int8_t*>(fieldPtr));
        return 1;
    }

    static int SetInt8(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<int8_t*>(fieldPtr) = (int8_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetUInt8(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, *reinterpret_cast<uint8_t*>(fieldPtr));
        return 1;
    }

    static int SetUInt8(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<uint8_t*>(fieldPtr) = (uint8_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetInt16(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, *reinterpret_cast<int16_t*>(fieldPtr));
        return 1;
    }

    static int SetInt16(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<int16_t*>(fieldPtr) = (int16_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetUInt16(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, *reinterpret_cast<uint16_t*>(fieldPtr));
        return 1;
    }

    static int SetUInt16(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<uint16_t*>(fieldPtr) = (uint16_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetInt32(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, *reinterpret_cast<int32_t*>(fieldPtr));
        return 1;
    }

    static int SetInt32(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        if ((il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_LITERAL) != 0)
            return luaL_error(L, "zlua: cannot assign to const field");
        *reinterpret_cast<int32_t*>(fieldPtr) = (int32_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetUInt32(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, (lua_Integer)*reinterpret_cast<uint32_t*>(fieldPtr));
        return 1;
    }

    static int SetUInt32(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<uint32_t*>(fieldPtr) = (uint32_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetInt64(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, (lua_Integer)*reinterpret_cast<int64_t*>(fieldPtr));
        return 1;
    }

    static int SetInt64(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<int64_t*>(fieldPtr) = (int64_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetUInt64(lua_State* L, void* fieldPtr)
    {
        lua_pushinteger(L, (lua_Integer)*reinterpret_cast<uint64_t*>(fieldPtr));
        return 1;
    }

    static int SetUInt64(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<uint64_t*>(fieldPtr) = (uint64_t)lua_tointeger(L, valueIndex);
        return 0;
    }

    static int GetFloat(lua_State* L, void* fieldPtr)
    {
        lua_pushnumber(L, *reinterpret_cast<float*>(fieldPtr));
        return 1;
    }

    static int SetFloat(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<float*>(fieldPtr) = (float)lua_tonumber(L, valueIndex);
        return 0;
    }

    static int GetDouble(lua_State* L, void* fieldPtr)
    {
        lua_pushnumber(L, *reinterpret_cast<double*>(fieldPtr));
        return 1;
    }

    static int SetDouble(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        (void)field;
        *reinterpret_cast<double*>(fieldPtr) = lua_tonumber(L, valueIndex);
        return 0;
    }

    static int GetString(lua_State* L, void* fieldPtr)
    {
        Marshaling::PushCString(L, *reinterpret_cast<Il2CppString**>(fieldPtr));
        return 1;
    }

    static int SetString(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        Il2CppString* value = Marshaling::PopCString(L, valueIndex);
        *reinterpret_cast<Il2CppString**>(fieldPtr) = value;
        FieldBridge::WriteBarrierForFieldType(field->type, (void**)fieldPtr);
        return 0;
    }

    static int GetObject(lua_State* L, void* fieldPtr)
    {
        ObjectRegistry::PushObject(L, *reinterpret_cast<Il2CppObject**>(fieldPtr));
        return 1;
    }

    static int SetObject(lua_State* L, const FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        Il2CppObject* value = ObjectRegistry::GetObject(L, valueIndex);
        *reinterpret_cast<Il2CppObject**>(fieldPtr) = value;
        FieldBridge::WriteBarrierForFieldType(field->type, (void**)fieldPtr);
        return 0;
    }

    void FieldBridge::WriteBarrierForFieldType(const Il2CppType* type, void** targetAddress)
    {
#if IL2CPP_ENABLE_WRITE_BARRIERS
#if IL2CPP_ENABLE_STRICT_WRITE_BARRIERS
        if (type != nullptr && il2cpp::vm::Type::IsPointerType(type))
            return;

        if (type != nullptr && il2cpp::vm::Type::IsStruct(type))
        {
            Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
            void* iter = NULL;
            FieldInfo* nestedField;
            while ((nestedField = il2cpp::vm::Class::GetFields(klass, &iter)) != NULL)
            {
                if (il2cpp::vm::Field::GetFlags(nestedField) & FIELD_ATTRIBUTE_STATIC)
                    continue;

                void* nestedAddress = il2cpp::vm::Field::GetInstanceFieldDataPointer((void*)targetAddress, nestedField);
                WriteBarrierForFieldType(nestedField->type, (void**)nestedAddress);
            }
        }
        else
        {
            il2cpp::gc::GarbageCollector::SetWriteBarrier(targetAddress);
        }
#else
        il2cpp::gc::GarbageCollector::SetWriteBarrier(targetAddress);
#endif
#else
        (void)type;
        (void)targetAddress;
#endif
    }

    bool FieldBridge::IsPublicField(const FieldInfo* field)
    {
        return (il2cpp::vm::Field::GetFlags(const_cast<FieldInfo*>(field)) & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) == FIELD_ATTRIBUTE_PUBLIC;
    }

    void* FieldBridge::ComputeStaticFieldAddress(const FieldInfo* field)
    {
        il2cpp::vm::Class::Init(field->parent);
        return (uint8_t*)field->parent->static_fields + field->offset;
    }

    int32_t FieldBridge::ComputeInstanceFieldOffset(const FieldInfo* field)
    {
        int32_t offset = (int32_t)il2cpp::vm::Field::GetOffset(const_cast<FieldInfo*>(field));
        if (field->parent->byval_arg.valuetype)
            offset -= (int32_t)sizeof(Il2CppObject);
        return offset;
    }

    FieldAccessor FieldBridge::ResolveFieldAccessor(const FieldInfo* field)
    {
        FieldAccessor accessor = { nullptr, nullptr };
        const Il2CppType* type = field->type;
        switch (type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            accessor.getter = GetBoolean;
            accessor.setter = SetBoolean;
            break;
        case IL2CPP_TYPE_I1:
            accessor.getter = GetInt8;
            accessor.setter = SetInt8;
            break;
        case IL2CPP_TYPE_U1:
            accessor.getter = GetUInt8;
            accessor.setter = SetUInt8;
            break;
        case IL2CPP_TYPE_I2:
            accessor.getter = GetInt16;
            accessor.setter = SetInt16;
            break;
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
            accessor.getter = GetUInt16;
            accessor.setter = SetUInt16;
            break;
        case IL2CPP_TYPE_I4:
            accessor.getter = GetInt32;
            accessor.setter = SetInt32;
            break;
        case IL2CPP_TYPE_U4:
            accessor.getter = GetUInt32;
            accessor.setter = SetUInt32;
            break;
        case IL2CPP_TYPE_I8:
            accessor.getter = GetInt64;
            accessor.setter = SetInt64;
            break;
        case IL2CPP_TYPE_U8:
            accessor.getter = GetUInt64;
            accessor.setter = SetUInt64;
            break;
        case IL2CPP_TYPE_R4:
            accessor.getter = GetFloat;
            accessor.setter = SetFloat;
            break;
        case IL2CPP_TYPE_R8:
            accessor.getter = GetDouble;
            accessor.setter = SetDouble;
            break;
        case IL2CPP_TYPE_STRING:
            accessor.getter = GetString;
            accessor.setter = SetString;
            break;
        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_OBJECT:
            accessor.getter = GetObject;
            accessor.setter = SetObject;
            break;
        default:
            break;
        }
        return accessor;
    }
}
