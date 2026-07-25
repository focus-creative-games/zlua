#pragma once

#include <vector>

#include "../ZLuaCommon.h"
#include "MarshalDefs.h"

namespace zlua
{
class ArrayMarshal
{
  public:
  static bool TryGetConsecutiveTableLength(lua_State* L, int index, int& length);
  static bool TryReadIntSequence(lua_State* L, int index, int expectedCount, std::vector<int32_t>& values);

  static void PushAsArrayObject(lua_State* L, Il2CppArray* array, const MarshalMetaInfo* meta);
  static inline void PushAsArrayObject(lua_State* L, Il2CppArray* array)
  {
      PushAsArrayObject(L, array, nullptr);
  }
  static Il2CppArray* PopFromArrayObject(lua_State* L, int arrayIndex, Il2CppClass* klass);
  static void PushAsBytes(lua_State* L, Il2CppArray* array);
  static Il2CppArray* PopFromBytes(lua_State* L, int arrayIndex, Il2CppClass* klass);

  static void PushAsTable(lua_State* L, Il2CppArray* array);
  static Il2CppArray* PopFromTable(lua_State* L, int arrayIndex, Il2CppClass* klass);

  static Il2CppArray* PopFromArrayObjectOrTable(lua_State* L, int arrayIndex, Il2CppClass* klass);
};
} // namespace zlua
