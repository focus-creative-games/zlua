#include "StringMarshal.h"

#include "ObjectRegistry.h"
#include "../utils/LuaException.h"

#include "vm/String.h"
#include "utils/utf8-cpp/source/utf8/unchecked.h"

namespace zlua
{

static char* s_utf8Scratch = nullptr;
static int32_t s_utf8ScratchCap = 0;

static Il2CppChar* s_utf16Scratch = nullptr;
static int32_t s_utf16ScratchCap = 0;

static inline void EnsureUtf8Scratch(int32_t need)
{
    if (need <= s_utf8ScratchCap)
        return;

    delete[] s_utf8Scratch;
    s_utf8Scratch = new char[need];
    s_utf8ScratchCap = need;
}

static inline void EnsureUtf16Scratch(int32_t need)
{
    if (need <= s_utf16ScratchCap)
        return;

    delete[] s_utf16Scratch;
    s_utf16Scratch = new Il2CppChar[need];
    s_utf16ScratchCap = need;
}

// Compresses UTF-16 → UTF-8 into s_utf8Scratch. Returns byte length written.
static size_t ConvertToUtf8(Il2CppString* str)
{
    const int32_t len = str->length;
    EnsureUtf8Scratch(len * 3);

    const Il2CppChar* begin = str->chars;
    char* end = utf8::unchecked::utf16to8(begin, begin + len, s_utf8Scratch);
    return static_cast<size_t>(end - s_utf8Scratch);
}

// On success, s_utf8Scratch holds len ASCII bytes. On failure, content is undefined.
static inline bool TryAllAsciiChars(const Il2CppString* str)
{
    const int32_t len = str->length;
    EnsureUtf8Scratch(len);

    const Il2CppChar* chars = str->chars;
    for (int32_t i = 0; i < len; i++)
    {
        if (chars[i] > 0x7F)
            return false;
        s_utf8Scratch[i] = static_cast<char>(chars[i]);
    }
    return true;
}

// Expands UTF-8 → UTF-16 into s_utf16Scratch. Returns UTF-16 length written.
// Matches StringUtils::Utf8ToUtf16: invalid UTF-8 yields length 0.
static int32_t ConvertToUtf16(const char* utf8, size_t len)
{
    // Worst case: all ASCII → 1 UTF-16 unit per byte.
    EnsureUtf16Scratch(static_cast<int32_t>(len));

    Il2CppChar* end = utf8::unchecked::utf8to16(utf8, utf8 + len, s_utf16Scratch);
    return static_cast<int32_t>(end - s_utf16Scratch);
}

// On success, s_utf16Scratch holds len UTF-16 units. On failure, content is undefined.
static inline bool TryAllAsciiBytes(const char* utf8, size_t len)
{
    EnsureUtf16Scratch(static_cast<int32_t>(len));

    for (size_t i = 0; i < len; i++)
    {
        if (static_cast<uint8_t>(utf8[i]) > 0x7F)
            return false;
        s_utf16Scratch[i] = static_cast<Il2CppChar>(static_cast<uint8_t>(utf8[i]));
    }
    return true;
}

void StringMarshal::Push(lua_State* L, Il2CppString* str)
{
    if (str == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    if (TryAllAsciiChars(str))
    {
        lua_pushlstring(L, s_utf8Scratch, static_cast<size_t>(str->length));
        return;
    }

    size_t nbytes = ConvertToUtf8(str);
    lua_pushlstring(L, s_utf8Scratch, nbytes);
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

        if (TryAllAsciiBytes(str, len))
            return il2cpp::vm::String::NewUtf16(s_utf16Scratch, static_cast<int32_t>(len));

        int32_t utf16Len = ConvertToUtf16(str, len);
        return il2cpp::vm::String::NewUtf16(s_utf16Scratch, utf16Len);
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
    LuaException::ThrowFormat("zlua.string expects string or nil, got: %s", lua_typename(L, lua_type(L, idx)));
    return nullptr;
}
} // namespace zlua
