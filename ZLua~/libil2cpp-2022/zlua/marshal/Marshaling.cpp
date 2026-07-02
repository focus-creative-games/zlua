#include "Marshaling.h"

#include "StructMarshal.h"
#include "PrimitiveMarshal.h"
#include "StringMarshal.h"
#include "ObjectMarshal.h"
#include "OpaqueValueMarshal.h"

#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"

#include "vm/Class.h"

namespace zlua
{

void Marshaling::PushByType(lua_State* L, void* ptr, const Il2CppType* type)
{
    if (type->byref)
    {
        OpaqueValueMarshal::Push(L, ptr, type);
        return;
    }
restart:
    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
        lua_pushnil(L);
        break;
    case IL2CPP_TYPE_BOOLEAN:
        PrimitiveMarshal::PushBool(L, *(bool*)ptr);
        break;
    case IL2CPP_TYPE_CHAR:
        PrimitiveMarshal::PushUInt16(L, *(uint16_t*)ptr);
        break;
    case IL2CPP_TYPE_I1:
        PrimitiveMarshal::PushInt8(L, *(int8_t*)ptr);
        break;
    case IL2CPP_TYPE_U1:
        PrimitiveMarshal::PushUInt8(L, *(uint8_t*)ptr);
        break;
    case IL2CPP_TYPE_I2:
        PrimitiveMarshal::PushInt16(L, *(int16_t*)ptr);
        break;
    case IL2CPP_TYPE_U2:
        PrimitiveMarshal::PushUInt16(L, *(uint16_t*)ptr);
        break;
    case IL2CPP_TYPE_I4:
        PrimitiveMarshal::PushInt32(L, *(int32_t*)ptr);
        break;
    case IL2CPP_TYPE_U4:
        PrimitiveMarshal::PushUInt32(L, *(uint32_t*)ptr);
        break;
    case IL2CPP_TYPE_I8:
        PrimitiveMarshal::PushInt64(L, *(int64_t*)ptr);
        break;
    case IL2CPP_TYPE_U8:
        PrimitiveMarshal::PushUInt64(L, *(uint64_t*)ptr);
        break;
    case IL2CPP_TYPE_R4:
        PrimitiveMarshal::PushFloat(L, *(float*)ptr);
        break;
    case IL2CPP_TYPE_R8:
        PrimitiveMarshal::PushDouble(L, *(double*)ptr);
        break;
    case IL2CPP_TYPE_I:
        PrimitiveMarshal::PushIntPtr(L, *(intptr_t*)ptr);
        break;
    case IL2CPP_TYPE_U:
        PrimitiveMarshal::PushUIntPtr(L, *(uintptr_t*)ptr);
        break;
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        PrimitiveMarshal::PushPointer(L, *(void**)ptr);
        break;
    case IL2CPP_TYPE_STRING:
        StringMarshal::Push(L, *(Il2CppString**)ptr);
        break;
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        ObjectMarshal::Push(L, *(Il2CppObject**)ptr);
        break;
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        if (klass->enumtype)
        {
            type = &klass->element_class->byval_arg;
            goto restart;
        }
        if (il2cpp::vm::Class::IsValuetype(klass))
        {
            StructMarshal::PushValue(L, ptr, klass);
        }
        else
        {
            ObjectMarshal::Push(L, *(Il2CppObject**)ptr);
        }
        break;
    }
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetNotSupportedException("can't marshal VAR or MVAR type"));
        break;
    default:
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentException("type", "Invalid type"));
        break;
    }
}

void Marshaling::PopByType(lua_State* L, int idx, void* ptr, const Il2CppType* type)
{
    if (type->byref)
    {
        *(void**)ptr = PrimitiveMarshal::PopPointer(L, idx);
        OpaqueValueMarshal::Pop(L, idx, ptr, type);
        return;
    }

restart:
    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
        break;
    case IL2CPP_TYPE_BOOLEAN:
        *(bool*)ptr = PrimitiveMarshal::PopBool(L, idx);
        break;
    case IL2CPP_TYPE_CHAR:
        *(uint16_t*)ptr = PrimitiveMarshal::PopUInt16(L, idx);
        break;
    case IL2CPP_TYPE_I1:
        *(int8_t*)ptr = PrimitiveMarshal::PopInt8(L, idx);
        break;
    case IL2CPP_TYPE_U1:
        *(uint8_t*)ptr = PrimitiveMarshal::PopUInt8(L, idx);
        break;
    case IL2CPP_TYPE_I2:
        *(int16_t*)ptr = PrimitiveMarshal::PopInt16(L, idx);
        break;
    case IL2CPP_TYPE_U2:
        *(uint16_t*)ptr = PrimitiveMarshal::PopUInt16(L, idx);
        break;
    case IL2CPP_TYPE_I4:
        *(int32_t*)ptr = PrimitiveMarshal::PopInt32(L, idx);
        break;
    case IL2CPP_TYPE_U4:
        *(uint32_t*)ptr = PrimitiveMarshal::PopUInt32(L, idx);
        break;
    case IL2CPP_TYPE_I8:
        *(int64_t*)ptr = PrimitiveMarshal::PopInt64(L, idx);
        break;
    case IL2CPP_TYPE_U8:
        *(uint64_t*)ptr = PrimitiveMarshal::PopUInt64(L, idx);
        break;
    case IL2CPP_TYPE_R4:
        *(float*)ptr = PrimitiveMarshal::PopFloat(L, idx);
        break;
    case IL2CPP_TYPE_R8:
        *(double*)ptr = PrimitiveMarshal::PopDouble(L, idx);
        break;
    case IL2CPP_TYPE_I:
        *(intptr_t*)ptr = PrimitiveMarshal::PopIntPtr(L, idx);
        break;
    case IL2CPP_TYPE_U:
        *(uintptr_t*)ptr = PrimitiveMarshal::PopUIntPtr(L, idx);
        break;
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        *(void**)ptr = PrimitiveMarshal::PopPointer(L, idx);
        break;
    case IL2CPP_TYPE_STRING:
        *(Il2CppString**)ptr = StringMarshal::Pop(L, idx);
        break;
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        *(Il2CppObject**)ptr = ObjectMarshal::PopNotDelegate(L, idx, il2cpp::vm::Class::FromIl2CppType(type));
        break;
    case IL2CPP_TYPE_CLASS:
        *(Il2CppObject**)ptr = ObjectMarshal::Pop(L, idx, il2cpp::vm::Class::FromIl2CppType(type));
        break;
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        if (il2cpp::vm::Class::IsValuetype(klass))
        {
            if (klass->enumtype)
            {
                type = &klass->element_class->byval_arg;
                goto restart;
            }
            if (klass->nullabletype)
            {
                StructMarshal::PopNullableValue(L, idx, klass, ptr);
                break;
            }
            StructMarshal::PopValue(L, idx, klass, ptr);
        }
        else
        {
            *(Il2CppObject**)ptr = ObjectMarshal::Pop(L, idx, klass);
        }
        break;
    }
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
        LuaException::Throw("zlua: can't marshal VAR or MVAR type");
        break;
    default:
        LuaException::ThrowFormat("zlua: invalid type: %d", type->type);
        break;
    }
}
} // namespace zlua
