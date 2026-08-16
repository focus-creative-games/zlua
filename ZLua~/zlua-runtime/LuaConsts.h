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

namespace zlua
{
struct LuaConsts
{
    // Standard Lua metamethod keys
    static constexpr const char* MetaIndex = "__index";
    static constexpr const char* MetaNewIndex = "__newindex";
    static constexpr const char* MetaCall = "__call";
    static constexpr const char* MetaGc = "__gc";
    static constexpr const char* MetaToString = "__tostring";
    static constexpr const char* MetaLen = "__len";
    static constexpr const char* MetaMode = "__mode";

    // Weak-table __mode values
    static constexpr const char* WeakModeValue = "v";

    // ZLua type-table keys
    static constexpr const char* FullName = "__fullname";
    static constexpr const char* Klass = "__klass";
    static constexpr const char* ByValInstanceMt = "__byval_instance_mt";
    static constexpr const char* ByObjInstanceMt = "__byobj_instance_mt";
    static constexpr const char* Struct = "__struct";
    static constexpr const char* Enum = "__enum";
    static constexpr const char* Type = "__type";
    static constexpr const char* Nullable = "__nullable";
    static constexpr const char* UdKind = "__zlua_ud_kind";

    // ZLua userdata kind values
    static constexpr const char* UdKindByVal = "byval";
    static constexpr const char* UdKindByObj = "byobj";
    static constexpr const char* UdKindOpaqueParameter = "opaque_parameter";

    // ZLua property/event table keys
    static constexpr const char* Get = "get";
    static constexpr const char* Set = "set";
    static constexpr const char* Fire = "fire";

    // ZLua static metatable extra field
    static constexpr const char* Default = "_default";
};
} // namespace zlua
