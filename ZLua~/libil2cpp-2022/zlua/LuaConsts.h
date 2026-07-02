#pragma once

namespace zlua
{
struct LuaConsts
{
    // Standard Lua metamethod keys
    static constexpr const char* MetaIndex = "__index";
    static constexpr const char* MetaNewIndex = "__newindex";
    static constexpr const char* MetaCall = "__call";
    static constexpr const char* MetaGc = "__gc";
    static constexpr const char* MetaToString = "__tostring";
    static constexpr const char* MetaLen = "__len";
    static constexpr const char* MetaMode = "__mode";

    // Weak-table __mode values
    static constexpr const char* WeakModeValue = "v";

    // ZLua type-table keys
    static constexpr const char* FullName = "__fullname";
    static constexpr const char* Klass = "__klass";
    static constexpr const char* ByValInstanceMt = "__byval_instance_mt";
    static constexpr const char* ByObjInstanceMt = "__byobj_instance_mt";
    static constexpr const char* Struct = "__struct";
    static constexpr const char* Enum = "__enum";
    static constexpr const char* Type = "__type";
    static constexpr const char* Nullable = "__nullable";
    static constexpr const char* UdKind = "__zlua_ud_kind";

    // ZLua userdata kind values
    static constexpr const char* UdKindByVal = "byval";
    static constexpr const char* UdKindByObj = "byobj";
    static constexpr const char* UdKindOpaqueParameter = "opaque_parameter";

    // ZLua property/event table keys
    static constexpr const char* Get = "get";
    static constexpr const char* Set = "set";
    static constexpr const char* Fire = "fire";

    // ZLua static metatable extra field
    static constexpr const char* Default = "_default";
};
} // namespace zlua
