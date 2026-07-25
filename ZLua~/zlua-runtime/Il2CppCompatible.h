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
#include "vm/MetadataCache.h"
#include "vm/Reflection.h"

// Unity ≤2022 ships utf8-cpp under libil2cpp/utils; Unity 6+ only under External/utfcpp.
#if ZLUA_UNITY_VERSION < 60000000
#include "utils/utf8-cpp/source/utf8/unchecked.h"
#else
#include "External/utfcpp/source/utf8.h"
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

} // namespace zlua
