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
    int count = lua_gettop(L);
    std::string line = "[ZLua] ";
    for (int i = 1; i <= count; ++i)
    {
        if (i > 1)
            line.push_back('\t');
        size_t len = 0;
        const char* str = luaL_tolstring(L, i, &len);
        if (str != nullptr)
            line.append(str, len);
        lua_pop(L, 1);
    }
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
        const char* err = lua_tostring(s_L, -1);
        lua_settop(s_L, oldTop);
        LuaException::Throw(err != nullptr ? err : "lua dostring failed");
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
