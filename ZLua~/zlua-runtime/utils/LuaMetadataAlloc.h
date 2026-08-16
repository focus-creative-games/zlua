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

namespace zlua
{
class LuaMetadataAlloc
{
  public:
    static void Initialize();
    static void Shutdown();

    static void* Malloc(size_t size);
    static void* Calloc(size_t count, size_t size);

    static void* MallocZeroed(size_t size)
    {
        return Calloc(1, size);
    }

    template <typename T>
    static T* MallocAny()
    {
        return (T*)Malloc(sizeof(T));
    }

    template <typename T>
    static T* MallocAnyZeroed()
    {
        return (T*)Calloc(1, sizeof(T));
    }

    template <typename T>
    static T* CallocArray(size_t count)
    {
        return (T*)Calloc(count, sizeof(T));
    }

    template <typename T>
    static T* NewAny()
    {
        return new (MallocAnyZeroed<T>()) T();
    }
};
} // namespace zlua