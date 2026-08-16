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

#include "../ZLuaCommon.h"
#include "ObjectRegistry.h"
#include "MarshalDefs.h"

namespace zlua
{
class ObjectMarshal
{
  public:
    static void Push(lua_State* L, Il2CppObject* obj, const MarshalMetaInfo* meta);
    static void Push(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass);
    static void Push(lua_State* L, Il2CppObject* obj);
    static Il2CppObject* Pop(lua_State* L, int objIndex, Il2CppClass* klass);

    static inline Il2CppObject* PopByObjThis(lua_State* L, int objIndex)
    {
        IL2CPP_ASSERT(lua_type(L, objIndex) == LUA_TUSERDATA);
        return ObjectRegistry::PopThis(L, objIndex);
    }
};
} // namespace zlua
