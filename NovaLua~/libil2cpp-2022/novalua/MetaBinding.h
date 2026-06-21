#pragma once

#include "FieldBridge.h"

#include <string>
#include <unordered_map>

struct Il2CppClass;

namespace novalua
{
    enum class MetaKind : uint8_t
    {
        Method,
        Field,
        Property,
        Event,
    };

    struct MetaInfo
    {
        MetaKind kind;
        union
        {
            struct { int closureRef; } method;
            struct { const FieldInfo* field; FnFieldGetter getter; FnFieldSetter setter; } field;
            struct { int getterRef; int setterRef; } property;
            struct { int getterRef; int setterRef; int fireRef; } event;
        };
    };

    using NameMetaMap = std::unordered_map<std::string, MetaInfo>;

    struct TypeBinding
    {
        Il2CppClass* klass;
        NameMetaMap instanceMap;
        NameMetaMap staticMap;
    };

    class MetaBinding
    {
    public:
        static TypeBinding* EnsureBinding(lua_State* L, Il2CppClass* klass);
        static void PushInstanceMetatable(lua_State* L, Il2CppClass* klass);
        static void AttachStaticMetatable(lua_State* L, Il2CppClass* klass, int typeMetatableIndex);

        static int InstanceIndex(lua_State* L);
        static int InstanceNewIndex(lua_State* L);
        static int StaticIndex(lua_State* L);
        static int StaticNewIndex(lua_State* L);
    };
}
