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
#include "../utils/MetadataUtil.h"

namespace zlua
{

struct PropertyAccessor
{
    FnPropertyGetter getter;
    FnPropertySetter setter;
};

class PropertyBridge
{
public:
    static void Initialize();
    static PropertyAccessor ResolvePropertyAccessor(const PropertyInfo* property, bool isStatic);

    // Single choke point for every property access, so HybridCLR's lazy
    // methodPointer/invoker_method init happens once here and covers all accessor
    // shapes -- including the invoker_method-based ones that never cast the
    // pointer. The accessor is cached per property but the MethodInfo is resolved
    // per call (virtual dispatch), so a hot-update override only becomes visible
    // at this point. No-op without HybridCLR, and for AOT methods.

    static inline void InvokeGetter(lua_State* L, void* target, const PropertyMarshalCtx* ctx)
    {
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->property->get, target, ctx->getterSealed);
        ZLuaEnsureInvokable(method);
        ctx->getter(L, target, method, ctx);
    }

    static inline void InvokeSetter(lua_State* L, void* target, int valueIdx, const PropertyMarshalCtx* ctx)
    {
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->property->set, target, ctx->setterSealed);
        ZLuaEnsureInvokable(method);
        ctx->setter(L, target, valueIdx, method, ctx);
    }
};
} // namespace zlua
