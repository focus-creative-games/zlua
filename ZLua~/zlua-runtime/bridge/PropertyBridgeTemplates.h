#pragma once

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"

namespace zlua
{

// ---------------------------------------------------------------------------
// ByValue: direct methodPointer; Push(L, T) / Pop(L, idx) -> T
// (primitives, string, IntPtr, pointer, …)
// ---------------------------------------------------------------------------

template <typename T, void (*Push)(lua_State*, T)>
inline void PropertyInstanceGetterByValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    using FnGet = T (*)(void*, const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(target, method);
    Push(L, value);
}

template <typename T, T (*Pop)(lua_State*, int)>
inline void PropertyInstanceSetterByValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    using FnSet = void (*)(void*, T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(target, Pop(L, valueIdx), method);
}

template <typename T, void (*Push)(lua_State*, T)>
inline void PropertyStaticGetterByValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    using FnGet = T (*)(const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(method);
    Push(L, value);
}

template <typename T, T (*Pop)(lua_State*, int)>
inline void PropertyStaticSetterByValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    using FnSet = void (*)(T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(Pop(L, valueIdx), method);
}

// ---------------------------------------------------------------------------
// ByValueCtx: same ABI, but Push/Pop need PropertyMarshalCtx
// (object, Unity Vector2/3/4 by-value return)
// ---------------------------------------------------------------------------

template <typename T, void (*Push)(lua_State*, T, const PropertyMarshalCtx*)>
inline void PropertyInstanceGetterByValueCtx(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    using FnGet = T (*)(void*, const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(target, method);
    Push(L, value, ctx);
}

template <typename T, T (*Pop)(lua_State*, int, const PropertyMarshalCtx*)>
inline void PropertyInstanceSetterByValueCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    using FnSet = void (*)(void*, T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(target, Pop(L, valueIdx, ctx), method);
}

template <typename T, void (*Push)(lua_State*, T, const PropertyMarshalCtx*)>
inline void PropertyStaticGetterByValueCtx(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    using FnGet = T (*)(const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(method);
    Push(L, value, ctx);
}

template <typename T, T (*Pop)(lua_State*, int, const PropertyMarshalCtx*)>
inline void PropertyStaticSetterByValueCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    using FnSet = void (*)(T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(Pop(L, valueIdx, ctx), method);
}

// ---------------------------------------------------------------------------
// OutParamCtx: Pop into a stack local then pass by value (Unity Vector2/3/4 set).
// Avoids return-by-value ABI copies from Pop helpers.
// ---------------------------------------------------------------------------

template <typename T, void (*Pop)(lua_State*, int, const PropertyMarshalCtx*, T*)>
inline void PropertyInstanceSetterOutParamCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    using FnSet = void (*)(void*, T, const MethodInfo*);
    T value;
    Pop(L, valueIdx, ctx, &value);
    reinterpret_cast<FnSet>(method->methodPointer)(target, value, method);
}

template <typename T, void (*Pop)(lua_State*, int, const PropertyMarshalCtx*, T*)>
inline void PropertyStaticSetterOutParamCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    using FnSet = void (*)(T, const MethodInfo*);
    T value;
    Pop(L, valueIdx, ctx, &value);
    reinterpret_cast<FnSet>(method->methodPointer)(value, method);
}

// ---------------------------------------------------------------------------
// InvokerBuf: alloca + invoker_method; Push/Pop operate on raw buffer
// (ValueType, Nullable, byref Opaque)
// ---------------------------------------------------------------------------

template <void (*PushBuf)(lua_State*, void*, const PropertyMarshalCtx*)>
inline void PropertyInstanceGetterInvokerBuf(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    void* ret = alloca(ctx->meta->size);
    method->invoker_method(method->methodPointer, method, target, nullptr, ret);
    PushBuf(L, ret, ctx);
}

template <void (*PopBuf)(lua_State*, int, void*, const MethodInfo*, const PropertyMarshalCtx*)>
inline void PropertyInstanceSetterInvokerBuf(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    void* value = alloca(ctx->meta->size);
    PopBuf(L, valueIdx, value, method, ctx);
    void* params[1] = {value};
    method->invoker_method(method->methodPointer, method, target, params, nullptr);
}

template <void (*PushBuf)(lua_State*, void*, const PropertyMarshalCtx*)>
inline void PropertyStaticGetterInvokerBuf(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    void* ret = alloca(ctx->meta->size);
    method->invoker_method(method->methodPointer, method, nullptr, nullptr, ret);
    PushBuf(L, ret, ctx);
}

template <void (*PopBuf)(lua_State*, int, void*, const MethodInfo*, const PropertyMarshalCtx*)>
inline void PropertyStaticSetterInvokerBuf(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    void* value = alloca(ctx->meta->size);
    PopBuf(L, valueIdx, value, method, ctx);
    void* params[1] = {value};
    method->invoker_method(method->methodPointer, method, nullptr, params, nullptr);
}

} // namespace zlua
