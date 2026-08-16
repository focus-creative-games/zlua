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

#include <string>
#include <vector>

#include "LuaEnv.h"

#include "LuaGlobalRefs.h"
#include "LuaLoader.h"
#include "ZLuaLib.h"


#include "../marshal/ObjectRegistry.h"
#include "../marshal/StructRegistry.h"
#include "../marshal/MarshalDefs.h"
#include "../utils/MetadataUtil.h"
#include "../mt/AssemblyRegistry.h"
#include "../mt/MetaTableCache.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"

#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Exception.h"


namespace zlua
{

#include "../generated/BuiltinScripts.inc"

static lua_State* s_L = nullptr;
static int s_errorHandlerRef = LUA_NOREF;
static std::vector<int> s_pendingRefReleases;
static const MethodInfo* s_debugLogMethod = nullptr;

void LuaEnv::RegisterGlobals()
{
    DoStringIgnoreResult("__ZLUA_IL2CPP_PLAYER__=true");
    DoStringIgnoreResult(kZLuaGlobalsLua);
    InitErrorHandlerRef();
}

void LuaEnv::InitDebugLogMethod()
{
    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("UnityEngine.CoreModule");
    Il2CppClass* debugClass = MetadataUtil::ResolveType(assembly, "UnityEngine.Debug");
    s_debugLogMethod = MetadataUtil::FindMethod(debugClass, "Log", 1, true);
    IL2CPP_ASSERT(s_debugLogMethod != nullptr);
}

static void LogToUnity(const char* message)
{
    Il2CppString* msgStr = il2cpp::vm::String::New(message);
    void* params[1] = {msgStr};
    il2cpp::vm::Runtime::Invoke(s_debugLogMethod, nullptr, params, nullptr);
}

static int ZLuaPrint(lua_State* L)
{
    const int count = lua_gettop(L);
    std::string msg;
    for (int i = 1; i <= count; ++i)
    {
        if (i > 1)
            msg.push_back('\t');
        size_t len = 0;
        const char* str = luaL_tolstring(L, i, &len);
        if (str != nullptr)
            msg.append(str, len);
        lua_pop(L, 1);
    }

    // level 1 = first Lua caller of print (same as luaL_where).
    luaL_traceback(L, L, msg.c_str(), 1);
    size_t tbLen = 0;
    const char* tb = lua_tolstring(L, -1, &tbLen);
    std::string line = "[ZLua] ";
    if (tb != nullptr)
        line.append(tb, tbLen);
    lua_pop(L, 1);

    LogToUnity(line.c_str());
    return 0;
}

void LuaEnv::RegisterPrintCallback()
{
    InitDebugLogMethod();
    lua_pushcfunction(s_L, ZLuaPrint);
    lua_setglobal(s_L, "print");
}

void LuaEnv::RegisterZLuaApi()
{
    ZLuaLib::RegisterGlobals(s_L);
    DoStringIgnoreResult(kZLuaLibLua);
}

void LuaEnv::RegisterLibs()
{
    luaL_openlibs(s_L);
    RegisterPrintCallback();
    RegisterZLuaApi();
}

void LuaEnv::Initialize()
{
    IL2CPP_ASSERT(s_L == nullptr);
    s_L = luaL_newstate();
    IL2CPP_ASSERT(s_L != nullptr);
    RegisterGlobals();
    RegisterLibs();
    ObjectRegistry::Initialize(s_L);
    StructRegistry::Initialize(s_L);
    MetaTableCache::Initialize(s_L);
    LuaLoader::InstallHooks();
    AssemblyRegistry::InitializeCSharpRoot(s_L);
}

void LuaEnv::Shutdown()
{
    IL2CPP_ASSERT(s_L != nullptr);

    ProcessPendingRefReleases();
    MetaTableCache::Shutdown(s_L);
    StructRegistry::Shutdown(s_L);
    ObjectRegistry::Shutdown(s_L);
    LuaGlobalRefs::Clear();

    if (s_errorHandlerRef != LUA_NOREF)
    {
        luaL_unref(s_L, LUA_REGISTRYINDEX, s_errorHandlerRef);
        s_errorHandlerRef = LUA_NOREF;
    }

    lua_close(s_L);
    s_L = nullptr;
    LuaLoader::Clear();
}

lua_State* LuaEnv::GetState()
{
    return s_L;
}

lua_State* LuaEnv::GetStateForInvoke(Il2CppObject* target)
{
    if (s_L == nullptr)
    {
        LuaException::Throw("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
    }

    LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
    if (luaMethod->L != s_L)
    {
        LuaException::Throw("ZLua domain was Reset; discard old GetFunction delegates and re-bind.");
    }

    return s_L;
}

void LuaEnv::InitErrorHandlerRef()
{
    const int oldTop = lua_gettop(s_L);
    if (lua_getglobal(s_L, "__zluaErrorHandler") != LUA_TFUNCTION)
    {
        lua_settop(s_L, oldTop);
        LuaException::Throw("Lua global '__zluaErrorHandler' is not available.");
    }

    s_errorHandlerRef = luaL_ref(s_L, LUA_REGISTRYINDEX);
    lua_settop(s_L, oldTop);
}

int LuaEnv::GetErrorHandlerRef()
{
    IL2CPP_ASSERT(s_errorHandlerRef != LUA_NOREF);
    return s_errorHandlerRef;
}

int LuaEnv::PushErrorHandler()
{
    IL2CPP_ASSERT(s_errorHandlerRef != LUA_NOREF);
    lua_rawgeti(s_L, LUA_REGISTRYINDEX, s_errorHandlerRef);
    return lua_gettop(s_L);
}

void LuaEnv::DoStringIgnoreResult(const char* chunk)
{
    const int oldTop = lua_gettop(s_L);
    if (luaL_dostring(s_L, chunk) != LUA_OK)
    {
        const std::string err = LuaUtil::FormatErrorObject(s_L, -1);
        lua_settop(s_L, oldTop);
        LuaException::Throw(err);
    }
    lua_settop(s_L, oldTop);
}

void LuaEnv::AddPendingRef(int refIndex)
{
    IL2CPP_ASSERT(refIndex != LUA_NOREF);
    if (s_L == nullptr)
        return;
    s_pendingRefReleases.push_back(refIndex);
}

void LuaEnv::ProcessPendingRefReleases()
{
    if (s_L == nullptr)
    {
        s_pendingRefReleases.clear();
        return;
    }
    if (s_pendingRefReleases.empty())
        return;

    for (int refIndex : s_pendingRefReleases)
        luaL_unref(s_L, LUA_REGISTRYINDEX, refIndex);
    s_pendingRefReleases.clear();
}
} // namespace zlua
