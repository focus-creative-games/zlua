#include "MetaBinding.h"

#include "../LuaEnv.h"
#include "../MetadataUtil.h"
#include "../methodbridge/MethodBridge.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/ValueMarshaling.h"
#include "EventBridge.h"
#include "InstanceTarget.h"

#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Runtime.h"
#include "vm/Method.h"
#include "vm/Type.h"

#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace zlua
{
    static std::unordered_map<Il2CppClass*, TypeBinding*> s_bindings;

    static int InvokeStaticMethodDispatch(lua_State* L);
    static int InvokeInstanceMethodDispatch(lua_State* L);
    static int InvokeStaticMethodDirect(lua_State* L);
    static int InvokeInstanceMethodDirect(lua_State* L);
    static int ReleaseUserData(lua_State* L);
    static int ReleaseByValUserData(lua_State* L);
    static int DelegateInstanceCall(lua_State* L);

    static bool ShouldSkipMethodRegistration(const MethodInfo* method)
    {
        if (method == nullptr || method->name == nullptr || method->name[0] == '\0')
            return true;
        const char* name = method->name;
        if (strcmp(name, ".ctor") == 0 || strcmp(name, ".cctor") == 0)
            return true;
        if (strncmp(name, "add_", 4) == 0 || strncmp(name, "remove_", 7) == 0)
            return true;
        if (strncmp(name, "get_", 4) == 0 || strncmp(name, "set_", 4) == 0)
            return method->parameters_count == 0;
        return false;
    }

    static bool IsIndexerProperty(const PropertyInfo* property)
    {
        if (property == nullptr)
            return false;
        if (property->get != nullptr && property->get->parameters_count > 0)
            return true;
        if (property->set != nullptr && property->set->parameters_count > 1)
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
        //// Auto-implemented properties compile to private get_/set_ accessors; still bind by property name.
        //if (IsIndexerProperty(property))
        //    return false;
        return property->get != nullptr || property->set != nullptr;
    }

    static int StoreClosureRef(lua_State* L)
    {
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }

    static int CreateMethodDispatchClosureRef(lua_State* L, TypeBinding* binding, const char* name, bool isStatic)
    {
        lua_pushlightuserdata(L, binding);
        lua_pushstring(L, name);
        lua_pushboolean(L, isStatic ? 1 : 0);
        lua_pushcclosure(L, isStatic ? InvokeStaticMethodDispatch : InvokeInstanceMethodDispatch, 3);
        return StoreClosureRef(L);
    }

    static int CreateDirectMethodClosureRef(lua_State* L, const MethodInfo* method, bool isStatic)
    {
        lua_pushlightuserdata(L, (void*)method);
        lua_pushcclosure(L, isStatic ? InvokeStaticMethodDirect : InvokeInstanceMethodDirect, 1);
        return StoreClosureRef(L);
    }

    static int InvokeStaticMethodDirect(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        if (method == nullptr)
            return luaL_error(L, "zlua: invalid static method binding");
        return MethodBridge::InvokeMethod(L, method, nullptr, 1);
    }

    static int InvokeInstanceMethodDirect(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        void* target = InstanceTarget::ResolveMethodTarget(L, 1, method);
        if (method == nullptr || target == nullptr)
            return luaL_error(L, "zlua: invalid instance method invocation");
        return MethodBridge::InvokeMethod(L, method, target, 2);
    }

    static int InvokeStaticMethodDispatch(lua_State* L)
    {
        TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
        const char* name = lua_tostring(L, lua_upvalueindex(2));
        if (binding == nullptr || name == nullptr)
            return luaL_error(L, "zlua: invalid static method binding");

        const int argCount = lua_gettop(L);
        const MethodInfo* method = ValueMarshaling::FindMatchingMethod(binding->klass, name, true, L, 1, argCount);
        if (method == nullptr)
            return luaL_error(L, "zlua: no matching overload for %s", name);

        return MethodBridge::InvokeMethod(L, method, nullptr, 1);
    }

    static int InvokeInstanceMethodDispatch(lua_State* L)
    {
        TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
        const char* name = lua_tostring(L, lua_upvalueindex(2));
        if (binding == nullptr || name == nullptr)
            return luaL_error(L, "zlua: invalid instance method invocation");

        const int argCount = lua_gettop(L) - 1;
        const MethodInfo* method = ValueMarshaling::FindMatchingMethod(binding->klass, name, false, L, 2, argCount);
        if (method == nullptr)
            return luaL_error(L, "zlua: no matching overload for %s", name);

        void* target = InstanceTarget::ResolveMethodTarget(L, 1, method);
        if (target == nullptr)
            return luaL_error(L, "zlua: invalid instance method target");

        return MethodBridge::InvokeMethod(L, method, target, 2);
    }

    static int DelegateInstanceCall(lua_State* L)
    {
        Il2CppObject* obj = ObjectMarshal::Pop(L, 1);
        if (obj == nullptr || !MetadataUtil::IsDelegateClass(obj->klass))
            return luaL_error(L, "zlua: __call expects delegate userdata");

        Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(obj);
        const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(obj->klass);
        if (invokeMethod == nullptr)
            return luaL_error(L, "zlua: delegate Invoke missing");

        const int luaArgCount = lua_gettop(L) - 1;
        if (luaArgCount != invokeMethod->parameters_count)
        {
            return luaL_error(
                L,
                "zlua: delegate invoke expects %d argument(s), got %d",
                invokeMethod->parameters_count,
                luaArgCount);
        }

        const int paramCount = invokeMethod->parameters_count;
        std::vector<void*> params((size_t)paramCount, nullptr);
        std::vector<std::vector<uint8_t>> storage;
        storage.reserve((size_t)paramCount);

        for (int i = 0; i < paramCount; ++i)
        {
            const Il2CppType* paramType = invokeMethod->parameters[i];
            Il2CppClass* paramClass = il2cpp::vm::Class::FromIl2CppType(paramType, false);
            size_t sz = sizeof(void*);
            if (paramClass != nullptr && paramClass->byval_arg.valuetype)
                sz = ValueMarshaling::GetValueTypeInstanceSize(paramClass);
            else if (il2cpp::vm::Type::IsReference(paramType))
                sz = sizeof(Il2CppObject*);
            else if (paramType->type == IL2CPP_TYPE_R8)
                sz = sizeof(double);
            else if (paramType->type == IL2CPP_TYPE_R4)
                sz = sizeof(float);
            else
                sz = sizeof(int64_t);

            storage.emplace_back(sz);
            if (!ValueMarshaling::TryPop(L, i + 2, paramType, storage.back().data(), storage.back().size()))
                return luaL_error(L, "zlua: argument mismatch for delegate invoke");

            void* dataPtr = storage.back().data();
            params[(size_t)i] = il2cpp::vm::Type::IsReference(paramType) ? *(Il2CppObject**)dataPtr : dataPtr;
        }

        Il2CppException* exc = nullptr;
        Il2CppObject* ret = il2cpp::vm::Runtime::DelegateInvoke(delegate, params.data(), &exc);
        if (exc != nullptr)
            return luaL_error(L, "zlua: exception in delegate invoke");

        return ValueMarshaling::PushReturn(L, invokeMethod->return_type, ret);
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

    static int PCallClosureRefAt(lua_State* L, int closureRef, const int* argStackIndices, int argCount, int resultCount)
    {
        if (closureRef == LUA_NOREF)
            return 0;

        lua_rawgeti(L, LUA_REGISTRYINDEX, closureRef);
        for (int i = 0; i < argCount; ++i)
            lua_pushvalue(L, argStackIndices[i]);
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
            IL2CPP_ASSERT(info->field.getter != nullptr);
            if (isStatic)
                return info->field.getter(L, info->field.staticAddress);
            void* fieldPtr = InstanceTarget::ResolveFieldAddress(L, 1, info->field.instanceOffset, info->field.field);
            if (fieldPtr == nullptr)
                return luaL_error(L, "zlua: invalid target for field access: %s", key);
            return info->field.getter(L, fieldPtr);
        }
        case MetaKind::Property:
        {
            if (info->property.getterRef == LUA_NOREF)
                return luaL_error(L, "zlua: property has no getter: %s", key);
            return PCallClosureRef(L, info->property.getterRef, isStatic ? 0 : 1, 1);
        }
        case MetaKind::Event:
            return PushEventTable(L, *info);
        default:
            return 0;
        }
    }

    static int DispatchNewIndex(lua_State* L, const NameMetaMap* map, bool isStatic)
    {
        const char* key = lua_tostring(L, 2);
        const MetaInfo* info = LookupMeta(map, key);
        if (info == nullptr)
            return luaL_error(L, "zlua: member not found: %s", key != nullptr ? key : "");

        switch (info->kind)
        {
        case MetaKind::Field:
        {
            if (info->field.setter == nullptr)
                return luaL_error(L, "zlua: field is read-only: %s", key);
            if (isStatic)
                return info->field.setter(L, info->field.field, info->field.staticAddress, 3);
            void* fieldPtr = InstanceTarget::ResolveFieldAddress(L, 1, info->field.instanceOffset, info->field.field);
            if (fieldPtr == nullptr)
                return luaL_error(L, "zlua: invalid target for field assignment: %s", key);
            return info->field.setter(L, info->field.field, fieldPtr, 3);
        }
        case MetaKind::Property:
        {
            if (info->property.setterRef == LUA_NOREF)
                return luaL_error(L, "zlua: property is read-only: %s", key);
            if (isStatic)
            {
                const int args[] = { 3 };
                return PCallClosureRefAt(L, info->property.setterRef, args, 1, 0);
            }
            const int args[] = { 1, 3 };
            return PCallClosureRefAt(L, info->property.setterRef, args, 2, 0);
        }
        case MetaKind::Event:
        {
            if (info->event.setterRef == LUA_NOREF)
                return luaL_error(L, "zlua: event does not support assignment: %s", key);
            if (isStatic)
            {
                const int args[] = { 3 };
                return PCallClosureRefAt(L, info->event.setterRef, args, 1, 0);
            }
            const int args[] = { 1, 3 };
            return PCallClosureRefAt(L, info->event.setterRef, args, 2, 0);
        }
        default:
            return luaL_error(L, "zlua: cannot assign to method: %s", key);
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
            // Indexer accessors (get_Item / set_Item) are registered as methods, not properties.
            if (IsIndexerProperty(property))
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

    static void CollectBindableMethods(
        Il2CppClass* klass,
        bool isStatic,
        const std::unordered_set<const MethodInfo*>& reserved,
        std::vector<const MethodInfo*>& outMethods)
    {
        for (uint16_t i = 0; i < klass->method_count; ++i)
        {
            const MethodInfo* method = klass->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if (!IsPublicMethod(method))
                continue;
            if (ShouldSkipMethodRegistration(method))
                continue;
            if (il2cpp::vm::Method::IsGeneric(method))
                continue;
            if (reserved.find(method) != reserved.end())
                continue;

            const bool methodIsStatic = !il2cpp::vm::Method::IsInstance(method);
            if (methodIsStatic != isStatic)
                continue;

            outMethods.push_back(method);
        }
    }

    static void ValidateMethodAliasKeys(lua_State* L, Il2CppClass* ownerClass, const std::vector<const MethodInfo*>& methods)
    {
        std::unordered_set<std::string> methodNames;
        for (const MethodInfo* method : methods)
            methodNames.insert(method->name);

        std::unordered_map<std::string, const MethodInfo*> aliasOwners;
        for (const MethodInfo* method : methods)
        {
            std::string alias;
            if (!MetadataUtil::TryReadLuaAlias(method, alias))
                continue;

            if (methodNames.find(alias) != methodNames.end())
            {
                std::string message = std::string("zlua: Lua alias '") + alias + "' on "
                    + MetadataUtil::GetSignatureTypeName(ownerClass) + "." + method->name
                    + " must not duplicate a non-alias method name";
                luaL_error(L, "%s", message.c_str());
            }

            std::unordered_map<std::string, const MethodInfo*>::iterator existing = aliasOwners.find(alias);
            if (existing != aliasOwners.end())
            {
                std::string message = std::string("zlua: duplicate Lua alias '") + alias + "' on "
                    + MetadataUtil::GetSignatureTypeName(ownerClass) + " (" + existing->second->name + " and "
                    + method->name + ")";
                luaL_error(L, "%s", message.c_str());
            }

            aliasOwners[alias] = method;
        }
    }

    void MetaBinding::ValidateMethodAliasKeysOrThrow(lua_State* L, Il2CppClass* klass)
    {
        if (klass == nullptr || L == nullptr)
            return;

        il2cpp::vm::Class::Init(klass);
        std::vector<Il2CppClass*> chain;
        for (Il2CppClass* cursor = klass; cursor != nullptr; cursor = cursor->parent)
            chain.push_back(cursor);

        std::vector<const MethodInfo*> staticMethods;
        std::vector<const MethodInfo*> instanceMethods;
        for (int i = (int)chain.size() - 1; i >= 0; --i)
        {
            Il2CppClass* current = chain[(size_t)i];
            std::unordered_set<const MethodInfo*> reserved;
            CollectReservedMethods(current, reserved);
            CollectBindableMethods(current, true, reserved, staticMethods);
            CollectBindableMethods(current, false, reserved, instanceMethods);
        }

        ValidateMethodAliasKeys(L, klass, staticMethods);
        ValidateMethodAliasKeys(L, klass, instanceMethods);
    }

    int MetaBinding::PushMethodClosure(lua_State* L, const MethodInfo* method, bool isStatic)
    {
        if (method == nullptr)
            return luaL_error(L, "zlua: invalid method");
        return PushClosureRef(L, CreateDirectMethodClosureRef(L, method, isStatic));
    }

    bool MetaBinding::TryRegisterMethodAlias(
        lua_State* L,
        Il2CppClass* klass,
        bool isStatic,
        const char* aliasName,
        int closureStackIndex)
    {
        if (klass == nullptr || aliasName == nullptr || aliasName[0] == '\0')
            return false;

        TypeBinding* binding = EnsureBinding(L, klass);
        NameMetaMap& map = isStatic ? binding->staticMap : binding->instanceMap;
        if (map.find(aliasName) != map.end())
            return false;

        lua_pushvalue(L, closureStackIndex);
        const int closureRef = StoreClosureRef(L);

        MetaInfo info = {};
        info.kind = MetaKind::Method;
        info.method.closureRef = closureRef;
        map[aliasName] = info;
        return true;
    }

    static void RegisterMethodGroup(
        lua_State* L,
        TypeBinding* binding,
        const char* name,
        const std::vector<const MethodInfo*>& overloads,
        bool isStatic,
        NameMetaMap& map)
    {
        if (overloads.empty() || map.find(name) != map.end())
            return;

        MetaInfo info = {};
        info.kind = MetaKind::Method;
        if (overloads.size() == 1)
            info.method.closureRef = CreateDirectMethodClosureRef(L, overloads[0], isStatic);
        else
            info.method.closureRef = CreateMethodDispatchClosureRef(L, binding, name, isStatic);
        map[name] = info;
    }

    static void RegisterMethodAliases(
        lua_State* L,
        const std::vector<const MethodInfo*>& methods,
        bool isStatic,
        NameMetaMap& map)
    {
        for (const MethodInfo* method : methods)
        {
            std::string alias;
            if (!MetadataUtil::TryReadLuaAlias(method, alias))
                continue;
            if (map.find(alias) != map.end())
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Method;
            info.method.closureRef = CreateDirectMethodClosureRef(L, method, isStatic);
            map[alias] = info;
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
            if (map.find(field->name) != map.end())
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
            if (isStatic)
                info.field.staticAddress = FieldBridge::ComputeStaticFieldAddress(field);
            else
                info.field.instanceOffset = FieldBridge::ComputeInstanceFieldOffset(field);
            map[field->name] = info;
        }
    }

    static void RegisterProperties(lua_State* L, TypeBinding* binding, Il2CppClass* klass, NameMetaMap& map, bool isStatic)
    {
        void* propertyIter = NULL;
        const PropertyInfo* property;
        while ((property = il2cpp::vm::Class::GetProperties(klass, &propertyIter)) != NULL)
        {
            if (!IsPublicProperty(property))
                continue;
            if (IsIndexerProperty(property))
                continue;

            const bool propertyIsStatic = property->get != nullptr
                ? ((property->get->flags & METHOD_ATTRIBUTE_STATIC) != 0)
                : ((property->set->flags & METHOD_ATTRIBUTE_STATIC) != 0);
            if (propertyIsStatic != isStatic)
                continue;
            if (map.find(property->name) != map.end())
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Property;
            info.property.getterRef = LUA_NOREF;
            info.property.setterRef = LUA_NOREF;
            if (property->get != nullptr)
                info.property.getterRef = CreateDirectMethodClosureRef(L, property->get, propertyIsStatic);
            if (property->set != nullptr)
                info.property.setterRef = CreateDirectMethodClosureRef(L, property->set, propertyIsStatic);
            map[property->name] = info;
        }
    }

    static void RegisterEvents(lua_State* L, TypeBinding* binding, Il2CppClass* klass, NameMetaMap& map, bool isStatic)
    {
        (void)binding;
        void* eventIter = NULL;
        const EventInfo* eventInfo;
        while ((eventInfo = il2cpp::vm::Class::GetEvents(klass, &eventIter)) != NULL)
        {
            const MethodInfo* addMethod = eventInfo->add;
            const bool eventIsStatic = addMethod != nullptr && ((addMethod->flags & METHOD_ATTRIBUTE_STATIC) != 0);
            if (eventIsStatic != isStatic)
                continue;
            if (map.find(eventInfo->name) != map.end())
                continue;

            MetaInfo info = {};
            info.kind = MetaKind::Event;
            info.event.getterRef = LUA_NOREF;
            info.event.setterRef = LUA_NOREF;
            info.event.fireRef = LUA_NOREF;

            const int eventId = EventBridge::RegisterBinding(eventInfo, eventIsStatic);
            info.event.getterRef = EventBridge::CreateAddClosureRef(L, eventId);
            info.event.setterRef = EventBridge::CreateRemoveClosureRef(L, eventId);
            info.event.fireRef = EventBridge::CreateFireClosureRef(L, eventId);

            map[eventInfo->name] = info;
        }
    }

    static void BuildBinding(lua_State* L, TypeBinding* binding)
    {
        Il2CppClass* klass = binding->klass;
        il2cpp::vm::Class::Init(klass);

        std::vector<Il2CppClass*> chain;
        for (Il2CppClass* cursor = klass; cursor != nullptr; cursor = cursor->parent)
            chain.push_back(cursor);

        std::vector<const MethodInfo*> staticMethods;
        std::vector<const MethodInfo*> instanceMethods;
        for (int i = (int)chain.size() - 1; i >= 0; --i)
        {
            Il2CppClass* current = chain[(size_t)i];
            std::unordered_set<const MethodInfo*> reserved;
            CollectReservedMethods(current, reserved);

            RegisterProperties(L, binding, current, binding->instanceMap, false);
            RegisterProperties(L, binding, current, binding->staticMap, true);
            RegisterEvents(L, binding, current, binding->instanceMap, false);
            RegisterEvents(L, binding, current, binding->staticMap, true);
            RegisterFields(L, current, binding->instanceMap, false);
            RegisterFields(L, current, binding->staticMap, true);
            CollectBindableMethods(current, true, reserved, staticMethods);
            CollectBindableMethods(current, false, reserved, instanceMethods);
        }

        std::unordered_map<std::string, std::vector<const MethodInfo*>> staticGroups;
        std::unordered_map<std::string, std::vector<const MethodInfo*>> instanceGroups;
        for (const MethodInfo* method : staticMethods)
            staticGroups[method->name].push_back(method);
        for (const MethodInfo* method : instanceMethods)
            instanceGroups[method->name].push_back(method);

        for (const auto& kv : staticGroups)
            RegisterMethodGroup(L, binding, kv.first.c_str(), kv.second, true, binding->staticMap);
        for (const auto& kv : instanceGroups)
            RegisterMethodGroup(L, binding, kv.first.c_str(), kv.second, false, binding->instanceMap);

        RegisterMethodAliases(L, staticMethods, true, binding->staticMap);
        RegisterMethodAliases(L, instanceMethods, false, binding->instanceMap);
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
        const int result = DispatchIndex(L, map, false);
        if (result != 0)
            return result;

        const char* key = lua_tostring(L, 2);
        lua_pushnil(L);
        return 1;
    }

    int MetaBinding::InstanceNewIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        return DispatchNewIndex(L, map, false);
    }

    int MetaBinding::StaticIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        const int result = DispatchIndex(L, map, true);
        if (result != 0)
            return result;

        // Extras such as _default / __call live on the static metatable (SMT), not the type table.
        if (lua_getmetatable(L, 1) == 0)
            return 0;

        lua_pushvalue(L, 2);
        lua_rawget(L, -2);
        lua_remove(L, -2);
        return 1;
    }

    int MetaBinding::StaticNewIndex(lua_State* L)
    {
        NameMetaMap* map = (NameMetaMap*)lua_touserdata(L, lua_upvalueindex(2));
        return DispatchNewIndex(L, map, true);
    }

    static void FillInstanceMetatable(
        lua_State* L,
        TypeBinding* binding,
        int typeTableIndex,
        int mtIndex,
        const char* userDataKind,
        lua_CFunction gc,
        lua_CFunction tostring)
    {
        lua_pushvalue(L, typeTableIndex);
        lua_setfield(L, mtIndex, "__type");

        if (userDataKind != nullptr)
        {
            lua_pushstring(L, userDataKind);
            lua_setfield(L, mtIndex, "__zlua_ud_kind");
        }

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->instanceMap);
        lua_pushcclosure(L, MetaBinding::InstanceIndex, 2);
        lua_setfield(L, mtIndex, "__index");

        lua_pushlightuserdata(L, binding);
        lua_pushlightuserdata(L, &binding->instanceMap);
        lua_pushcclosure(L, MetaBinding::InstanceNewIndex, 2);
        lua_setfield(L, mtIndex, "__newindex");

        if (gc != nullptr)
        {
            lua_pushcfunction(L, gc);
            lua_setfield(L, mtIndex, "__gc");
        }

        if (tostring != nullptr)
        {
            lua_pushcfunction(L, tostring);
            lua_setfield(L, mtIndex, "__tostring");
        }
    }

    void MetaBinding::PushReferenceInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex)
    {
        TypeBinding* binding = EnsureBinding(L, klass);
        lua_newtable(L);
        const int mtIndex = lua_absindex(L, -1);
        FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, nullptr, ReleaseUserData, nullptr);
        if (MetadataUtil::IsDelegateClass(klass))
        {
            lua_pushcfunction(L, DelegateInstanceCall);
            lua_setfield(L, mtIndex, "__call");
        }
    }

    void MetaBinding::PushByValInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring)
    {
        TypeBinding* binding = EnsureBinding(L, klass);
        lua_newtable(L);
        const int mtIndex = lua_absindex(L, -1);
        FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, "byval", ReleaseByValUserData, tostring);
    }

    void MetaBinding::PushByObjInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex)
    {
        TypeBinding* binding = EnsureBinding(L, klass);
        lua_newtable(L);
        const int mtIndex = lua_absindex(L, -1);
        FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, "byobj", ReleaseUserData, nullptr);
    }

    static int ReleaseByValUserData(lua_State* L)
    {
        (void)L;
        return 0;
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
        ObjectMarshal::Release(L, 1);
        return 0;
    }
}
