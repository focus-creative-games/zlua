#include "FieldBridge.h"

#include "Marshaling.h"
#include "ObjectRegistry.h"

#include "gc/GarbageCollector.h"
#include "il2cpp-blob.h"
#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Type.h"

namespace novalua
{
    static void WriteBarrierForType(const Il2CppType* type, void** targetAddress)
    {
#if IL2CPP_ENABLE_WRITE_BARRIERS
#if IL2CPP_ENABLE_STRICT_WRITE_BARRIERS
        if (il2cpp::vm::Type::IsPointerType(type))
            return;

        if (il2cpp::vm::Type::IsStruct(type))
        {
            Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
            void* iter = NULL;
            FieldInfo* nestedField;
            while ((nestedField = il2cpp::vm::Class::GetFields(klass, &iter)) != NULL)
            {
                if (il2cpp::vm::Field::GetFlags(nestedField) & FIELD_ATTRIBUTE_STATIC)
                    continue;

                void* nestedAddress = il2cpp::vm::Field::GetInstanceFieldDataPointer((void*)targetAddress, nestedField);
                WriteBarrierForType(nestedField->type, (void**)nestedAddress);
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

    static bool IsPublicField(FieldInfo* field)
    {
        return (il2cpp::vm::Field::GetFlags(field) & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) == FIELD_ATTRIBUTE_PUBLIC;
    }

    FieldInfo* FieldBridge::FindPublicField(Il2CppClass* klass, const char* name, bool requireStatic)
    {
        if (klass == nullptr || name == nullptr || name[0] == '\0')
            return nullptr;

        il2cpp::vm::Class::Init(klass);
        FieldInfo* field = il2cpp::vm::Class::GetFieldFromName(klass, name);
        if (field == nullptr || !IsPublicField(field))
            return nullptr;

        const bool isStatic = !il2cpp::vm::Field::IsInstance(field);
        if (requireStatic != isStatic)
            return nullptr;

        if (isStatic && !il2cpp::vm::Field::IsNormalStatic(field))
            return nullptr;

        return field;
    }

    void* FieldBridge::GetFieldAddress(FieldInfo* field, Il2CppObject* instance)
    {
        if (il2cpp::vm::Field::IsNormalStatic(field))
        {
            il2cpp::vm::Class::Init(field->parent);
            return (uint8_t*)field->parent->static_fields + field->offset;
        }

        return il2cpp::vm::Field::GetInstanceFieldDataPointer(instance, field);
    }

    int FieldBridge::PushField(lua_State* L, FieldInfo* field, void* fieldPtr)
    {
        const Il2CppType* type = field->type;
        switch (type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            lua_pushboolean(L, *reinterpret_cast<uint8_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_I1:
            lua_pushinteger(L, *reinterpret_cast<int8_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_U1:
            lua_pushinteger(L, *reinterpret_cast<uint8_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_I2:
            lua_pushinteger(L, *reinterpret_cast<int16_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
            lua_pushinteger(L, *reinterpret_cast<uint16_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_I4:
            lua_pushinteger(L, *reinterpret_cast<int32_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_U4:
            lua_pushinteger(L, (lua_Integer)*reinterpret_cast<uint32_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_I8:
            lua_pushinteger(L, (lua_Integer)*reinterpret_cast<int64_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_U8:
            lua_pushinteger(L, (lua_Integer)*reinterpret_cast<uint64_t*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_R4:
            lua_pushnumber(L, *reinterpret_cast<float*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_R8:
            lua_pushnumber(L, *reinterpret_cast<double*>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_STRING:
            Marshaling::PushCString(L, *reinterpret_cast<Il2CppString**>(fieldPtr));
            return 1;
        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_OBJECT:
            ObjectRegistry::PushObject(L, *reinterpret_cast<Il2CppObject**>(fieldPtr));
            return 1;
        default:
            return luaL_error(L, "novalua: unsupported field read type: %d", (int)type->type);
        }
    }

    int FieldBridge::SetField(lua_State* L, FieldInfo* field, void* fieldPtr, int valueIndex)
    {
        if ((il2cpp::vm::Field::GetFlags(field) & FIELD_ATTRIBUTE_LITERAL) != 0)
            return luaL_error(L, "novalua: cannot assign to const field");

        const Il2CppType* type = field->type;
        switch (type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            *reinterpret_cast<uint8_t*>(fieldPtr) = lua_toboolean(L, valueIndex) ? 1 : 0;
            return 0;
        case IL2CPP_TYPE_I1:
            *reinterpret_cast<int8_t*>(fieldPtr) = (int8_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_U1:
            *reinterpret_cast<uint8_t*>(fieldPtr) = (uint8_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_I2:
            *reinterpret_cast<int16_t*>(fieldPtr) = (int16_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_U2:
        case IL2CPP_TYPE_CHAR:
            *reinterpret_cast<uint16_t*>(fieldPtr) = (uint16_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_I4:
            *reinterpret_cast<int32_t*>(fieldPtr) = (int32_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_U4:
            *reinterpret_cast<uint32_t*>(fieldPtr) = (uint32_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_I8:
            *reinterpret_cast<int64_t*>(fieldPtr) = (int64_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_U8:
            *reinterpret_cast<uint64_t*>(fieldPtr) = (uint64_t)lua_tointeger(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_R4:
            *reinterpret_cast<float*>(fieldPtr) = (float)lua_tonumber(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_R8:
            *reinterpret_cast<double*>(fieldPtr) = lua_tonumber(L, valueIndex);
            return 0;
        case IL2CPP_TYPE_STRING:
        {
            Il2CppString* value = Marshaling::PopCString(L, valueIndex);
            *reinterpret_cast<Il2CppString**>(fieldPtr) = value;
            WriteBarrierForType(type, (void**)fieldPtr);
            return 0;
        }
        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_OBJECT:
        {
            Il2CppObject* value = ObjectRegistry::GetObject(L, valueIndex);
            *reinterpret_cast<Il2CppObject**>(fieldPtr) = value;
            WriteBarrierForType(type, (void**)fieldPtr);
            return 0;
        }
        default:
            return luaL_error(L, "novalua: unsupported field write type: %d", (int)type->type);
        }
    }
}
