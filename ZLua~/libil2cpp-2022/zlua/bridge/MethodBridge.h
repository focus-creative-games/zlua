#pragma once

#include "BridgeDefs.h"

namespace zlua
{
  class MethodBridge
  {
    public:
      static void Initialize();
      static int DefaultInvokeLuaMethod(lua_State* L, void* target, int argStart, const MethodMarshalCtx* ctx);
      static FnLua2CsInvoker ResolveMethodInvoker(const MethodInfo* method);
  };
} // namespace zlua