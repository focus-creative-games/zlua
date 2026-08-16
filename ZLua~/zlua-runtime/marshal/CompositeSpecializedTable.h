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

struct CompositeSpecializedEntry
{
    const char* assemblyName;
    const char* typeFullName;
    LuaMarshalType marshalType;
    FnMarshalLua2Cs lua2csWriter;
    FnMarshalCs2Lua cs2luaWriter;
    uint16_t stackSlots;
};

class CompositeSpecializedTable
{
  public:
    static void Clear();
    static void RegisterEntries(const CompositeSpecializedEntry* entries, size_t count);

    static bool TryGet(
        Il2CppClass* klass,
        LuaMarshalType marshalType,
        FnMarshalLua2Cs* outLua2Cs,
        FnMarshalCs2Lua* outCs2Lua,
        uint16_t* outStackSlots);
};

} // namespace zlua
