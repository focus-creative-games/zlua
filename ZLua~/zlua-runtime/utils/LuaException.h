#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class LuaException : Il2CppObject
{
  public:
    static void Throw(const char* message);
    static void Throw(const std::string& message);
    static void ThrowFormat(const char* fmt, ...);
    static void Throw(Il2CppException* e);
    static int CallLuaError(Il2CppExceptionWrapper& e);
};

#define ZLUA_TRY_BEGIN() \
    try                  \
    {
#define ZLUA_TRY_END()                      \
    }                                       \
    catch (Il2CppExceptionWrapper & e)      \
    {                                       \
        return LuaException::CallLuaError(e); \
    }
} // namespace zlua