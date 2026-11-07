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

#include "StringMarshal.h"

#include "ObjectRegistry.h"
#include "../utils/LuaException.h"

#include "vm/String.h"
#include <climits>
#include <cstring>
#include <memory>
#include <utility>
// utf8::* comes from Il2CppCompatible.h (via ZLuaCommon.h) using ZLUA_UNITY_VERSION.

namespace zlua
{

// Domain-thread-only, like the Lua state. Retain at most 64 KiB per encoding.
static constexpr size_t kScratchLimitBytes = 64 * 1024;
static char* s_utf8Scratch = nullptr;
static size_t s_utf8ScratchCap = 0;
static Il2CppChar* s_utf16Scratch = nullptr;
static size_t s_utf16ScratchCap = 0;

// Borrows the shared cache for small conversions and owns temporary storage
// for large ones. Capacities and preserved lengths are measured in T elements.
template <typename T>
class ScratchBuffer
{
public:
    ScratchBuffer(T*& cachedData, size_t& cachedCapacity) : cachedData_(cachedData), cachedCapacity_(cachedCapacity) {}

    T* EnsureCapacity(size_t requiredCapacity, size_t preservedLength = 0)
    {
        T* currentData = temporaryData_ ? temporaryData_.get() : cachedData_;
        size_t currentCapacity = temporaryData_ ? temporaryCapacity_ : cachedCapacity_;
        if (requiredCapacity <= currentCapacity)
            return currentData;

        const size_t cacheLimit = kScratchLimitBytes / sizeof(T);
        size_t newCapacity = requiredCapacity;
        if (requiredCapacity <= cacheLimit)
        {
            newCapacity = cachedCapacity_ == 0 ? 256 / sizeof(T) : cachedCapacity_;
            while (newCapacity < requiredCapacity)
                newCapacity *= 2;
        }

        // Allocate and copy before replacing the old storage, so allocation
        // failure leaves both the cached data and any converted prefix intact.
        std::unique_ptr<T[]> newData(new T[newCapacity]);
        if (preservedLength != 0)
            std::memcpy(newData.get(), currentData, preservedLength * sizeof(T));

        if (requiredCapacity > cacheLimit)
        {
            temporaryData_ = std::move(newData);
            temporaryCapacity_ = newCapacity;
            return temporaryData_.get();
        }

        delete[] cachedData_;
        cachedData_ = newData.release();
        cachedCapacity_ = newCapacity;
        return cachedData_;
    }

private:
    T*& cachedData_;
    size_t& cachedCapacity_;
    std::unique_ptr<T[]> temporaryData_;
    size_t temporaryCapacity_ = 0;
};

void StringMarshal::ClearScratch()
{
    delete[] s_utf8Scratch;
    s_utf8Scratch = nullptr;
    s_utf8ScratchCap = 0;
    delete[] s_utf16Scratch;
    s_utf16Scratch = nullptr;
    s_utf16ScratchCap = 0;
}

void StringMarshal::Push(lua_State* L, Il2CppString* str)
{
    if (str == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    const size_t len = static_cast<size_t>(str->length);
    if (len == 0)
    {
        lua_pushliteral(L, "");
        return;
    }
    ScratchBuffer<char> buffer(s_utf8Scratch, s_utf8ScratchCap);
    char* output = buffer.EnsureCapacity(len);
    size_t asciiLength = 0;
    while (asciiLength < len && str->chars[asciiLength] <= 0x7F)
    {
        output[asciiLength] = static_cast<char>(str->chars[asciiLength]);
        ++asciiLength;
    }
    if (asciiLength == len)
    {
        lua_pushlstring(L, output, len);
        return;
    }

    // Keep the converted ASCII prefix. Each remaining UTF-16 unit needs at
    // most three UTF-8 bytes, including unpaired surrogates.
    const size_t remainingLength = len - asciiLength;
    if (remainingLength > (SIZE_MAX - asciiLength) / 3)
        LuaException::Throw("zlua: string is too long to marshal to UTF-8");
    const size_t requiredCapacity = asciiLength + remainingLength * 3;
    output = buffer.EnsureCapacity(requiredCapacity, asciiLength);
    char* end = utf8::unchecked::utf16to8(str->chars + asciiLength, str->chars + len, output + asciiLength);
    lua_pushlstring(L, output, static_cast<size_t>(end - output));
}

Il2CppString* StringMarshal::Pop(lua_State* L, int idx)
{
    int type = lua_type(L, idx);
    if (type == LUA_TSTRING)
    {
        size_t len = 0;
        const char* str = lua_tolstring(L, idx, &len);
        if (len == 0)
        {
            return il2cpp::vm::String::Empty();
        }
        if (len > static_cast<size_t>(INT32_MAX))
            LuaException::Throw("zlua: string is too long to marshal to System.String");

        ScratchBuffer<Il2CppChar> buffer(s_utf16Scratch, s_utf16ScratchCap);
        Il2CppChar* output = buffer.EnsureCapacity(len);
        size_t asciiLength = 0;
        while (asciiLength < len && static_cast<uint8_t>(str[asciiLength]) <= 0x7F)
        {
            output[asciiLength] = static_cast<Il2CppChar>(static_cast<uint8_t>(str[asciiLength]));
            ++asciiLength;
        }
        if (asciiLength == len)
            return il2cpp::vm::String::NewUtf16(output, static_cast<int32_t>(len));

        // The ASCII prefix is already valid and converted. Validate the suffix
        // before unchecked decoding; invalid UTF-8 makes the whole string empty.
        if (!utf8::is_valid(str + asciiLength, str + len))
            return il2cpp::vm::String::Empty();

        Il2CppChar* end = utf8::unchecked::utf8to16(str + asciiLength, str + len, output + asciiLength);
        return il2cpp::vm::String::NewUtf16(output, static_cast<int32_t>(end - output));
    }
    if (type == LUA_TUSERDATA)
    {
        Il2CppObject* obj = ObjectRegistry::Pop(L, idx);
        IL2CPP_ASSERT(obj != nullptr);
        Il2CppClass* klass = obj->klass;
        if (klass->byval_arg.type != IL2CPP_TYPE_STRING)
        {
            LuaException::ThrowFormat("zlua argument mismatch: object type:%s.%s is not of type: System.String", klass->namespaze, klass->name);
        }
        return (Il2CppString*)obj;
    }
    if (type == LUA_TNIL)
    {
        return nullptr;
    }
    LuaException::ThrowFormat("zlua argument mismatch: expected string or nil, got: %s", lua_typename(L, lua_type(L, idx)));
    return nullptr;
}
} // namespace zlua
