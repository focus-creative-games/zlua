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
    /// Cached Invoke MethodMarshalCtx keyed by delegate klass (avoids GetDelegateInvoke per __call).
    static const MethodMarshalCtx* GetOrCreateInvokeMarshalCtx(lua_State* L, Il2CppClass* delegateClass);
};
} // namespace zlua
