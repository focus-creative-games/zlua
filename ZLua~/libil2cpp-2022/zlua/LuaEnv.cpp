#include "LuaEnv.h"

#include "marshal/ObjectMarshal.h"
#include "MetadataUtil.h"
#include "mt/AssemblyRegistry.h"
#include "ZLuaLib.h"
#include "LuaUtil.h"

#include "il2cpp-config.h"
#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Exception.h"
#include "vm/Array.h"
#include "gc/GarbageCollector.h"
#include "gc/WriteBarrier.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace zlua
{
    static lua_State* s_L = nullptr;
    static Il2CppDelegate* s_ModuleLoader = nullptr;
    static const MethodInfo* s_moduleLoaderInvoker = nullptr;
    static bool s_moduleLoaderHooksInstalled = false;
    static std::unordered_map<std::string, int> s_ModuleRefs;
    static std::unordered_map<std::string, int> s_ModuleFunctionRefs;
    static int s_errorHandlerRef = LUA_NOREF;
    static std::vector<int> s_pendingRefReleases;
    static const MethodInfo* s_debugLogMethod = nullptr;
    static bool s_debugLogMethodResolved = false;

    static const MethodInfo* EnsureDebugLogMethod()
    {
        if (s_debugLogMethodResolved)
            return s_debugLogMethod;

        s_debugLogMethodResolved = true;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("UnityEngine.CoreModule");
        Il2CppClass* debugClass = MetadataUtil::ResolveType(assembly, "UnityEngine.Debug");
        s_debugLogMethod = MetadataUtil::FindMethod(debugClass, "Log", 1, true);

        return s_debugLogMethod;
    }

    static void LogToUnity(const char* message)
    {
        const MethodInfo* logMethod = EnsureDebugLogMethod();
        IL2CPP_ASSERT(logMethod != nullptr);
        Il2CppString* msgStr = il2cpp::vm::String::New(message);
        void* params[1] = { msgStr };
        il2cpp::vm::Runtime::Invoke(logMethod, nullptr, params, nullptr);
    }

    static int EnsureErrorHandlerRef(lua_State* L)
    {
        if (s_errorHandlerRef != LUA_NOREF)
            return s_errorHandlerRef;

        const int oldTop = lua_gettop(L);
        if (lua_getglobal(L, "__zluaErrorHandler") != LUA_TFUNCTION)
        {
            lua_settop(L, oldTop);
            LuaEnv::RaiseLuaException("Lua global '__zluaErrorHandler' is not available.");
        }

        s_errorHandlerRef = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_settop(L, oldTop);
        return s_errorHandlerRef;
    }

    void LuaEnv::RaiseLuaException(const char* msg)
    {
        il2cpp::vm::Exception::Raise(
            il2cpp::vm::Exception::GetInvalidOperationException(msg != nullptr ? msg : "ZLua error"));
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

    static std::string TryLoadModuleSource(const char* moduleName)
    {
        if (s_ModuleLoader == nullptr || moduleName == nullptr || moduleName[0] == '\0')
            return std::string();

        Il2CppString* moduleNameStr = il2cpp::vm::String::New(moduleName);
        void* params[1] = { moduleNameStr };
        Il2CppException* exc = nullptr;
        const MethodInfo* invoke = il2cpp::vm::Runtime::GetDelegateInvoke(((Il2CppObject*)s_ModuleLoader)->klass);
        Il2CppObject* result = il2cpp::vm::Runtime::Invoke(invoke, s_ModuleLoader, params, &exc);
        if (exc != nullptr)
            il2cpp::vm::Exception::Raise(exc);

        if (result == nullptr)
            return std::string();
        if (result->klass->byval_arg.type == IL2CPP_TYPE_STRING)
        {
            Il2CppString* sourceStr = (Il2CppString*)result;
            return il2cpp::utils::StringUtils::Utf16ToUtf8(
                il2cpp::utils::StringUtils::GetChars(sourceStr),
                il2cpp::utils::StringUtils::GetLength(sourceStr));
        }
        if (result->klass->byval_arg.type == IL2CPP_TYPE_SZARRAY && result->klass->rank == 1 &&
            result->klass->element_class->byval_arg.type == IL2CPP_TYPE_I1)
        {
            Il2CppArray* charArray = (Il2CppArray*)result;
            return std::string((char*)il2cpp::vm::Array::GetFirstElementAddress(charArray), charArray->max_length);
        }

        return std::string();
    }

    static int ZLuaLoadModule(lua_State* L)
    {
        const char* moduleName = luaL_checkstring(L, 1);
        try
        {
            std::string source = TryLoadModuleSource(moduleName);
            if (source.empty())
            {
                lua_pushnil(L);
                return 1;
            }

            lua_pushlstring(L, source.c_str(), source.size());
            return 1;
        }
        catch (Il2CppExceptionWrapper& e)
        {
            return LuaUtil::RaiseAsLuaError(L, "Exception in module loader", e.ex);
        }
    }

    static void InstallModuleLoaderHooks()
    {
        if (s_moduleLoaderHooksInstalled || s_L == nullptr)
            return;

        lua_pushcfunction(s_L, ZLuaLoadModule);
        lua_setglobal(s_L, "__zlua_load_module");

        const char* installSearcherChunk = R"(
if rawget(_G, '__zlua_module_searcher_installed') then
    return
end
_G.__zlua_module_searcher_installed = true

local function zlua_module_searcher(modname)
    local src = __zlua_load_module(modname)
    if src == nil then
        return nil
    end
    local chunk, err = load(src, '@' .. modname:gsub('%.', '/') .. '.lua')
    if not chunk then
        error(err, 2)
    end
    return chunk
end

table.insert(package.searchers, 2, zlua_module_searcher)
)";
        LuaEnv::DoStringIgnoreResult(installSearcherChunk);
        s_moduleLoaderHooksInstalled = true;
    }

    void LuaEnv::RegisterRoots()
    {
        il2cpp::gc::GarbageCollector::RegisterRoot((char*)&s_ModuleLoader, sizeof(s_ModuleLoader));
    }

    void LuaEnv::Create(Il2CppDelegate* moduleLoader)
    {
        if (moduleLoader == nullptr)
        {
            RaiseLuaException("module loader is null");
        }
        s_ModuleLoader = moduleLoader;
        s_moduleLoaderInvoker = il2cpp::vm::Runtime::GetDelegateInvoke(((Il2CppObject*)s_ModuleLoader)->klass);
        IL2CPP_ASSERT(s_moduleLoaderInvoker);

        if (s_L != nullptr)
        {
            InstallModuleLoaderHooks();
            return;
        }

        s_L = luaL_newstate();
        if (s_L == nullptr)
        {
            RaiseLuaException("Failed to create lua state");
        }

        luaL_openlibs(s_L);
        RegisterPrintCallback();
        ObjectMarshal::EnsureObjectCache(s_L);
        InstallModuleLoaderHooks();
    }

    void LuaEnv::Shutdown()
    {
        if (s_L == nullptr)
            return;

        ProcessPendingRefReleases();
        ObjectMarshal::Shutdown();

        for (auto& kv : s_ModuleFunctionRefs)
            luaL_unref(s_L, LUA_REGISTRYINDEX, kv.second);
        s_ModuleFunctionRefs.clear();

        for (auto& kv : s_ModuleRefs)
            luaL_unref(s_L, LUA_REGISTRYINDEX, kv.second);
        s_ModuleRefs.clear();

        if (s_errorHandlerRef != LUA_NOREF)
        {
            luaL_unref(s_L, LUA_REGISTRYINDEX, s_errorHandlerRef);
            s_errorHandlerRef = LUA_NOREF;
        }

        lua_close(s_L);
        s_L = nullptr;
        s_ModuleLoader = nullptr;
        s_moduleLoaderHooksInstalled = false;
    }

    lua_State* LuaEnv::GetState()
    {
        return s_L;
    }

    Il2CppDelegate* LuaEnv::GetModuleLoader()
    {
        return s_ModuleLoader;
    }

    std::string LuaEnv::LoadModuleSource(const char* moduleName)
    {
        if (s_ModuleLoader == nullptr)
        {
            RaiseLuaException("Lua module loader is not configured");
        }

        std::string source = TryLoadModuleSource(moduleName);
        if (source.empty())
        {
            std::string msg = std::string("Lua module '") + moduleName + "' cannot be loaded.";
            RaiseLuaException(msg.c_str());
        }

        return source;
    }

    void LuaEnv::EnsureModuleLoaded(const char* moduleName)
    {
        if (moduleName == nullptr || moduleName[0] == '\0')
            return;

        if (s_ModuleRefs.find(moduleName) != s_ModuleRefs.end())
            return;

        if (s_ModuleLoader == nullptr)
        {
            RaiseLuaException("Lua module loader is not configured");
        }

        const int oldTop = lua_gettop(s_L);
        if (lua_getglobal(s_L, "require") != LUA_TFUNCTION)
        {
            lua_settop(s_L, oldTop);
            RaiseLuaException("Lua global 'require' is not available.");
        }

        lua_pushstring(s_L, moduleName);
        if (lua_pcall(s_L, 1, 1, 0) != LUA_OK)
        {
            const char* err = lua_tostring(s_L, -1);
            std::string msg = std::string("Error requiring lua module '") + moduleName + "': " + (err ? err : "");
            lua_settop(s_L, oldTop);
            RaiseLuaException(msg.c_str());
        }

        if (!lua_istable(s_L, -1))
        {
            lua_settop(s_L, oldTop);
            std::string msg = std::string("Lua module '") + moduleName + "' must return a table.";
            RaiseLuaException(msg.c_str());
        }

        const int moduleRef = luaL_ref(s_L, LUA_REGISTRYINDEX);
        s_ModuleRefs[moduleName] = moduleRef;
        lua_settop(s_L, oldTop);
    }

    int LuaEnv::GetModuleRef(const char* moduleName)
    {
        EnsureModuleLoaded(moduleName);
        auto it = s_ModuleRefs.find(moduleName);
        if (it == s_ModuleRefs.end())
            return LUA_NOREF;
        return it->second;
    }

    int LuaEnv::FindModuleFunctionRef(const char* moduleName, const char* methodName)
    {
        const int moduleRef = GetModuleRef(moduleName);
        const int oldTop = lua_gettop(s_L);
        lua_rawgeti(s_L, LUA_REGISTRYINDEX, moduleRef);
        lua_getfield(s_L, -1, methodName);
        if (!lua_isfunction(s_L, -1))
        {
            lua_settop(s_L, oldTop);
            std::string msg = std::string("Lua function '") + moduleName + "." + methodName + "' not found.";
            RaiseLuaException(msg.c_str());
        }

        const int functionRef = luaL_ref(s_L, LUA_REGISTRYINDEX);
        lua_settop(s_L, oldTop);
        return functionRef;
    }

    int LuaEnv::GetOrCreateModuleFunctionRef(const char* moduleName, const char* methodName)
    {
        std::string key = std::string(moduleName) + "::" + methodName;
        auto it = s_ModuleFunctionRefs.find(key);
        if (it != s_ModuleFunctionRefs.end())
            return it->second;

        const int functionRef = FindModuleFunctionRef(moduleName, methodName);
        s_ModuleFunctionRefs[key] = functionRef;
        return functionRef;
    }

    void LuaEnv::DoStringIgnoreResult(const char* chunk)
    {
        const int oldTop = lua_gettop(s_L);
        if (luaL_dostring(s_L, chunk) != LUA_OK)
        {
            const char* err = lua_tostring(s_L, -1);
            lua_settop(s_L, oldTop);
            RaiseLuaException(err != nullptr ? err : "lua dostring failed");
        }
        lua_settop(s_L, oldTop);
    }

    void LuaEnv::RegisterPrintCallback()
    {
        EnsureDebugLogMethod();
        lua_pushcfunction(s_L, ZLuaPrint);
        lua_setglobal(s_L, "print");
    }

    void LuaEnv::RegisterZLuaApi()
    {
        AssemblyRegistry::EnsureCSharpRoot();
        ZLuaLib::RegisterGlobals();
    }

    int LuaEnv::PushErrorHandler(lua_State* L)
    {
        IL2CPP_ASSERT(L == s_L);
        lua_rawgeti(L, LUA_REGISTRYINDEX, EnsureErrorHandlerRef(L));
        return lua_gettop(L);
    }

    void LuaEnv::EnsureErrorHandlerCached()
    {
        IL2CPP_ASSERT(s_L != nullptr);
        EnsureErrorHandlerRef(s_L);
    }

    void LuaEnv::AddPendingRef(int refIndex)
    {
        if (refIndex == LUA_NOREF)
            return;
        s_pendingRefReleases.push_back(refIndex);
    }

    void LuaEnv::ProcessPendingRefReleases()
    {
        if (s_L == nullptr || s_pendingRefReleases.empty())
            return;

        for (int refIndex : s_pendingRefReleases)
            luaL_unref(s_L, LUA_REGISTRYINDEX, refIndex);
        s_pendingRefReleases.clear();
    }
}
