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
    const FieldInfo* field;

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

struct MethodMarshalCtx
{
    const MethodInfo* method;
    FnResolveMethodThis resolveThis;
    FnLua2CsInvoker lua2CsInvoker;
    const MarshalMetaInfo** paramsMeta; // exclude this
    const MarshalMetaInfo* retMeta;
    int32_t valueSize; // sizeof(void*) for refrence type, klass->instance - sizeof(Il2CppObject) for struct
    int32_t totalParamsSize;
    int32_t luaArity; // Σ paramsMeta[i]->stackSlots
    bool byVal;
    /* Precomputed: true when method is effectively sealed (non-virtual, method final, klass sealed, or byVal). */
    bool sealed;
};

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
