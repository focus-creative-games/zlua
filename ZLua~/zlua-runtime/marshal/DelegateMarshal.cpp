#include "DelegateMarshal.h"

#include "../bridge/DelegateBridge.h"
#include "../lvm/LuaEnv.h"
#include "../utils/Collection.h"
#include "../utils/LuaException.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaUtil.h"
#include "../utils/MetadataUtil.h"
#include "MarshalDefs.h"
#include "ObjectRegistry.h"
#include "OpaqueValueMarshal.h"

#include <cstring>

#include "vm/Field.h"
#include "vm/Method.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Type.h"

namespace zlua
{
// Layout must match GenericMethod.cpp for the active Unity version.
#if ZLUA_UNITY_VERSION < 20220000
// Unity 2021: SharedGenericMethodInfo
struct FgsMethodInfoCopy : MethodInfo
{
    Il2CppMethodPointer virtualCallMethodPointer;
};
#else
// Unity 2022+: FullySharedGenericMethodInfo
struct FgsMethodInfoCopy : MethodInfo
{
    Il2CppMethodPointer rawVirtualMethodPointer;
    Il2CppMethodPointer rawDirectMethodPointer;
    InvokerMethod rawInvokerMethod;
};
#endif

// Per-delegateClass construction recipe (computed once).
struct LuaDelegateCtorCache
{
    const MethodInfo* ctorMethod;
    const MethodInfo* originalInvokeMethod; // real Invoke (marshal)
    const MethodInfo* finalInvokeImpl;      // FGS: invoker-swapped copy; else == invokeMethod
    const MethodMarshalCtx* marshalCtx;
    // Closed-delegate invoke_impl: typed DelegateBridge, or FGS unresolved stub →
    // LuaDelegateInvoke.
    Il2CppMethodPointer invokeImpl;
};

AppendOnlyRawPointerHashMap<Il2CppClass, const LuaDelegateCtorCache*> s_luaDelegateCtorByClass;

// Like HybridCLR InterpreterInvoke (Unity 2021+ InvokerMethod with __ret).
// Not InterpreterDelegateInvoke — ZLua lua→delegate never carries a multicast
// list.
static void LuaDelegateInvoke(Il2CppMethodPointer, const MethodInfo* method, void* __this, void** __args, void* __ret)
{
    LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(__this);
    Il2CppObject* targetObj = reinterpret_cast<Il2CppObject*>(luaMethod);
    lua_State* L = LuaEnv::GetState();
    OpaqueParameterScope opaqueScope;
    const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
    LuaStackGuard guard(L);
    int errfunc = LuaEnv::PushErrorHandler();
    LuaUtil::PushRef(L, luaMethod->funcRef);

    const uint8_t paramCount = method->parameters_count;
    for (uint8_t i = 0; i < paramCount; i++)
        ctx->paramsMeta[i]->cs2luaWriter(L, __args[i], ctx->paramsMeta[i]);

    const bool hasRet = method->return_type->type != IL2CPP_TYPE_VOID;
    LuaUtil::PCall(L, paramCount, hasRet ? 1 : 0, errfunc);
    if (hasRet)
        ctx->retMeta->lua2csWriter(L, -1, __ret, ctx->retMeta);
}

bool IsValueType(const Il2CppType* type)
{
    switch (type->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
    case IL2CPP_TYPE_I1:
    case IL2CPP_TYPE_U1:
    case IL2CPP_TYPE_CHAR:
    case IL2CPP_TYPE_I2:
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_I4:
    case IL2CPP_TYPE_U4:
    case IL2CPP_TYPE_R4:
    case IL2CPP_TYPE_I8:
    case IL2CPP_TYPE_U8:
    case IL2CPP_TYPE_R8:
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U:
    case IL2CPP_TYPE_TYPEDBYREF:
    case IL2CPP_TYPE_VALUETYPE:
        return true;
    case IL2CPP_TYPE_GENERICINST:
        return type->data.generic_class->type->type == IL2CPP_TYPE_VALUETYPE;
    default:
        return false;
    }
}

void FullGenericSharingReflectionInvoke(Il2CppMethodPointer, const MethodInfo* method, void* __this, void** __args, void* __ret)
{
    LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(__this);
    Il2CppObject* targetObj = reinterpret_cast<Il2CppObject*>(luaMethod);
    lua_State* L = LuaEnv::GetState();
    OpaqueParameterScope opaqueScope;
    const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
    LuaStackGuard guard(L);
    int errfunc = LuaEnv::PushErrorHandler();
    LuaUtil::PushRef(L, luaMethod->funcRef);

    IL2CPP_ASSERT(method->is_inflated);
    const Il2CppGenericContext* genericContext = &method->genericMethod->context;
    const uint8_t paramCount = method->parameters_count;
    for (uint8_t i = 0; i < paramCount; i++)
    {
        {
            const Il2CppType* paramType = method->parameters[i];
            bool passByVal = paramType->byref || !IsValueType(paramType);
            if (passByVal)
            {
                ctx->paramsMeta[i]->cs2luaWriter(L, &__args[i], ctx->paramsMeta[i]);
            }
            else
            {
                ctx->paramsMeta[i]->cs2luaWriter(L, __args[i], ctx->paramsMeta[i]);
            }
        }
    }

    const bool hasRet = method->return_type->type != IL2CPP_TYPE_VOID;
    LuaUtil::PCall(L, paramCount, hasRet ? 1 : 0, errfunc);
    if (hasRet)
        ctx->retMeta->lua2csWriter(L, -1, __ret, ctx->retMeta);
}

static const MethodInfo* CreateFgsLuaDelegateInvokeMethod(const MethodInfo* invokeMethod)
{
    auto* src = reinterpret_cast<const FgsMethodInfoCopy*>(invokeMethod);
    auto* copy = LuaMetadataAlloc::MallocAnyZeroed<FgsMethodInfoCopy>();
    std::memcpy(copy, src, sizeof(FgsMethodInfoCopy));
    copy->invoker_method = FullGenericSharingReflectionInvoke;
    // copy->flags |= METHOD_ATTRIBUTE_STATIC;
    return copy;
}

const LuaDelegateCtorCache* GetOrCreateLuaDelegateCtor(lua_State* L, Il2CppClass* delegateClass)
{
    auto it = s_luaDelegateCtorByClass.find(delegateClass);
    if (it != s_luaDelegateCtorByClass.end())
        return it->second;

    auto* cache = LuaMetadataAlloc::MallocAnyZeroed<LuaDelegateCtorCache>();
    const MethodInfo* ctor = il2cpp::vm::Class::GetMethodFromName(delegateClass, ".ctor", 2);
    cache->ctorMethod = ctor;
    cache->originalInvokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
    cache->marshalCtx = DelegateBridge::GetOrCreateMethodMarshalCtx(L, cache->originalInvokeMethod);
    if (il2cpp::vm::Method::HasFullGenericSharingSignature(cache->originalInvokeMethod))
    {
        // FGS: typed bridge has the wrong calling convention. Keep unresolved stubs
        // on a MethodInfo copy whose invoker is LuaDelegateInvoke; install that
        // stub as invoke_impl.
        cache->finalInvokeImpl = CreateFgsLuaDelegateInvokeMethod(cache->originalInvokeMethod);
    }
    else
    {
        cache->finalInvokeImpl = cache->originalInvokeMethod;
    }
    cache->invokeImpl = DelegateBridge::Resolve(delegateClass);
    if (cache->invokeImpl == nullptr)
        LuaException::ThrowFormat("zlua: delegate bridge resolve failed, class:%s", MetadataUtil::GetTypeFullName(delegateClass));
    s_luaDelegateCtorByClass.insert({delegateClass, cache});
    return cache;
}

void DelegateMarshal::Push(lua_State* L, Il2CppDelegate* delegate, int metatableRefIndex)
{
    Il2CppObject* target = delegate->target;
    if (target != nullptr && target->klass == MetadataUtil::GetLuaMethodClass())
    {
        LuaMethod* luaMethod = (LuaMethod*)target;
        LuaUtil::PushRef(L, luaMethod->funcRef);
        return;
    }
    Il2CppObject* obj = reinterpret_cast<Il2CppObject*>(delegate);
    ObjectRegistry::Push(L, obj, obj->klass, metatableRefIndex);
}

inline void ConstructDelegate(const MethodInfo* ctor, Il2CppDelegate* delegate, Il2CppObject* target, Il2CppMethodPointer methodPointer, const MethodInfo* method)
{

    void* ctorArgs[2] = {target, (void*)&method};
    ctor->invoker_method(ctor->methodPointer, ctor, delegate, ctorArgs, NULL);
    // il2cpp::gc::WriteBarrier::GenericStore((void**)&delegate->target, target);
    // il2cpp::gc::WriteBarrier::GenericStore((void**)&delegate->invoke_impl_this,
    // target); delegate->method = method; delegate->invoke_impl = methodPointer;
}

Il2CppDelegate* DelegateMarshal::CreateFromFuncRef(lua_State* L, Il2CppClass* delegateClass, int funcRef)
{
    const LuaDelegateCtorCache* ctor = GetOrCreateLuaDelegateCtor(L, delegateClass);

    Il2CppClass* luaMethodClass = MetadataUtil::GetLuaMethodClass();
    LuaMethod* target = reinterpret_cast<LuaMethod*>(il2cpp::vm::Object::New(luaMethodClass));
    target->disposed = false;
    target->L = L;
    target->funcRef = funcRef;
    target->methodMarshalCtx = ctor->marshalCtx;

    Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(il2cpp::vm::Object::New(delegateClass));
    if (il2cpp::vm::Method::HasFullGenericSharingSignature(ctor->originalInvokeMethod))
    {
        ConstructDelegate(ctor->ctorMethod, delegate, reinterpret_cast<Il2CppObject*>(target), ctor->invokeImpl, ctor->finalInvokeImpl);
    }
    else
    {
        il2cpp::vm::Type::ConstructClosedDelegate(delegate, reinterpret_cast<Il2CppObject*>(target), ctor->invokeImpl, ctor->finalInvokeImpl);
    }
    return delegate;
}

Il2CppDelegate* DelegateMarshal::Pop(lua_State* L, int delegateIndex, Il2CppClass* delegateClass)
{
    Il2CppClass* luaMethodClass = MetadataUtil::GetLuaMethodClass();
    int type = lua_type(L, delegateIndex);
    if (type == LUA_TFUNCTION)
    {
        int funcRef = LuaUtil::ToLuaRef(L, delegateIndex);
        return CreateFromFuncRef(L, delegateClass, funcRef);
    }
    else if (type == LUA_TUSERDATA)
    {
        Il2CppObject* obj = ObjectRegistry::Pop(L, delegateIndex);
        if (obj == nullptr || obj->klass != delegateClass)
        {
            LuaException::Throw("zlua: argument mismatch: expected delegate");
        }
        return (Il2CppDelegate*)obj;
    }
    LuaException::Throw("zlua: argument mismatch: expected delegate");
    return nullptr;
}
} // namespace zlua
