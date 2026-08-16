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
