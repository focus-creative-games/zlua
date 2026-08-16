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

#include "LuaException.h"

#include "il2cpp-api-types.h"
#include "vm/Exception.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Type.h"

#include "MetadataUtil.h"
#include "../lvm/LuaEnv.h"

#include "utils/StringUtils.h"

#include <cstdarg>

namespace zlua
{
namespace
{
std::string FormatStringV(const char* format, va_list args)
{
    va_list argsCopy;
    va_copy(argsCopy, args);
#if IL2CPP_COMPILER_MSVC
    int n = _vscprintf_p(format, argsCopy);
#else
    char buf[1];
    int n = vsnprintf(buf, 0, format, argsCopy);
#endif
    va_end(argsCopy);
    if (n == -1)
        return std::string();

    std::string ret((size_t)n + 1, 0);
    n = vsnprintf(&ret[0], ret.size(), format, args);
    if (n == -1)
        return std::string();

    if (!ret.empty() && ret[ret.size() - 1] == '\0')
        ret.resize(ret.size() - 1);
    return ret;
}
} // namespace

void LuaException::Throw(const char* message)
{
    Il2CppException* ex = (Il2CppException*)il2cpp::vm::Object::New(MetadataUtil::GetLuaExceptionClass());
    il2cpp::vm::Runtime::ObjectInit((Il2CppObject*)ex);

    if (message != nullptr)
        IL2CPP_OBJECT_SETREF(ex, message, il2cpp::vm::String::New(message));
    il2cpp::vm::Exception::Raise(ex);
}

void LuaException::Throw(const std::string& message)
{
    Throw(message.c_str());
}

void LuaException::ThrowFormat(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const std::string message = FormatStringV(fmt, args);
    va_end(args);
    Throw(message.c_str());
}

void LuaException::Throw(Il2CppException* e)
{
    il2cpp::vm::Exception::Raise(e);
}

int LuaException::CallLuaError(Il2CppExceptionWrapper& e)
{
    Il2CppException* ex = e.ex;
    lua_State* L = LuaEnv::GetState();
    {
        std::string message("lua error: ");
        message.append(il2cpp::vm::Type::GetName(&ex->klass->byval_arg, Il2CppTypeNameFormat::IL2CPP_TYPE_NAME_FORMAT_IL));
        if (ex->message != nullptr)
        {
            message += ": ";
            message += "\n";
            message.append(il2cpp::utils::StringUtils::Utf16ToUtf8(ex->message->chars, ex->message->length));
        }

        lua_pushstring(L, message.c_str());
    }
    // we should destruct message before returning
    return lua_error(L);
}

} // namespace zlua