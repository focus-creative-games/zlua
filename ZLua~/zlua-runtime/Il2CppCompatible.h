#pragma once

/*
 * Multi Unity / Tuanjie il2cpp API shim (spec 11-MULTI-VERSION §12).
 * Assumes generated/ZLuaConf.inc was included by ZLuaCommon.h first.
 */

#ifndef ZLUA_UNITY_VERSION
#error "ZLUA_UNITY_VERSION must be defined by generated/ZLuaConf.inc"
#endif
#ifndef ZLUA_TUANJIE_ENGINE
#error "ZLUA_TUANJIE_ENGINE must be defined by generated/ZLuaConf.inc"
#endif

#include "il2cpp-config.h"
#include "il2cpp-api-types.h"
#include "il2cpp-object-internals.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-tabledefs.h"
#include "gc/WriteBarrier.h"
#include "utils/Memory.h"
#include "vm/Array.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/MetadataCache.h"
#include "vm/Reflection.h"
#if ZLUA_UNITY_VERSION >= 20220000
#include "metadata/GenericMethod.h"
#endif

// Unity ≤2022 ships utf8-cpp under libil2cpp/utils; Unity 6+ only under External/utfcpp.
#if ZLUA_UNITY_VERSION < 60000000
#include "utils/utf8-cpp/source/utf8/unchecked.h"
#else
#include "External/utfcpp/source/utf8.h"
#endif

#if IL2CPP_SIZEOF_VOID_P == 8
#define ZLUA_ARCH_64 1
#else
#define ZLUA_ARCH_32 1
#endif

// il2cpp 2022.3.33+ and 2021.3.40+ support method return type custom attribute
#if ZLUA_UNITY_VERSION >= 20220333 || (ZLUA_UNITY_VERSION / 10000 == 2021 && ZLUA_UNITY_VERSION >= 20210340)
#define ZLUA_SUPPORT_METHOD_RETURN_TYPE_CUSTOM_ATTRIBUTE 1
#else
#define ZLUA_SUPPORT_METHOD_RETURN_TYPE_CUSTOM_ATTRIBUTE 0
#endif

// Stock Unity: Memory::Calloc(count, size). Tuanjie: Calloc(count, size, Il2CppMemLabel).
inline void* ZLuaIl2CppCalloc(size_t count, size_t size)
{
#if ZLUA_TUANJIE_ENGINE
    return il2cpp::utils::Memory::Calloc(count, size, IL2CPP_MEM_STRING);
#else
    return il2cpp::utils::Memory::Calloc(count, size);
#endif
}

// Unlabeled Free is present on both Unity and Tuanjie.
inline void ZLuaIl2CppFree(void* memory)
{
    il2cpp::utils::Memory::Free(memory);
}

namespace zlua
{

// Unity 2021: MetadataCache::HasAttribute(handle). Unity 2022+: Reflection::HasAttribute(handle).
inline bool CustomAttributeHandleHasAttribute(
    Il2CppMetadataCustomAttributeHandle handle, Il2CppClass* attributeClass)
{
#if ZLUA_UNITY_VERSION < 20220000
    return il2cpp::vm::MetadataCache::HasAttribute(handle, attributeClass);
#else
    return il2cpp::vm::Reflection::HasAttribute(handle, attributeClass);
#endif
}

// Array::IndexFromIndices exists from ~Unity 2022.3.24; earlier 2022.3.x / 2021 use the same formula.
inline il2cpp_array_size_t ArrayIndexFromIndices(Il2CppArray* array, const int32_t* indices)
{
#if ZLUA_UNITY_VERSION < 20220324
    Il2CppClass* ac = array->klass;
    il2cpp_array_size_t pos = (il2cpp_array_size_t)(indices[0] - array->bounds[0].lower_bound);
    for (int32_t i = 1; i < ac->rank; i++)
    {
        pos = pos * array->bounds[i].length + (il2cpp_array_size_t)(indices[i] - array->bounds[i].lower_bound);
    }
    return pos;
#else
    return il2cpp::vm::Array::IndexFromIndices(array, indices);
#endif
}

// Class::IsGenericTypeDefinition is missing on early Unity 2021; equivalent to IsGeneric && !IsInflated.
inline bool ZLuaIsGenericTypeDefinition(const Il2CppClass* klass)
{
    return il2cpp::vm::Class::IsGeneric(klass) && !il2cpp::vm::Class::IsInflated(klass);
}

// GenericMethod::IsAnUnresolvedCallStubWasNotFound exists on Unity 2022+; 2021 has no such stubs.
inline bool ZLuaIsAnUnresolvedCallStubWasNotFound(Il2CppMethodPointer methodPointer)
{
#if ZLUA_UNITY_VERSION < 20220000
    (void)methodPointer;
    return false;
#else
    return il2cpp::metadata::GenericMethod::IsAnUnresolvedCallStubWasNotFound(methodPointer);
#endif
}

// Unity 6000.5.0+: Class::il2cpp_type_from_type_info gains an image parameter; pass nullptr for global search.
inline const Il2CppType* ZLuaIl2CppTypeFromTypeInfo(
    const il2cpp::vm::TypeNameParseInfo& info, il2cpp::vm::TypeSearchFlags searchFlags)
{
#if ZLUA_UNITY_VERSION >= 60000500
    return il2cpp::vm::Class::il2cpp_type_from_type_info(info, searchFlags, nullptr);
#else
    return il2cpp::vm::Class::il2cpp_type_from_type_info(info, searchFlags);
#endif
}

// Unity 6000.5.0+: Object::Unbox renamed to GetRawData (same: payload after Il2CppObject header).
inline void* ZLuaObjectUnbox(Il2CppObject* obj)
{
#if ZLUA_UNITY_VERSION >= 60000500
    return il2cpp::vm::Object::GetRawData(obj);
#else
    return il2cpp::vm::Object::Unbox(obj);
#endif
}

} // namespace zlua
