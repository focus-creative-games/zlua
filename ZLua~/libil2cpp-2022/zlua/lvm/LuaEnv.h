#pragma once

#include "../ZLuaCommon.h"

namespace zlua
{
class LuaEnv
{
  public:
    static void Initialize();
    static void Shutdown();

    static lua_State* GetState();
    static int GetErrorHandlerRef();

    static int PushErrorHandler();

    static void DoStringIgnoreResult(const char* chunk);

    static void AddPendingRef(int refIndex);
    static void ProcessPendingRefReleases();

  private:
    static void RegisterGlobals();
    static void RegisterLibs();
    static void RegisterPrintCallback();
    static void RegisterZLuaApi();
    static void InitDebugLogMethod();
    static void InitErrorHandlerRef();
};
} // namespace zlua
