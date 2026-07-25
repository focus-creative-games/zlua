#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
struct MethodMarshalCtx;

class DelegateBridge
{
  public:
    static void Initialize();
    static Il2CppMethodPointer Resolve(Il2CppClass* delegateClass);
    static const MethodMarshalCtx* GetOrCreateMethodMarshalCtx(lua_State* L, const MethodInfo* method);
};
} // namespace zlua
