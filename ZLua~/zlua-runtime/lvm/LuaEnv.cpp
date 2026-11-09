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
#include "../marshal/StringMarshal.h"
#include "../marshal/StructRegistry.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../utils/MetadataUtil.h"
#include "../mt/AssemblyRegistry.h"
#include "../mt/MetaBinding.h"
#include "../mt/MetaTableCache.h"
#include "../mt/TypeRegistry.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"

#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Exception.h"
#include "os/Mutex.h"

namespace zlua
{

#include "../generated/BuiltinScripts.inc"

static lua_State* s_L = nullptr;
static uintptr_t s_domainId = 0;
static int s_errorHandlerRef = LUA_NOREF;
static std::vector<int> s_pendingRefReleases;
static baselib::ReentrantLock s_pendingRefReleasesMutex;
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
    // Refuse to wrap: even a pointer-sized counter must never revive an old domain.
    if (s_domainId == UINTPTR_MAX)
        LuaException::Throw("ZLua domain identity space exhausted.");
    lua_State* L = luaL_newstate();
    IL2CPP_ASSERT(L != nullptr);
    {
        il2cpp::os::FastAutoLock lock(&s_pendingRefReleasesMutex);
        ++s_domainId;
        s_L = L;
    }
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
    StructRegistry::BeginShutdown();
    // Finalizers may call C#, read payloads or construct fresh userdata. Keep
    // loader roots, metatables, object slots and binding contexts valid until
    // lua_close has run every closing callback.
    {
        StructPayloadProtectedCallScope scope;
        lua_close(s_L);
    }
    {
        il2cpp::os::FastAutoLock lock(&s_pendingRefReleasesMutex);
        s_L = nullptr;
        s_pendingRefReleases.clear();
    }

    // The registry no longer exists: clear native caches without luaL_unref.
    LuaLoader::Clear();
    MetaTableCache::Shutdown(nullptr);
    ObjectRegistry::Shutdown(nullptr);
    LuaGlobalRefs::Clear();
    TypeRegistry::Shutdown();
    MetaBinding::ShutdownState();
    MarshalMeta::InvalidateStateRefs();
    OpaqueValueMarshal::ShutdownState();
    s_errorHandlerRef = LUA_NOREF;
    StructRegistry::Shutdown(nullptr);
    StringMarshal::ClearScratch();
}

lua_State* LuaEnv::GetState()
{
    return s_L;
}

uintptr_t LuaEnv::GetDomainId()
{
    return s_domainId;
}

lua_State* LuaEnv::GetStateForInvoke(Il2CppObject* target)
{
    if (s_L == nullptr)
    {
        LuaException::Throw("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
    }

    LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
    if (luaMethod->domainId != s_domainId)
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
    if (ProtectedLuaDoString(s_L, chunk) != LUA_OK)
    {
        const std::string err = LuaUtil::FormatErrorObject(s_L, -1);
        lua_settop(s_L, oldTop);
        LuaException::Throw(err);
    }
    lua_settop(s_L, oldTop);
}

void LuaEnv::AddPendingRef(uintptr_t domainId, int refIndex)
{
    IL2CPP_ASSERT(refIndex != LUA_NOREF);
    il2cpp::os::FastAutoLock lock(&s_pendingRefReleasesMutex);
    // Finalizer threads must check state ownership under the same lock as enqueue.
    if (s_L == nullptr || s_domainId != domainId)
        return;
    s_pendingRefReleases.push_back(refIndex);
}

void LuaEnv::ProcessPendingRefReleases()
{
    il2cpp::os::FastAutoLock lock(&s_pendingRefReleasesMutex);
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
