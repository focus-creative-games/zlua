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
#include "MetaBinding.h"

#if ZLUA_FAST_METATABLE

namespace zlua
{

class FastMetatable
{
  public:
    static FastMemberCtx* AllocCtx(TypeBinding* binding, FastInstanceKind kind, const MetaInfo& info);

    static void FillMemberTables(lua_State* L, TypeBinding* binding, const NameMetaMap* map, FastInstanceKind kind, int memberTableIndex,
                                 int setterTableIndex);

    static void SealMetatable(lua_State* L, int mtIndex);

    /* Late-bound method alias (zlua.register_method): rawset closure into sealed __index table. */
    static void RawSetMethodOnIndexTable(lua_State* L, Il2CppClass* klass, bool isStatic, bool isByVal, const char* aliasName, int closureStackIndex);
};

} // namespace zlua

#endif /* ZLUA_FAST_METATABLE */
