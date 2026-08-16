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
};

enum class MetaTableKind : uint8_t
{
    ReferenceByObj,
    StructByObj,
    StructByVal,
    Static,
};

struct MarshalMetaInfo;
typedef void (*FnMarshalLua2Cs)(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* ctx);
typedef void (*FnMarshalCs2Lua)(lua_State* L, void* address, const MarshalMetaInfo* ctx);

/// Pre-resolved Table / UnpackedValues member (no name lookup on hot path).
struct CompositeMember
{
    const char* clrName;
    bool optional;
    bool isField;
    union
    {
        const FieldInfo* field;
        const PropertyInfo* property;
    };
    /// Offset into valuetype payload (excludes Il2CppObject header). Valid when isField.
    int32_t fieldOffset;
    const MarshalMetaInfo* memberMeta;
};

struct MarshalMetaInfo
{
    FnMarshalLua2Cs lua2csWriter;
    FnMarshalCs2Lua cs2luaWriter;
    const Il2CppType* type;
    Il2CppClass* typeKlass; // declared type for marshal facade (class/interface/object/array/struct/...)
    int32_t size;
    int luaByValRefIndex;
    int luaByObjRefIndex;
    bool passByValue;
    LuaMarshalType marshalType;
    uint16_t stackSlots; // Default/Table/... = 1; UnpackedValues = memberCount
    uint16_t memberCount;
    const CompositeMember* members;
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
    int32_t minLuaArity;
    uint8_t firstDefaultParamIndex;
    uint8_t defaultParamCount; // parameters_count - firstDefaultParamIndex
    void** defaultValueSlots;          // [defaultParamCount] valuetype native buffers
    Il2CppObject** defaultObjectSlots; // [defaultParamCount]; nullptr if no reference defaults
};

struct MethodMarshalCtx
{
    const MethodInfo* method;
    FnResolveMethodThis resolveThis;
    FnLua2CsInvoker lua2CsInvoker;
    const MarshalMetaInfo** paramsMeta; // exclude this
    const MarshalMetaInfo* retMeta;
    int32_t luaArity; // max Σ paramsMeta[i]->stackSlots (extension: from param 1)
    const MethodDefaultArgs* defaults; // nullptr when method has no optional defaults
    bool byVal;
    /* Precomputed: true when method is effectively sealed (non-virtual, method final, klass sealed, or byVal). */
    bool sealed;
    /* C# extension method bound as instance (static-as-instance). Set once at Bind. */
    bool isExtension;
};

static inline int32_t GetMinLuaArity(const MethodMarshalCtx* ctx)
{
    return ctx->defaults != nullptr ? ctx->defaults->minLuaArity : ctx->luaArity;
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
    MethodOverloadResolutionKind kind;
    const MethodMarshalCtx* method;
};

struct LuaMethod : Il2CppObject
{
    bool disposed;
    lua_State* L;
    int32_t funcRef;
    const MethodMarshalCtx* methodMarshalCtx;
};


} // namespace zlua
