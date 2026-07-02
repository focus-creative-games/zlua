#include "EventBridge.h"

#include "../LuaEnv.h"
#include "../marshal/Marshaling.h"
#include "../marshal/ObjectMarshal.h"
#include "../methodbridge/MethodBridge.h"
#include "InstanceTarget.h"

#include "il2cpp-object-internals.h"
#include "vm/Class.h"
#include "vm/MetadataCache.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Type.h"

#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

namespace zlua
{
    enum class LuaDelegateStubKind : uint8_t
    {
        Void,
        Int32,
        String,
        Unsupported,
    };

    struct EventBinding
    {
        const EventInfo* eventInfo = nullptr;
        const MethodInfo* addMethod = nullptr;
        const MethodInfo* removeMethod = nullptr;
        const MethodInfo* raiseMethod = nullptr;
        const MethodInfo* invokeMethod = nullptr;
        Il2CppClass* delegateClass = nullptr;
        bool isStatic = false;
        LuaDelegateStubKind stubKind = LuaDelegateStubKind::Unsupported;
    };

    struct EventSubscriptionKey
    {
        Il2CppObject* target = nullptr;
        int eventId = 0;
        const void* luaFunctionPointer = nullptr;

        bool operator==(const EventSubscriptionKey& other) const
        {
            return eventId == other.eventId
                && luaFunctionPointer == other.luaFunctionPointer
                && target == other.target;
        }
    };

    struct EventSubscriptionKeyHash
    {
        size_t operator()(const EventSubscriptionKey& key) const
        {
            return std::hash<int>()(key.eventId)
                ^ (std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(key.target)) << 1)
                ^ (std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(key.luaFunctionPointer)) << 2);
        }
    };

    static std::vector<EventBinding> s_eventBindings;
    static std::unordered_map<EventSubscriptionKey, Il2CppDelegate*, EventSubscriptionKeyHash> s_eventHandlerCache;
    static std::unordered_map<Il2CppDelegate*, int> s_delegateFunctionRefs;
    static int s_nextEventId = 1;

    static void LuaDelegateStub_Void(Il2CppObject* closedThis, const MethodInfo* method);
    static void LuaDelegateStub_Int32(Il2CppObject* closedThis, int32_t arg0, const MethodInfo* method);
    static void LuaDelegateStub_String(Il2CppObject* closedThis, Il2CppString* arg0, const MethodInfo* method);

    static bool CallLuaFunctionRef(int funcRef, const std::function<void(lua_State*)>& pushArgs, int argCount)
    {
        lua_State* L = LuaEnv::GetState();
        const int restoreTop = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);

        lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
        if (!lua_isfunction(L, -1))
        {
            lua_settop(L, restoreTop);
            return false;
        }

        if (pushArgs)
            pushArgs(L);

        const int callResult = lua_pcall(L, argCount, 0, errfunc);
        lua_settop(L, restoreTop);
        return callResult == LUA_OK;
    }

    static void LuaDelegateStub_Void(Il2CppObject* closedThis, const MethodInfo* method)
    {
        (void)method;
        Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(closedThis);
        std::unordered_map<Il2CppDelegate*, int>::iterator it = s_delegateFunctionRefs.find(delegate);
        if (it == s_delegateFunctionRefs.end())
            return;
        CallLuaFunctionRef(it->second, nullptr, 0);
    }

    static void LuaDelegateStub_Int32(Il2CppObject* closedThis, int32_t arg0, const MethodInfo* method)
    {
        (void)method;
        Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(closedThis);
        std::unordered_map<Il2CppDelegate*, int>::iterator it = s_delegateFunctionRefs.find(delegate);
        if (it == s_delegateFunctionRefs.end())
            return;
        CallLuaFunctionRef(it->second, [arg0](lua_State* L) { lua_pushinteger(L, arg0); }, 1);
    }

    static void LuaDelegateStub_String(Il2CppObject* closedThis, Il2CppString* arg0, const MethodInfo* method)
    {
        (void)method;
        Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(closedThis);
        std::unordered_map<Il2CppDelegate*, int>::iterator it = s_delegateFunctionRefs.find(delegate);
        if (it == s_delegateFunctionRefs.end())
            return;
        CallLuaFunctionRef(it->second, [arg0](lua_State* L) { Marshaling::PushCString(L, arg0); }, 1);
    }

    static LuaDelegateStubKind ResolveStubKind(const MethodInfo* invokeMethod)
    {
        if (invokeMethod == nullptr)
            return LuaDelegateStubKind::Unsupported;
        if (invokeMethod->return_type != nullptr && invokeMethod->return_type->type != IL2CPP_TYPE_VOID)
            return LuaDelegateStubKind::Unsupported;
        if (invokeMethod->parameters_count == 0)
            return LuaDelegateStubKind::Void;
        if (invokeMethod->parameters_count != 1)
            return LuaDelegateStubKind::Unsupported;
        switch (invokeMethod->parameters[0]->type)
        {
        case IL2CPP_TYPE_I4:
            return LuaDelegateStubKind::Int32;
        case IL2CPP_TYPE_STRING:
            return LuaDelegateStubKind::String;
        default:
            return LuaDelegateStubKind::Unsupported;
        }
    }

    static Il2CppMethodPointer SelectStub(LuaDelegateStubKind kind)
    {
        switch (kind)
        {
        case LuaDelegateStubKind::Void:
            return reinterpret_cast<Il2CppMethodPointer>(LuaDelegateStub_Void);
        case LuaDelegateStubKind::Int32:
            return reinterpret_cast<Il2CppMethodPointer>(LuaDelegateStub_Int32);
        case LuaDelegateStubKind::String:
            return reinterpret_cast<Il2CppMethodPointer>(LuaDelegateStub_String);
        default:
            return nullptr;
        }
    }

    static const MethodInfo* ResolveRaiseMethod(const EventInfo* eventInfo, bool isStatic)
    {
        if (eventInfo == nullptr)
            return nullptr;
        if (eventInfo->raise != nullptr)
            return eventInfo->raise;
        if (eventInfo->parent == nullptr || eventInfo->name == nullptr)
            return nullptr;

        std::string raiseName = std::string("Raise") + eventInfo->name;
        il2cpp::vm::Class::Init(eventInfo->parent);
        for (uint16_t i = 0; i < eventInfo->parent->method_count; ++i)
        {
            const MethodInfo* method = eventInfo->parent->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if (strcmp(method->name, raiseName.c_str()) != 0)
                continue;
            const bool methodIsStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
            if (methodIsStatic != isStatic)
                continue;
            return method;
        }
        return nullptr;
    }

    static Il2CppDelegate* CreateLuaDelegate(lua_State* L, int handlerIndex, const EventBinding& binding)
    {
        if (binding.delegateClass == nullptr || binding.invokeMethod == nullptr)
            return nullptr;

        Il2CppMethodPointer stub = SelectStub(binding.stubKind);
        if (stub == nullptr)
            return nullptr;
        if (!lua_isfunction(L, handlerIndex))
            return nullptr;

        lua_pushvalue(L, handlerIndex);
        const int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (funcRef == LUA_NOREF)
            return nullptr;

        Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(il2cpp::vm::Object::New(binding.delegateClass));
        if (delegate == nullptr)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
            return nullptr;
        }

        il2cpp::vm::Type::ConstructClosedDelegate(
            delegate,
            reinterpret_cast<Il2CppObject*>(delegate),
            stub,
            binding.invokeMethod);
        s_delegateFunctionRefs[delegate] = funcRef;
        return delegate;
    }

    static bool TryResolveSubscriptionTarget(
        lua_State* L,
        const EventBinding& binding,
        bool requireHandler,
        Il2CppObject** outTarget,
        int* outHandlerIndex,
        int* outArgStartIndex)
    {
        if (outTarget == nullptr || outHandlerIndex == nullptr || outArgStartIndex == nullptr)
            return false;

        *outTarget = nullptr;
        *outHandlerIndex = 0;
        *outArgStartIndex = 0;

        if (binding.isStatic)
        {
            *outHandlerIndex = 1;
            *outArgStartIndex = 1;
            return !requireHandler || (lua_gettop(L) >= 1 && lua_isfunction(L, 1));
        }

        if (lua_gettop(L) < 1)
            return false;

        *outTarget = ObjectMarshal::Pop(L, 1);
        if (*outTarget == nullptr)
            return false;

        if (requireHandler)
        {
            if (lua_gettop(L) < 2 || !lua_isfunction(L, 2))
                return false;
            *outHandlerIndex = 2;
        }

        *outArgStartIndex = 2;
        return true;
    }

    static void InvokeAddOrRemove(
        lua_State* L,
        const EventBinding& binding,
        Il2CppObject* target,
        Il2CppDelegate* delegate,
        bool isAdd)
    {
        const MethodInfo* accessor = isAdd ? binding.addMethod : binding.removeMethod;
        if (accessor == nullptr || delegate == nullptr)
            return;

        void* params[1] = { delegate };
        Il2CppException* exc = nullptr;
        il2cpp::vm::Runtime::Invoke(accessor, binding.isStatic ? nullptr : target, params, &exc);
        if (exc != nullptr)
            luaL_error(L, "zlua: exception in event %s", binding.eventInfo != nullptr ? binding.eventInfo->name : "?");
    }

    static void ReleaseDelegate(Il2CppDelegate* delegate)
    {
        std::unordered_map<Il2CppDelegate*, int>::iterator refIt = s_delegateFunctionRefs.find(delegate);
        if (refIt != s_delegateFunctionRefs.end())
        {
            luaL_unref(LuaEnv::GetState(), LUA_REGISTRYINDEX, refIt->second);
            s_delegateFunctionRefs.erase(refIt);
        }
    }

    static int MutateSubscription(lua_State* L, bool isAdd)
    {
        const int eventId = (int)lua_tointeger(L, lua_upvalueindex(1));
        if (eventId <= 0 || eventId >= (int)s_eventBindings.size())
            return luaL_error(L, "zlua: invalid event subscription");

        const EventBinding& binding = s_eventBindings[(size_t)eventId];
        Il2CppObject* target = nullptr;
        int handlerIndex = 0;
        int argStartIndex = 0;
        if (!TryResolveSubscriptionTarget(L, binding, true, &target, &handlerIndex, &argStartIndex))
            return luaL_error(L, "zlua: invalid event subscription");

        const void* luaFunctionPointer = lua_topointer(L, handlerIndex);
        EventSubscriptionKey key = { target, eventId, luaFunctionPointer };

        if (isAdd)
        {
            Il2CppDelegate* delegate = nullptr;
            std::unordered_map<EventSubscriptionKey, Il2CppDelegate*, EventSubscriptionKeyHash>::iterator cached =
                s_eventHandlerCache.find(key);
            if (cached == s_eventHandlerCache.end())
            {
                delegate = CreateLuaDelegate(L, handlerIndex, binding);
                if (delegate == nullptr)
                    return luaL_error(L, "zlua: event %s handler cannot be bound", binding.eventInfo->name);
                s_eventHandlerCache[key] = delegate;
            }
            else
            {
                delegate = cached->second;
            }

            InvokeAddOrRemove(L, binding, target, delegate, true);
            return 0;
        }

        std::unordered_map<EventSubscriptionKey, Il2CppDelegate*, EventSubscriptionKeyHash>::iterator cached =
            s_eventHandlerCache.find(key);
        if (cached == s_eventHandlerCache.end())
            return luaL_error(L, "zlua: event %s handler was not registered through get", binding.eventInfo->name);

        InvokeAddOrRemove(L, binding, target, cached->second, false);
        ReleaseDelegate(cached->second);
        s_eventHandlerCache.erase(cached);
        return 0;
    }

    static int EventAdd(lua_State* L)
    {
        return MutateSubscription(L, true);
    }

    static int EventRemove(lua_State* L)
    {
        return MutateSubscription(L, false);
    }

    static int EventFire(lua_State* L)
    {
        const int eventId = (int)lua_tointeger(L, lua_upvalueindex(1));
        if (eventId <= 0 || eventId >= (int)s_eventBindings.size())
            return luaL_error(L, "zlua: invalid event fire invocation");

        const EventBinding& binding = s_eventBindings[(size_t)eventId];
        if (binding.raiseMethod == nullptr)
            return luaL_error(L, "zlua: event %s has no raise method", binding.eventInfo->name);

        Il2CppObject* target = nullptr;
        int handlerIndex = 0;
        int argStartIndex = 0;
        if (!TryResolveSubscriptionTarget(L, binding, false, &target, &handlerIndex, &argStartIndex))
            return luaL_error(L, "zlua: invalid event fire invocation");

        void* invokeTarget = binding.isStatic ? nullptr : target;
        if (!binding.isStatic && binding.raiseMethod->klass != nullptr && binding.raiseMethod->klass->byval_arg.valuetype)
            invokeTarget = InstanceTarget::ResolveMethodTarget(L, 1, binding.raiseMethod);

        return MethodBridge::InvokeMethod(L, binding.raiseMethod, invokeTarget, argStartIndex);
    }

    static int StoreClosureRef(lua_State* L, lua_CFunction fn, int eventId)
    {
        lua_pushinteger(L, eventId);
        lua_pushcclosure(L, fn, 1);
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }

    int EventBridge::RegisterBinding(const EventInfo* eventInfo, bool isStatic)
    {
        EventBinding binding = {};
        binding.eventInfo = eventInfo;
        binding.isStatic = isStatic;
        binding.addMethod = eventInfo != nullptr ? eventInfo->add : nullptr;
        binding.removeMethod = eventInfo != nullptr ? eventInfo->remove : nullptr;
        binding.raiseMethod = ResolveRaiseMethod(eventInfo, isStatic);

        if (eventInfo != nullptr && eventInfo->eventType != nullptr)
            binding.delegateClass = il2cpp::vm::Class::FromIl2CppType(eventInfo->eventType);

        if (binding.delegateClass != nullptr)
        {
            il2cpp::vm::Class::Init(binding.delegateClass);
            binding.invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(binding.delegateClass);
            binding.stubKind = ResolveStubKind(binding.invokeMethod);
        }

        const int eventId = s_nextEventId++;
        if ((int)s_eventBindings.size() <= eventId)
            s_eventBindings.resize((size_t)eventId + 1);
        s_eventBindings[(size_t)eventId] = binding;
        return eventId;
    }

    int EventBridge::CreateAddClosureRef(lua_State* L, int eventId)
    {
        return StoreClosureRef(L, EventAdd, eventId);
    }

    int EventBridge::CreateRemoveClosureRef(lua_State* L, int eventId)
    {
        return StoreClosureRef(L, EventRemove, eventId);
    }

    int EventBridge::CreateFireClosureRef(lua_State* L, int eventId)
    {
        if (eventId <= 0 || eventId >= (int)s_eventBindings.size() || s_eventBindings[(size_t)eventId].raiseMethod == nullptr)
            return LUA_NOREF;
        return StoreClosureRef(L, EventFire, eventId);
    }
}
