#include "LuaEnv.h"

#include "Marshaling.h"

#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Exception.h"
#include "vm/Array.h"
#include "gc/GarbageCollector.h"
#include "gc/WriteBarrier.h"

#include <unordered_map>
#include <string>

namespace novalua
{
    static lua_State* s_L = nullptr;
    static Il2CppDelegate** s_gcFixedDatas = nullptr;
    static Il2CppDelegate* s_ModuleLoader = nullptr;
    static const MethodInfo* s_moduleLoaderInvoker = nullptr;
    static std::unordered_map<std::string, int> s_ModuleRefs;
    static std::unordered_map<std::string, int> s_ModuleFunctionRefs;

    void LuaEnv::RaiseLuaException(const char* msg)
    {
        il2cpp::vm::Exception::Raise(
            il2cpp::vm::Exception::GetInvalidOperationException(msg != nullptr ? msg : "NovaLua error"));
    }

    static int NovaLuaPrint(lua_State* L)
    {
        int count = lua_gettop(L);
        std::string line = "[NovaLua] ";
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
        printf("%s\n", line.c_str());
        return 0;
    }

    void LuaEnv::Create(Il2CppDelegate* moduleLoader)
    {
        if (moduleLoader == nullptr)
        {
            RaiseLuaException("module loader is null");
        }
        if (s_gcFixedDatas == nullptr)
        {
            s_gcFixedDatas = (Il2CppDelegate**)il2cpp::gc::GarbageCollector::AllocateFixed(sizeof(void*), nullptr);
        }
        il2cpp::gc::WriteBarrier::GenericStore(s_gcFixedDatas, moduleLoader);
        s_ModuleLoader = moduleLoader;
        s_moduleLoaderInvoker = il2cpp::vm::Runtime::GetDelegateInvoke(((Il2CppObject*)s_ModuleLoader)->klass);
        IL2CPP_ASSERT(s_moduleLoaderInvoker);
        
        if (s_L != nullptr)
        {
            return;
        }

        s_L = luaL_newstate();
        if (s_L == nullptr)
        {
            RaiseLuaException("Failed to create lua state");
        }

        luaL_openlibs(s_L);
        RegisterPrintCallback();
    }

    void LuaEnv::Shutdown()
    {
        if (s_L == nullptr)
            return;

        for (auto& kv : s_ModuleFunctionRefs)
            luaL_unref(s_L, LUA_REGISTRYINDEX, kv.second);
        s_ModuleFunctionRefs.clear();

        for (auto& kv : s_ModuleRefs)
            luaL_unref(s_L, LUA_REGISTRYINDEX, kv.second);
        s_ModuleRefs.clear();

        lua_close(s_L);
        s_L = nullptr;
        s_ModuleLoader = nullptr;
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
        else if (result->klass->byval_arg.type == IL2CPP_TYPE_SZARRAY && result->klass->rank == 1 && result->klass->element_class->byval_arg.type == IL2CPP_TYPE_I1)

        {
            Il2CppArray* charArray = (Il2CppArray*)result;
            return std::string((char*)il2cpp::vm::Array::GetFirstElementAddress(charArray), charArray->max_length);
        }

        RaiseLuaException("Lua module loader must return a string or byte array");
    }

    void LuaEnv::EnsureModuleLoaded(const char* moduleName)
    {
        if (moduleName == nullptr || moduleName[0] == '\0')
            return;

        if (s_ModuleRefs.find(moduleName) != s_ModuleRefs.end())
            return;

        std::string source = LoadModuleSource(moduleName);
        if (source.empty())
        {
            std::string msg = std::string("Lua module '") + moduleName + "' cannot be loaded.";
            RaiseLuaException(msg.c_str());
        }

        const int oldTop = lua_gettop(s_L);
        if (luaL_loadbufferx(s_L, source.c_str(), source.size(), moduleName, nullptr) != LUA_OK)
        {
            const char* err = lua_tostring(s_L, -1);
            std::string msg = std::string("Error loading lua module '") + moduleName + "': " + (err ? err : "");
            lua_settop(s_L, oldTop);
            RaiseLuaException(msg.c_str());
        }

        if (lua_pcall(s_L, 0, 1, 0) != LUA_OK)
        {
            const char* err = lua_tostring(s_L, -1);
            std::string msg = std::string("Error executing lua module '") + moduleName + "': " + (err ? err : "");
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
        lua_pushcfunction(s_L, NovaLuaPrint);
        lua_setglobal(s_L, "print");
    }
}
