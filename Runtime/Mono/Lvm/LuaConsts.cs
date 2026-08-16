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

namespace ZLua
{
    /// <summary>
    /// Mirrors <c>zlua/LuaConsts.h</c>.
    /// </summary>
    public static class LuaConsts
    {
        public const int LuaMultiRet = -1;
        public const int LuaiMaxStack = 1_000_000;
        // Lua 5.1 / LuaJIT: fixed pseudo-indices. 5.3/5.4: (-LUAI_MAXSTACK - 1000). 5.5: (-(INT_MAX/2 + 1000)).
#if ZLUA_LUA_5_1 || ZLUA_USE_LUAJIT
        public const int LuaRegistryIndex = -10000;
        public const int LuaEnvironIndex = -10001;
        public const int LuaGlobalsIndex = -10002;
#elif ZLUA_LUA_5_5
        public const int LuaRegistryIndex = unchecked((int)(-(int.MaxValue / 2 + 1000)));
#else
        public const int LuaRegistryIndex = -LuaiMaxStack - 1000;
#endif
        public const int LuaNoRef = -1;
        public const int LuaRefNil = -2;

        public const string MetaIndex = "__index";
        public const string MetaNewIndex = "__newindex";
        public const string MetaCall = "__call";
        public const string MetaGc = "__gc";
        public const string MetaToString = "__tostring";
        public const string MetaLen = "__len";
        public const string MetaMode = "__mode";

        public const string WeakModeValue = "v";

        public const string FullName = "__fullname";
        public const string Klass = "__klass";
        public const string ByValInstanceMt = "__byval_instance_mt";
        public const string ByObjInstanceMt = "__byobj_instance_mt";
        public const string Struct = "__struct";
        public const string Enum = "__enum";
        public const string Type = "__type";
        public const string Nullable = "__nullable";
        public const string UdKind = "__zlua_ud_kind";

        public const string UdKindByVal = "byval";
        public const string UdKindByObj = "byobj";
        public const string UdKindOpaqueParameter = "opaque_parameter";

        public const string Get = "get";
        public const string Set = "set";
        public const string Fire = "fire";
        public const string Default = "_default";
    }
}
