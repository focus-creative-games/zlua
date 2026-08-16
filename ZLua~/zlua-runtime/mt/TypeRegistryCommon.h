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
#include "../marshal/MarshalDefs.h"

namespace zlua
{
struct TypeBinding;

class TypeRegistryCommon
{
  public:
    static Il2CppObject* GetByObjThis(lua_State* L, int index);
    static void RegisterNativeInstanceMethod(lua_State* L, TypeBinding* binding, const char* name, lua_CFunction fn);
    static int TypeTableToString(lua_State* L);
    static int ObjectInstanceToString(lua_State* L);
    static void WriteCommonTypeFields(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding);
    static void RegisterStaticLiteralFields(lua_State* L, Il2CppClass* klass, int typeTableIndex);
    static void AttachStaticTypeMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction callFn, lua_CFunction extraFn,
                                          const char* extraFieldName, TypeBinding* binding);
    static void AttachReferenceInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding);
    static void AttachByValInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding);
    static void AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding);
};
} // namespace zlua
