#include "MetaBinding.h"

#include "FieldBridge.h"
#include "MethodBridge.h"
#include "ObjectRegistry.h"

#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Runtime.h"

#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace novalua
{
    static std::unordered_map<Il2CppClass*, TypeBinding*> s_bindings;

    static int InvokeStaticMethod(lua_State* L);
    static int InvokeInstanceMethod(lua_State* L);
    static int ReleaseUserData(lua_State* L);
    static int ReleaseUserData(lua_State* L);

    static bool IsSpecialMethodName(const char* name)
    {
        if (name == nullptr || name[0] == '\0')
            return true;
        if (strcmp(name, ".ctor") == 0 || strcmp(name, ".cctor") == 0)
            return true;
        if (name[0] == 'g' && strncmp(name, "get_", 4) == 0)
            return true;
        if (name[0] == 's' && strncmp(name, "set_", 4) == 0)
            return true;
        if (name[0] == 'a' && strncmp(name, "add_", 4) == 0)
            return true;
        if (name[0] == 'r' && strncmp(name, "remove_", 7) == 0)
            return true;
        return false;
    }

    static bool IsPublicMethod(const MethodInfo* method)
    {
        return (method->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK) == METHOD_ATTRIBUTE_PUBLIC;
    }

    static bool IsPublicProperty(const PropertyInfo* property)
    {
        if (property == nullptr)
            return false;
        if (property->get != nullptr && IsPublicMethod(property->get))
            return true;
        if (property->set != nullptr && IsPublicMethod(property->set))
            return true;
        return false;
    }

    static int StoreClosureRef(lua_State* L)
    {
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }

    static int CreateStaticMethodClosureRef(lua_State* L, const MethodInfo* method)
    {
        lua_pushlightuserdata(L, (void*)method);
        lua_pushcclosure(L, InvokeStaticMethod, 1);
        return StoreClosureRef(L);
    }

    static int CreateInstanceMethodClosureRef(lua_State* L, const MethodInfo* method)
    {
        lua_pushlightuserdata(L, (void*)method);
        lua_pushcclosure(L, InvokeInstanceMethod, 1);
        return StoreClosureRef(L);
    }

    static int InvokeStaticMethod(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        if (method == nullptr)
            return luaL_error(L, "novalua: invalid static method binding");

        if (method->parameters_count == 0)
            return MethodBridge::InvokeStaticInt(L, method);
        if (method->parameters_count == 1)
            return MethodBridge::InvokeStaticVoidInt(L, method, 1);
        if (method->parameters_count == 2)
            return MethodBridge::InvokeStaticIntIntInt(L, method, 1);

        return luaL_error(L, "novalua: unsupported static method signature: %s", method->name);
    }

    static int InvokeInstanceMethod(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        Il2CppObject* instance = ObjectRegistry::GetObject(L, 1);
        if (method == nullptr || instance == nullptr)
            return luaL_error(L, "novalua: invalid instance method invocation");

        if (method->parameters_count == 0)
            return MethodBridge::InvokeInstanceInt(L, method, instance, 2);
        if (method->parameters_count == 1)
            return MethodBridge::InvokeInstanceVoidInt(L, method, instance, 2);

        return luaL_error(L, "novalua: unsupported instance method signature: %s", method->name);
    }

    static int PCallClosureRef(lua_State* L, int closureRef, int argCount, int resultCount)
    {
        if (closureRef == LUA_NOREF)
            return 0;

        lua_rawgeti(L, LUA_REGISTRYINDEX, closureRef);
        for (int i = 1; i <= argCount; ++i)
            lua_pushvalue(L, i);
        if (lua_pcall(L, argCount, resultCount, 0) != LUA_OK)
            return lua_error(L);
        return resultCount;
    }

    static int PushClosureRef(lua_State* L, int closureRef)
    {
        if (closureRef == LUA_NOREF)
            return 0;
        lua_rawgeti(L, LUA_REGISTRYINDEX, closureRef);
        return 1;
    }

    static int PushEventTable(lua_State* L, const MetaInfo& info)
    {
        lua_newtable(L);
        if (info.event.getterRef != LUA_NOREF)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, info.event.getterRef);
            lua_setfield(L, -2, "get");
        }
        if (info.event.setterRef != LUA_NOREF)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, info.event.setterRef);
            lua_setfield(L, -2, "set");
        }
        if (info.event.fireRef != LUA_NOREF)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, info.event.fireRef);
            lua_setfield(L, -2, "fire");
        }
        return 1;
    }

    static const MetaInfo* LookupMeta(const NameMetaMap* map, const char* key)
    {
        if (map == nullptr || key == nullptr)
            return nullptr;
        NameMetaMap::const_iterator it = map->find(key);
        if (it == map->end())
            return nullptr;
        return &it->second;
    }

    static int DispatchIndex(lua_State* L, const NameMetaMap* map, bool isStatic)
    {
        const char* key = lua_tostring(L, 2);
        const MetaInfo* info = LookupMeta(map, key);
        if (info == nullptr)
            return 0;

        switch (info->kind)
        {
        case MetaKind::Method:
            return PushClosureRef(L, info->method.closureRef);
        case MetaKind::Field:
        {
            void* fieldPtr = FieldBridge::GetFieldAddress(info->field.field, isStatic ? nullptr : ObjectRegistry::GetObject(L, 1));
            if (fieldPtr == nullptr || info->field.getter == nullptr)
                return 0;
            return info->field.getter(L, fieldPtr);
        }
        case MetaKind::Property:
        {
            if (info->property.getterRef == LUA_NOREF)
                return 0;
            return PCallClosureRef(L, info->property.getterRef, isStatic ? 0 : 1, 1);
        }
        case MetaKind::Event:
            return PushEventTable(L, *info);
        default:
            return 0;
        }
    }

    static int DispatchNewIndex(lua_State* L, const NameMetaMap* map, bool isStatic, bool allowRawSet)
    {
        const char* key = lua_tostring(L, 2);
        const MetaInfo* info = LookupMeta(map, key);
        if (info == nullptr)
        {
            if (allowRawSet)
            {
                lua_pushvalue(L, 3);
                lua_setfield(L, 1, key);
                return 0;
            }
            return luaL_error(L, "novalua: member not found: %s", key != nullptr ? key : "");
        }

        switch (info->kind)
        {
        case MetaKind::Field:
        {
            if (info->field.setter == nullptr)
                return luaL_error(L, "novalua: field is read-only: %s", key);
            void* fieldPtr = FieldBridge::GetFieldAddress(info->field.field, isStatic ? nullptr : ObjectRegistry::GetObject(L, 1));
            if (fieldPtr == nullptr)
                return luaL_error(L, "novalua: invalid target for field assignment: %s", key);
            return info->field.setter(L, info->field.field, fieldPtr, 3);
        }
        case MetaKind::Property:
        {
            if (info->property.setterRef == LUA_NOREF)
                return luaL_error(L, "novalua: property is read-only: %s", key);
            if (isStatic)
            {
                lua_pushvalue(L, 3);
                return PCallClosureRef(L, info->property.setterRef, 1, 0);
            }
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 3);
            return PCallClosureRef(L, info->property.setterRef, 2, 0);
        }
        case MetaKind::Event:
        {
            if (info->event.setterRef == LUA_NOREF)
                return luaL_error(L, "novalua: event does not support assignment: %s", key);
            if (isStatic)
            {
                lua_pushvalue(L, 3);
                return PCallClosureRef(L, info->event.setterRef, 1, 0);
            }
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 3);
            return PCallClosureRef(L, info->event.setterRef, 2, 0);
        }
        default:
            return luaL_error(L, "novalua: cannot assign to method: %s", key);
        }
    }

    static void CollectReservedMethods(Il2CppClass* klass, std::unordered_set<const MethodInfo*>& reserved)
    {
        void* propertyIter = NULL;
        const PropertyInfo* property;
        while ((property = il2cpp::vm::Class::GetProperties(klass, &propertyIter)) != NULL)
        {
            if (!IsPublicProperty(property))
                continue;
            if (property->get != nullptr)
                reserved.insert(property->get);
            if (property->set != nullptr)
                reserved.insert(property->set);
        }

        void* eventIter = NULL;
        const EventInfo* eventInfo;
        while ((eventInfo = il2cpp::vm::Class::GetEvents(klass, &eventIter)) != NULL)
        {
            if (eventInfo->add != nullptr)
                reserved.insert(eventInfo->add);
            if (eventInfo->remove != nullptr)
                reserved.insert(eventInfo->remove);
            if (eventInfo->raise != nullptr)
                reserved.insert(eventInfo->raise);
        }
    }

    static void RegisterMethods(lua_State* L, Il2CppClass* klass, NameMetaMap& map, bool isStatic, const std::unordered_set<const MethodInfo*>& reserved)
    {
        for (uint16_t i = 0; i < klass->method_count; ++i)
        {
            const MethodInfo* method = klass->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if (!IsPublicMethod(method))
                continue;
            if (IsSpecialMethodName(method->name))
                continue;
            if (reserved.find(method) != reserved.end())
                continue;

            const bool methodIsStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
            if (methodIsStatic != isStatic)
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Method;
            info.method.closureRef = isStatic
                ? CreateStaticMethodClosureRef(L, method)
                : CreateInstanceMethodClosureRef(L, method);
            map[method->name] = info;
        }
    }

    static void RegisterFields(lua_State* L, Il2CppClass* klass, NameMetaMap& map, bool isStatic)
    {
        (void)L;
        void* fieldIter = NULL;
        FieldInfo* field;
        while ((field = il2cpp::vm::Class::GetFields(klass, &fieldIter)) != NULL)
        {
            if (!FieldBridge::IsPublicField(field))
                continue;
            const bool fieldIsStatic = !il2cpp::vm::Field::IsInstance(field);
            if (fieldIsStatic != isStatic)
                continue;
            if (fieldIsStatic && !il2cpp::vm::Field::IsNormalStatic(field))
                continue;

            FieldAccessor accessor = FieldBridge::ResolveFieldAccessor(field);
            if (accessor.getter == nullptr)
                continue;
            if ((il2cpp::vm::Field::GetFlags(field) & FIELD_ATTRIBUTE_LITERAL) != 0)
                accessor.setter = nullptr;

            MetaInfo info = {};
            info.kind = MetaKind::Field;
            info.field.field = field;
            info.field.getter = accessor.getter;
            info.field.setter = accessor.setter;
            map[field->name] = info;
        }
    }

    static void RegisterProperties(lua_State* L, Il2CppClass* klass, NameMetaMap& map, bool isStatic)
    {
        void* propertyIter = NULL;
        const PropertyInfo* property;
        while ((property = il2cpp::vm::Class::GetProperties(klass, &propertyIter)) != NULL)
        {
            if (!IsPublicProperty(property))
                continue;

            const bool propertyIsStatic = property->get != nullptr
                ? ((property->get->flags & METHOD_ATTRIBUTE_STATIC) != 0)
                : ((property->set->flags & METHOD_ATTRIBUTE_STATIC) != 0);
            if (propertyIsStatic != isStatic)
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Property;
            info.property.getterRef = LUA_NOREF;
            info.property.setterRef = LUA_NOREF;
            if (property->get != nullptr)
                info.property.getterRef = propertyIsStatic
                    ? CreateStaticMethodClosureRef(L, property->get)
                    : CreateInstanceMethodClosureRef(L, property->get);
            if (property->set != nullptr)
                info.property.setterRef = propertyIsStatic
                    ? CreateStaticMethodClosureRef(L, property->set)
                    : CreateInstanceMethodClosureRef(L, property->set);
            map[property->name] = info;
        }
    }

    static void RegisterEvents(lua_State* L, Il2CppClass* klass, NameMetaMap& map, bool isStatic)
    {
        void* eventIter = NULL;
        const EventInfo* eventInfo;
        while ((eventInfo = il2cpp::vm::Class::GetEvents(klass, &eventIter)) != NULL)
        {
            const MethodInfo* addMethod = eventInfo->add;
            const bool eventIsStatic = addMethod != nullptr && ((addMethod->flags & METHOD_ATTRIBUTE_STATIC) != 0);
            if (eventIsStatic != isStatic)
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Event;
            info.event.getterRef = LUA_NOREF;
            info.event.setterRef = LUA_NOREF;
            info.event.fireRef = LUA_NOREF;

            if (eventInfo->add != nullptr)
                info.event.setterRef = eventIsStatic
                    ? CreateStaticMethodClosureRef(L, eventInfo->add)
                    : CreateInstanceMethodClosureRef(L, eventInfo->add);
            if (eventInfo->remove != nullptr)
                info.event.getterRef = eventIsStatic
                    ? CreateStaticMethodClosureRef(L, eventInfo->remove)
                    : CreateInstanceMethodClosureRef(L, eventInfo->remove);
            if (eventInfo->raise != nullptr)
                info.event.fireRef = eventIsStatic
                    ? CreateStaticMethodClosureRef(L, eventInfo->raise)
                    : CreateInstanceMethodClosureRef(L, eventInfo->raise);

            map[eventInfo->name] = info;
        }
    }

    static void BuildBinding(lua_State* L, TypeBinding* binding)
    {
        Il2CppClass* klass = binding->klass;
        il2cpp::vm::Class::Init(klass);

        std::unordered_set<const MethodInfo*> reserved;
        CollectReservedMethods(klass, reserved);

        RegisterProperties(L, klass, binding->instanceMap, false);
        RegisterProperties(L, klass, binding->staticMap, true);
        RegisterEvents(L, klass, binding->instanceMap, false);
        RegisterEvents(L, klass, binding->staticMap, true);
        RegisterFields(L, klass, binding->instanceMap, false);
        RegisterFields(L, klass, binding->staticMap, true);
        RegisterMethods(L, klass, binding->instanceMap, false, reserved);
        RegisterMethods(L, klass, binding->staticMap, true, reserved);
    }

    TypeBinding* MetaBinding::EnsureBinding(lua_State* L, Il2CppClass* klass)
    {
        std::unordered_map<Il2CppClass*, TypeBinding*>::iterator it = s_bindings.find(klass);
        if (it != s_bindings.end())
            return it->second;

        TypeBinding* binding = new TypeBinding();
        binding->klass = klass;
        BuildBinding(L, binding);
        s_bindings[klass] = binding;
        return binding;
    }

    int MetaBinding::InstanceIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        return DispatchIndex(L, map, false);
    }

    int MetaBinding::InstanceNewIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        return DispatchNewIndex(L, map, false, false);
    }

    int MetaBinding::StaticIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        const int result = DispatchIndex(L, map, true);
        if (result != 0)
            return result;

        lua_pushstring(L, lua_tostring(L, 2));
        lua_rawget(L, 1);
        return 1;
    }

    int MetaBinding::StaticNewIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        return DispatchNewIndex(L, map, true, true);
    }

    void MetaBinding::PushInstanceMetatable(lua_State* L, Il2CppClass* klass)
    {
        TypeBinding* binding = EnsureBinding(L, klass);
        lua_newtable(L);
        const int mtIndex = lua_absindex(L, -1);

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->instanceMap);
        lua_pushcclosure(L, InstanceIndex, 2);
        lua_setfield(L, mtIndex, "__index");

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->instanceMap);
        lua_pushcclosure(L, InstanceNewIndex, 2);
        lua_setfield(L, mtIndex, "__newindex");

        lua_pushcfunction(L, ReleaseUserData);
        lua_setfield(L, mtIndex, "__gc");
    }

    void MetaBinding::AttachStaticMetatable(lua_State* L, Il2CppClass* klass, int typeMetatableIndex)
    {
        TypeBinding* binding = EnsureBinding(L, klass);
        const int absIndex = lua_absindex(L, typeMetatableIndex);

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->staticMap);
        lua_pushcclosure(L, StaticIndex, 2);
        lua_setfield(L, absIndex, "__index");

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->staticMap);
        lua_pushcclosure(L, StaticNewIndex, 2);
        lua_setfield(L, absIndex, "__newindex");
    }

    static int ReleaseUserData(lua_State* L)
    {
        ObjectRegistry::ReleaseObject(L, 1);
        return 0;
    }
}
