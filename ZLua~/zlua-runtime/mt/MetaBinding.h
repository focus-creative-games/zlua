#pragma once
#include <deque>
#include <vector>

#include "../utils/Collection.h"
#include "../bridge/PropertyBridge.h"
#include "../bridge/FieldBridge.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/MarshalMeta.h"

#include "../utils/dynamic_array.h"

struct Il2CppClass;

namespace zlua
{
enum class MetaKind : uint8_t
{
    Method,
    Field,
    Property,
    // Event,
};

struct MetaInfo
{
    MetaKind kind;

    union
    {
        struct
        {
            int closureRef;
        } method;

        FieldMarshalCtx field;
        PropertyMarshalCtx property;

        // struct
        // {
        //     const EventInfo* event;
        //     int getterRef;
        //     int setterRef;
        //     int fireRef;
        // } event;
    };
};

typedef AppendOnlyStringHashMap<MetaInfo> NameMetaMap;

#if ZLUA_FAST_METATABLE
/* Resolve this / field address strategy for tagged getters/setters (OPTIMIZATION.md). */
enum class FastInstanceKind : uint8_t
{
    Static = 0,
    StructByVal,
    StructByObj,
    ReferenceByObj,
};

struct FastMemberCtx
{
    FastInstanceKind kind;
    MetaInfo info;
};
#endif

struct TypeBinding
{
    Il2CppClass* klass;
    mutable NameMetaMap byobjInstanceMap;
    mutable NameMetaMap byvalInstanceMap;
    mutable NameMetaMap staticMap;
    const MethodMarshalCtx* uniqueCtorMethod;
    const MethodGroups* ctorGroups;
#if ZLUA_FAST_METATABLE
    /* deque: pointers passed to Lua lightuserdata stay valid across push_back. */
    std::deque<FastMemberCtx> fastMemberStorage;
#endif
};

enum class ClosureKind
{
    Unknown,
    Direct,
    Indirect,
    DirectGeneric,
};

class MetaBinding
{
  public:
    static TypeBinding* EnsureBinding(lua_State* L, Il2CppClass* klass);
    static bool TryRegisterMethodAlias(lua_State* L, Il2CppClass* klass, bool isStatic, const std::string& aliasName, int closureStackIndex);
    static MethodMarshalCtx* CreateMethodMarshalCtx(lua_State* L, const MethodInfo* method, bool isByVal);
    static int CreateDirectMethodClosureRef(lua_State* L, const MethodInfo* method, const TypeBinding* binding, bool isStatic, bool isByVal);
    // static int PushMethodClosure(lua_State* L, const MethodInfo* method, bool isStatic);
    static bool IsDirectMethodClosure(lua_State* L, int closureStackIndex);
    static const MethodMarshalCtx* GetMethodMarshalCtxFromClosure(lua_State* L, int closureStackIndex);
    static const TypeBinding* GetTypeBindingFromClosure(lua_State* L, int closureStackIndex);
};
} // namespace zlua
