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
class LuaEnv
{
  public:
    static void Initialize();
    static void Shutdown();

    static lua_State* GetState();
    /// Validate that <paramref name="target"/> (LuaMethod) still belongs to the active state.
    static lua_State* GetStateForInvoke(Il2CppObject* target);
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
