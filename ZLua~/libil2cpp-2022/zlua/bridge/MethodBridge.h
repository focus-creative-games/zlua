#pragma once

#include "BridgeDefs.h"
#include "../utils/MetadataUtil.h"

namespace zlua
{
class MethodBridge
{
  public:
    static void Initialize();
    static int DefaultInvokeLuaMethod(lua_State* L, void* target, int argStart, const MethodInfo* method, const MethodMarshalCtx* ctx);
    static FnLua2CsInvoker ResolveMethodInvoker(const MethodInfo* method);

    static inline int InvokeLua2Cs(lua_State* L, void* target, int argStart, const MethodMarshalCtx* ctx)
    {
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->method, target, ctx->sealed);
        return ctx->lua2CsInvoker(L, target, argStart, method, ctx);
    }
};
} // namespace zlua
