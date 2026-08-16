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

#include "MarshalDefs.h"

namespace zlua
{

class CompositeMarshal
{
  public:
    static void PopTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void PushTable(lua_State* L, void* address, const MarshalMetaInfo* meta);
    static void PopUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void PushUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta);

    static void Lua2CSMarshalTable(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void CS2LuaMarshalTable(lua_State* L, void* address, const MarshalMetaInfo* meta);
    static void Lua2CSMarshalUnpacked(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta);
    static void CS2LuaMarshalUnpacked(lua_State* L, void* address, const MarshalMetaInfo* meta);
};

} // namespace zlua
