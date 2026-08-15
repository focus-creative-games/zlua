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

    static inline void InvokeGetter(lua_State* L, void* target, const PropertyMarshalCtx* ctx)
    {
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->property->get, target, ctx->getterSealed);
        ctx->getter(L, target, method, ctx);
    }

    static inline void InvokeSetter(lua_State* L, void* target, int valueIdx, const PropertyMarshalCtx* ctx)
    {
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->property->set, target, ctx->setterSealed);
        ctx->setter(L, target, valueIdx, method, ctx);
    }
};
} // namespace zlua
