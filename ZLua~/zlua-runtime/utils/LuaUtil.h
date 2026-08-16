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

#include <string>

namespace zlua
{
class LuaUtil
{
  public:
    // Lua number is integer-valued (integer tag, or number with no fractional part).
    static bool IsStrictLuaInteger(lua_State* L, int index, lua_Integer& value);
    static int PushRef(lua_State* L, int refIndex);
    static int PCallClosureRefAt(lua_State* L, int closureRef, const int* argStackIndices, int argCount, int resultCount);

    static void PCall(lua_State* L, int nargs, int nresults, int errfunc);
    /// Convert the Lua error object at index to a non-empty message string.
    static std::string FormatErrorObject(lua_State* L, int index);

    static int ToLuaRef(lua_State* L)
    {
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }

    static int ToLuaRef(lua_State* L, int index)
    {
        lua_pushvalue(L, index);
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }
};
} // namespace zlua
