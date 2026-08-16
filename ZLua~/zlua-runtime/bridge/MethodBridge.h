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
        const int32_t minArity = GetMinLuaArity(ctx);
        if (available < minArity)
        {
            if (minArity == ctx->luaArity)
                LuaException::ThrowFormat("zlua: argument mismatch: expected %d argument(s), got %d", minArity, available);
            LuaException::ThrowFormat("zlua: argument mismatch: expected %d..%d argument(s), got %d", minArity, ctx->luaArity, available);
        }
        const MethodInfo* method = MetadataUtil::ResolveInvokeMethod(ctx->method, target, ctx->sealed);
        return ctx->lua2CsInvoker(L, target, argStart, method, ctx);
    }
};
} // namespace zlua
