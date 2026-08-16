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
#include <unordered_map>

#include "../utils/CsStringHash.h"
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

/// Compact tagged entry for NameMetaMap. Field/Property payloads live on LuaMetadataAlloc.
struct MetaInfo
{
    MetaKind kind;
    union
    {
        int closureRef; // Method
        FieldMarshalCtx* field;
        PropertyMarshalCtx* property;
    };
};

typedef std::unordered_map<const char*, MetaInfo, CsStringHash, CsStringEqual> NameMetaMap;

#if ZLUA_FAST_METATABLE
/* Resolve this / field address strategy for tagged getters/setters (OPTIMIZATION.md). */
enum class FastInstanceKind : uint8_t
{
    Static = 0,
    StructByVal,
    StructByObj,
    ReferenceByObj,
};

/// Slim ctx for lightuserdata upvalues. Field/Property point at the same heap ctx as NameMetaMap.
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
