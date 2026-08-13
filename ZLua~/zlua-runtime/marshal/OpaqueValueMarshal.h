#pragma once

#include "MarshalDefs.h"

namespace zlua
{
#if ZLUA_ARCH_64
typedef uint64_t OpaqueParameterHandleType;
#else
typedef uint32_t OpaqueParameterHandleType;
#endif

class OpaqueParameterScope
{
  public:
    OpaqueParameterScope();
    ~OpaqueParameterScope();

  private:
    size_t _oldStackSize;
};

class OpaqueValueMarshal
{
  public:
    static void Push(lua_State* L, void* valueAddress, const Il2CppType* type);
    static void Pop(lua_State* L, int index, void* valueAddress, const Il2CppType* type);

    static void PushTypedReference(lua_State* L, void* valueAddress);
    static void PopTypedReference(lua_State* L, int index, void* valueAddress);

    static void PushValueFromHandle(lua_State* L, int handleIndex);
    static void SetValueToHandle(lua_State* L, int handleIndex, int valueIndex);
};
} // namespace zlua
