#pragma once

#include "BridgeDefs.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"

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
        // Require enough slots from argStart; do not require an exact top match.
        // Callers (e.g. valuetype ctors) may push temporaries after the Lua args.
        const int top = lua_gettop(L);
        const int available = top >= argStart ? (top - argStart + 1) : 0;
        if (available < ctx->luaArity)
        {
            LuaException::ThrowFormat("zlua: argument mismatch: expected %d argument(s), got %d", ctx->luaArity, available);
        }
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->method, target, ctx->sealed);
        return ctx->lua2CsInvoker(L, target, argStart, method, ctx);
    }
};
} // namespace zlua
