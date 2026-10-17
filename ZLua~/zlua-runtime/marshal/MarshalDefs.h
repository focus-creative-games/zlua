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

#pragma once

#include <climits>

#include "../ZLuaCommon.h"

namespace zlua
{
enum class LuaMarshalType : uint8_t
{
    Default = 0,
    UserData,
    Bytes,
    OpaqueValue,
    UnpackedValues,
    Table,
    /// Return-only: ByObj UserData with viewKlass = runtime actual type (not declared type).
    ActualTypeUserData,
};

enum class MetaTableKind : uint8_t
{
    ReferenceByObj,
    StructByObj,
    StructByVal,
    Static,
};

struct MarshalMetaInfo;
/// needsWriteBarrierOnWriteReference: destination is a GC-tracked slot (not stack/alloca).
/// Distinct from MarshalMetaInfo::needsWriteBarrier (type itself embeds managed refs).
typedef void (*FnMarshalLua2Cs)(lua_State* L, int valueIdx, void* address, bool needsWriteBarrierOnWriteReference, const MarshalMetaInfo* ctx);
typedef void (*FnMarshalCs2Lua)(lua_State* L, void* address, const MarshalMetaInfo* ctx);

/// Pre-resolved Table / UnpackedValues member (no name lookup on hot path).
struct CompositeMember
{
    const char* clrName;
    union
    {
        const FieldInfo* field;
        const PropertyInfo* property;
    };
    const MarshalMetaInfo* memberMeta;
    /// Offset into valuetype payload (excludes Il2CppObject header). Valid when isField.
    int32_t fieldOffset;
    bool optional;
    bool isField;
};

struct MarshalMetaInfo
{
    FnMarshalLua2Cs lua2csWriter;
    FnMarshalCs2Lua cs2luaWriter;
    const Il2CppType* type;
    Il2CppClass* typeKlass; // declared type for marshal facade (class/interface/object/array/struct/...)
    const CompositeMember* members;
    int32_t size;
    int luaByValRefIndex;
    int luaByObjRefIndex;
    /// Matches MarshalMeta metatable-ref epoch when luaBy*RefIndex are valid for the current lua_State.
    int luaMetatableRefEpoch;
    uint16_t stackSlots; // Default/Table/... = 1; UnpackedValues = memberCount
    uint16_t memberCount;
    bool passByValue;
    /// Precomputed: writing this type into GC-tracked memory needs a write barrier.
    /// Reference types always; valuetypes only when they embed references.
    /// Callers still pass false when the destination is stack / alloca.
    bool needsWriteBarrier;
    LuaMarshalType marshalType;
};

struct FieldMarshalCtx
{
    // MetaTableKind kind;
    const MarshalMetaInfo* meta;

    union
    {
        void* staticAddress;
        int32_t instanceOffsetIncludingHeader;
        // int32_t instanceOffsetExcludingHeader;
    };
};


struct PropertyMarshalCtx;

typedef void (*FnPropertyGetter)(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx);
typedef void (*FnPropertySetter)(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx);

struct PropertyMarshalCtx
{
    // MetaTableKind kind;
    const PropertyInfo* property;
    Il2CppClass* valueTypeKlass;
    FnPropertyGetter getter;
    FnPropertySetter setter;
    const MarshalMetaInfo* meta;
    /* Precomputed: true when accessor is effectively sealed (non-virtual, method final, or klass sealed). */
    bool getterSealed;
    bool setterSealed;
};

struct MethodMarshalCtx;

typedef int (*FnLua2CsInvoker)(lua_State* L, void* target, int argStart, const MethodInfo* method, const MethodMarshalCtx* ctx);

typedef void* (*FnResolveMethodThis)(lua_State*, int);

/// Side data for methods with trailing C# default parameters. Absent methods keep MethodMarshalCtx.defaults == nullptr.
/// Dense trailing region only: slots[i] corresponds to CLR param (firstDefaultParamIndex + i).
struct MethodDefaultArgs
{
    void** defaultValueSlots;          // [defaultParamCount] valuetype native buffers
    Il2CppObject** defaultObjectSlots; // [defaultParamCount]; nullptr if no reference defaults
    int32_t minLuaArity;
    uint8_t firstDefaultParamIndex;
    uint8_t defaultParamCount; // parameters_count - firstDefaultParamIndex
};

struct MethodMarshalCtx
{
    const MethodInfo* method;
    FnResolveMethodThis resolveThis;
    FnLua2CsInvoker lua2CsInvoker;
    const MarshalMetaInfo** paramsMeta; // exclude this
    const MarshalMetaInfo* retMeta;
    const MethodDefaultArgs* defaults; // nullptr when method has no optional defaults
    int32_t luaArity; // Σ paramsMeta[i]->stackSlots treating params as 1 slot (extension: from param 1)
    /* CLR param index of params T[], or -1. Bind-time only; hot path never re-queries attributes. */
    int32_t paramsParamIndex;
    bool byVal;
    /* Precomputed: true when method is effectively sealed (non-virtual, method final, klass sealed, or byVal). */
    bool sealed;
    /* C# extension method bound as instance (static-as-instance). Set once at Bind. */
    bool isExtension;
};

static inline bool HasParamsArray(const MethodMarshalCtx* ctx)
{
    return ctx->paramsParamIndex >= 0;
}

static inline int32_t GetMaxLuaArity(const MethodMarshalCtx* ctx)
{
    return HasParamsArray(ctx) ? INT32_MAX : ctx->luaArity;
}

static inline int32_t GetMinLuaArity(const MethodMarshalCtx* ctx)
{
    if (ctx->defaults != nullptr)
        return ctx->defaults->minLuaArity;
    if (HasParamsArray(ctx))
    {
        const MarshalMetaInfo* meta = ctx->paramsMeta[ctx->paramsParamIndex];
        const int slots = meta->stackSlots > 0 ? meta->stackSlots : 1;
        const int32_t min = ctx->luaArity - slots;
        return min < 0 ? 0 : min;
    }
    return ctx->luaArity;
}

static inline bool HasOptionalDefaults(const MethodMarshalCtx* ctx)
{
    return ctx->defaults != nullptr;
}

static inline bool ParamHasCachedDefault(const MethodMarshalCtx* ctx, uint8_t paramIndex)
{
    return ctx->defaults != nullptr && paramIndex >= ctx->defaults->firstDefaultParamIndex;
}

static inline uint8_t DefaultSlotIndex(const MethodDefaultArgs* defaults, uint8_t paramIndex)
{
    IL2CPP_ASSERT(defaults != nullptr && paramIndex >= defaults->firstDefaultParamIndex);
    return static_cast<uint8_t>(paramIndex - defaults->firstDefaultParamIndex);
}

struct MethodGroup
{
    const MethodMarshalCtx** methods;
    size_t methodCount;
};

constexpr size_t kMaxSmallArgCount = 4;

struct MethodGroups
{
    const MethodGroup* smallArgCountMethodGroups[kMaxSmallArgCount + 1];
    const MethodMarshalCtx** largeArgCountMethods;
    size_t largeArgCountMethodCount;
};


enum class UserDataKind : uint8_t
{
    Unknown,
    ByObj,
    ByVal,
};

struct UserDataInfo
{
    UserDataKind kind;
    Il2CppClass* klass;
};

struct UserDataHeader
{
    UserDataKind kind;
};

struct ByValUserDataHeader
{
    UserDataHeader header;
    Il2CppClass* klass;

    // int32_t totalSizeWithHeader;

    uint8_t* Payload()
    {
        return reinterpret_cast<uint8_t*>(this + 1);
    }

    const uint8_t* Payload() const
    {
        return reinterpret_cast<const uint8_t*>(this + 1);
    }
};

struct ZLuaObjectUserData
{
    UserDataHeader header;
    uint32_t slotIndex;
    Il2CppObject* obj;
    Il2CppClass* viewKlass; // declared-type facade for IMT / cache key
};

// C# implicit conversion category for overload resolution (METHOD_OVERLOAD_SPEC §3.6).
enum class ConversionKind : uint8_t
{
    None = 0,
    Identity,
    ImplicitNumeric,
    ImplicitEnum,
    NullLiteral,
    ImplicitReference,
    ImplicitExtendedInteger,
    ImplicitBoxing,
    ImplicitArray,
    NotConvertible,
};

enum class MethodOverloadResolutionKind : uint8_t
{
    None = 0,
    BestMatch,
    Ambiguous,
};

struct MethodOverloadResolutionResult
{
    const MethodMarshalCtx* method;
    MethodOverloadResolutionKind kind;
};

struct LuaMethod : Il2CppObject
{
    lua_State* L;
    const MethodMarshalCtx* methodMarshalCtx;
    int32_t funcRef;
    bool disposed;
};


} // namespace zlua
