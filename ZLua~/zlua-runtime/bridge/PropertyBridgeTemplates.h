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

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/TypedMarshal.h"

namespace zlua
{

// ---------------------------------------------------------------------------
// HybridCLR fallback for the direct-methodPointer accessors below.
//
// Those accessors cast methodPointer to the property's concrete native
// signature. A HybridCLR-interpreted accessor has no such entry point, so for
// those we go through invoker_method instead -- which always works and needs no
// generated native2managed bridge.
//
// Reachable two ways: a property declared on a hot-update type, and a virtual
// property on an AOT type overridden by a hot-update subclass. The second only
// shows up in the MethodInfo resolved per call (PropertyBridge::InvokeGetter ->
// MetadataUtil::ResolveInvokeMethod), so the check has to be per call, not at
// accessor-resolution time. Cost when compiled in is one bitfield read; the
// whole path compiles out when ZLUA_ENABLE_HYBRIDCLR is 0.
//
// invoker_method ABI: follow ctx->meta->passByValue exactly as
// MethodBridge::DefaultInvokeLuaMethod does -- a passByValue type occupies the arg
// slot directly, anything else passes a pointer to storage. Reusing that flag
// instead of classifying T here keeps one source of truth; note it covers more
// than managed references (IL2CPP_TYPE_PTR / FNPTR are passByValue too, while
// IntPtr is not), which a per-type trait would be easy to get wrong.
// Return values always come back through a caller-supplied buffer.
// ---------------------------------------------------------------------------

template <typename T>
inline T ZLuaInvokerGet(void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    IL2CPP_ASSERT(sizeof(T) >= ctx->meta->size);
    (void)ctx;
    T value{};
    method->invoker_method(method->methodPointer, method, target, nullptr, &value);
    return value;
}

template <typename T>
inline void ZLuaInvokerSet(void* target, T value, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    // Same invariant DefaultInvokeLuaMethod asserts: passByValue implies pointer-sized.
    IL2CPP_ASSERT(!ctx->meta->passByValue || ctx->meta->size == sizeof(void*));

    void* params[1];
    if (ctx->meta->passByValue)
    {
        params[0] = *reinterpret_cast<void**>(&value);
    }
    else
    {
        params[0] = &value;
    }

    method->invoker_method(method->methodPointer, method, target, params, nullptr);
}

// ---------------------------------------------------------------------------
// ByValue: direct methodPointer; Push(L, T) / Pop(L, idx) -> T
// (primitives, string, IntPtr, pointer, …)
// ---------------------------------------------------------------------------

template <typename T, void (*Push)(lua_State*, T)>
inline void PropertyInstanceGetterByValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    if (ZLuaRequiresGenericInvoke(method))
    {
        Push(L, ZLuaInvokerGet<T>(target, method, ctx));
        return;
    }

    using FnGet = T (*)(void*, const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(target, method);
    Push(L, value);
}

template <typename T, T (*Pop)(lua_State*, int)>
inline void PropertyInstanceSetterByValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(target, Pop(L, valueIdx), method, ctx);
        return;
    }

    using FnSet = void (*)(void*, T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(target, Pop(L, valueIdx), method);
}

template <typename T, void (*Push)(lua_State*, T)>
inline void PropertyStaticGetterByValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    if (ZLuaRequiresGenericInvoke(method))
    {
        Push(L, ZLuaInvokerGet<T>(nullptr, method, ctx));
        return;
    }

    using FnGet = T (*)(const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(method);
    Push(L, value);
}

template <typename T, T (*Pop)(lua_State*, int)>
inline void PropertyStaticSetterByValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(nullptr, Pop(L, valueIdx), method, ctx);
        return;
    }

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
    if (ZLuaRequiresGenericInvoke(method))
    {
        Push(L, ZLuaInvokerGet<T>(target, method, ctx), ctx);
        return;
    }

    using FnGet = T (*)(void*, const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(target, method);
    Push(L, value, ctx);
}

template <typename T, T (*Pop)(lua_State*, int, const PropertyMarshalCtx*)>
inline void PropertyInstanceSetterByValueCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(target, Pop(L, valueIdx, ctx), method, ctx);
        return;
    }

    using FnSet = void (*)(void*, T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(target, Pop(L, valueIdx, ctx), method);
}

template <typename T, void (*Push)(lua_State*, T, const PropertyMarshalCtx*)>
inline void PropertyStaticGetterByValueCtx(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    if (ZLuaRequiresGenericInvoke(method))
    {
        Push(L, ZLuaInvokerGet<T>(nullptr, method, ctx), ctx);
        return;
    }

    using FnGet = T (*)(const MethodInfo*);
    T value = reinterpret_cast<FnGet>(method->methodPointer)(method);
    Push(L, value, ctx);
}

template <typename T, T (*Pop)(lua_State*, int, const PropertyMarshalCtx*)>
inline void PropertyStaticSetterByValueCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(nullptr, Pop(L, valueIdx, ctx), method, ctx);
        return;
    }

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
    T value;
    Pop(L, valueIdx, ctx, &value);
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(target, value, method, ctx);
        return;
    }

    using FnSet = void (*)(void*, T, const MethodInfo*);
    reinterpret_cast<FnSet>(method->methodPointer)(target, value, method);
}

template <typename T, void (*Pop)(lua_State*, int, const PropertyMarshalCtx*, T*)>
inline void PropertyStaticSetterOutParamCtx(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    T value;
    Pop(L, valueIdx, ctx, &value);
    if (ZLuaRequiresGenericInvoke(method))
    {
        ZLuaInvokerSet<T>(nullptr, value, method, ctx);
        return;
    }

    using FnSet = void (*)(T, const MethodInfo*);
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

// for full generic sharing

template <typename T, void (*PushBuf)(lua_State*, T)>
inline void PropertyInstanceGetterInvokerPrimitiveBuf(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    T ret;
    method->invoker_method(method->methodPointer, method, target, nullptr, &ret);
    PushBuf(L, ret);
}

template <typename T, T (*PopBuf)(lua_State*, int)>
inline void PropertyInstanceSetterInvokerPrimitiveBuf(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    T value = PopBuf(L, valueIdx);
    void* params[1] = {&value};
    method->invoker_method(method->methodPointer, method, target, params, nullptr);
}

template <typename T, void (*PushBuf)(lua_State*, T)>
inline void PropertyStaticGetterInvokerPrimitiveBuf(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    T ret;
    method->invoker_method(method->methodPointer, method, nullptr, nullptr, &ret);
    PushBuf(L, ret);
}

template <typename T, T (*PopBuf)(lua_State*, int)>
inline void PropertyStaticSetterInvokerPrimitiveBuf(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    T value = PopBuf(L, valueIdx);
    void* params[1] = {&value};
    method->invoker_method(method->methodPointer, method, nullptr, params, nullptr);
}

// Ref-like (string / object / array / delegate / ptr / fnptr): invoker + TypedMarshal.
// Do not call ctx->getter/setter here — those may be the FGS wrappers themselves.

template <typename T>
inline void PropertyInstanceGetterInvokerRefLike(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    T ret = nullptr;
    method->invoker_method(method->methodPointer, method, target, nullptr, &ret);
    TypedMarshal::PushByType(L, &ret, method->return_type);
}

template <typename T>
inline void PropertyInstanceSetterInvokerRefLike(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    T value = nullptr;
    TypedMarshal::PopByType(L, valueIdx, &value, method->parameters[0]);
    void* params[1] = {&value};
    method->invoker_method(method->methodPointer, method, target, params, nullptr);
}

template <typename T>
inline void PropertyStaticGetterInvokerRefLike(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    T ret = nullptr;
    method->invoker_method(method->methodPointer, method, nullptr, nullptr, &ret);
    TypedMarshal::PushByType(L, &ret, method->return_type);
}

template <typename T>
inline void PropertyStaticSetterInvokerRefLike(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    (void)ctx;
    T value = nullptr;
    TypedMarshal::PopByType(L, valueIdx, &value, method->parameters[0]);
    void* params[1] = {&value};
    method->invoker_method(method->methodPointer, method, nullptr, params, nullptr);
}

} // namespace zlua
