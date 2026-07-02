#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class DelegateBridge
{
  public:
    static void Initialize();
    static Il2CppMethodPointer Resolve(Il2CppClass* delegateClass);
};
} // namespace zlua
