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

#include "OpaqueValueMarshal.h"
#include "TypedMarshal.h"

#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"

#include "vm/Class.h"

namespace zlua
{
#if ZLUA_ARCH_64
typedef uint64_t OpaqueParameterHandleType;
typedef uint32_t HandleGenerationType;
typedef uint32_t HandleIndexType;
#else
typedef uint32_t OpaqueParameterHandleType;
typedef uint16_t HandleGenerationType;
typedef uint16_t HandleIndexType;
#endif
constexpr size_t HandleGenerationShift = sizeof(HandleGenerationType) * 8;

struct OpaqueParameterData
{
    HandleGenerationType generation;
    const Il2CppType* type;
    void* valueAddress;
};

static std::vector<OpaqueParameterData> s_opaqueParameterDataStack;
static HandleGenerationType s_handleGeneration = 0;

static OpaqueParameterHandleType ComposeHandle(HandleGenerationType generation, HandleIndexType index)
{
    return ((OpaqueParameterHandleType)generation << HandleGenerationShift) | (OpaqueParameterHandleType)index;
}

static void ExtractHandle(OpaqueParameterHandleType handle, HandleGenerationType& generation, HandleIndexType& index)
{
    generation = (HandleGenerationType)(handle >> HandleGenerationShift);
    index = (HandleIndexType)handle;
}

static OpaqueParameterHandleType AllocateOpaqueParameterHandle(const Il2CppType* type, void* valueAddress)
{
    IL2CPP_ASSERT(s_opaqueParameterDataStack.size() < ((size_t)1 << HandleGenerationShift));
    HandleIndexType index = (HandleIndexType)s_opaqueParameterDataStack.size();
    s_opaqueParameterDataStack.emplace_back(OpaqueParameterData{s_handleGeneration, type, valueAddress});
    return ComposeHandle(s_handleGeneration, index);
}

OpaqueParameterScope::OpaqueParameterScope()
{
    ++s_handleGeneration;
    _oldStackSize = s_opaqueParameterDataStack.size();
}

OpaqueParameterScope::~OpaqueParameterScope()
{
    s_opaqueParameterDataStack.resize(_oldStackSize);
}

void OpaqueValueMarshal::Push(lua_State* L, void* valueAddress, const Il2CppType* type)
{
    OpaqueParameterHandleType handle = AllocateOpaqueParameterHandle(type, valueAddress);
    lua_pushlightuserdata(L, (void*)handle);
}

bool IsIl2CppTypeCompatible(const Il2CppType* fromType, const Il2CppType* toType)
{
    if (fromType->byref != toType->byref)
    {
        return false;
    }
    switch (toType->type)
    {
    case IL2CPP_TYPE_VOID:
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
    case IL2CPP_TYPE_STRING:
    case IL2CPP_TYPE_TYPEDBYREF:
    {
        return fromType->type == toType->type;
    }
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_SZARRAY:
    {
        return fromType->type == toType->type && IsIl2CppTypeCompatible(fromType->data.type, toType->data.type);
    }

    case IL2CPP_TYPE_OBJECT:
    {
        if (fromType->type == IL2CPP_TYPE_OBJECT)
        {
            return true;
        }
        // slow path
        break;
    }
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_VALUETYPE:
    {
        if (fromType->type == toType->type && fromType->data.typeHandle == toType->data.typeHandle)
        {
            return true;
        }
        // slow path
        break;
    }
    case IL2CPP_TYPE_GENERICINST:
    {
        if (fromType->type == IL2CPP_TYPE_GENERICINST && fromType->data.generic_class == toType->data.generic_class)
        {
            return true;
        }
        // slow path
        break;
    }
    default:
        return false;
    }
    Il2CppClass* fromClass = il2cpp::vm::Class::FromIl2CppType(fromType);
    Il2CppClass* toClass = il2cpp::vm::Class::FromIl2CppType(toType);
    return fromClass == toClass || il2cpp::vm::Class::IsAssignableFrom(toClass, fromClass);
}

static OpaqueParameterData& GetOpaqueParameterData(lua_State* L, int index)
{
    if (lua_islightuserdata(L, index))
    {
        OpaqueParameterHandleType handle = (OpaqueParameterHandleType)lua_touserdata(L, index);
        HandleGenerationType generation;
        HandleIndexType index;
        ExtractHandle(handle, generation, index);
        if (index < s_opaqueParameterDataStack.size())
        {
            OpaqueParameterData& data = s_opaqueParameterDataStack[index];
            if (data.generation == generation)
            {
                return data;
            }
        }
    }
    LuaException::Throw("zlua argument mismatch: invalid opaque value handle");
}

void OpaqueValueMarshal::Pop(lua_State* L, int index, void* valueAddress, const Il2CppType* type)
{
    OpaqueParameterData& data = GetOpaqueParameterData(L, index);
    if (!IsIl2CppTypeCompatible(data.type, type))
    {
        LuaException::Throw("zlua argument mismatch: opaque value handle type mismatch");
    }
    int32_t size = MetadataUtil::GetValueSize(type);
    memcpy(valueAddress, data.valueAddress, size);
}

void OpaqueValueMarshal::PushTypedReference(lua_State* L, void* valueAddress)
{
    LuaException::Throw("zlua: can't marshal typed reference type");
}

void OpaqueValueMarshal::PopTypedReference(lua_State* L, int index, void* valueAddress)
{
    LuaException::Throw("zlua: can't marshal typed reference type");
}

void OpaqueValueMarshal::PushValueFromHandle(lua_State* L, int handleIndex)
{
    OpaqueParameterData& data = GetOpaqueParameterData(L, handleIndex);
    if (data.type->byref)
    {
        Il2CppType derefType = *data.type;
        derefType.byref = false;
        void* derefValueAddress = *((void**)data.valueAddress);
        TypedMarshal::PushByType(L, derefValueAddress, &derefType);
    }
    else
    {
        TypedMarshal::PushByType(L, data.valueAddress, data.type);
    }
}

void OpaqueValueMarshal::SetValueToHandle(lua_State* L, int handleIndex, int valueIndex)
{
    OpaqueParameterData& data = GetOpaqueParameterData(L, handleIndex);
    if (data.type->byref)
    {
        Il2CppType derefType = *data.type;
        derefType.byref = false;
        void* derefValueAddress = *((void**)data.valueAddress);
        TypedMarshal::PopByType(L, valueIndex, derefValueAddress, &derefType);
    }
    else
    {
        TypedMarshal::PopByType(L, valueIndex, data.valueAddress, data.type);
    }
}
} // namespace zlua