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

#include "LuaLoader.h"

#include "LuaEnv.h"

#include "../utils/LuaException.h"

#include "vm/Runtime.h"
#include "vm/String.h"
#include "vm/Exception.h"
#include "vm/Array.h"
#include "gc/GarbageCollector.h"

namespace zlua
{
static Il2CppDelegate* s_ModuleLoader = nullptr;
static const MethodInfo* s_moduleLoaderInvoker = nullptr;

void LuaLoader::RegisterRoots()
{
    il2cpp::gc::GarbageCollector::RegisterRoot((char*)&s_ModuleLoader, sizeof(Il2CppDelegate*));
}

void LuaLoader::SetModuleLoader(Il2CppDelegate* moduleLoader)
{
    IL2CPP_ASSERT(moduleLoader != nullptr);
    s_ModuleLoader = moduleLoader;
    s_moduleLoaderInvoker = il2cpp::vm::Runtime::GetDelegateInvoke(s_ModuleLoader->object.klass);
    IL2CPP_ASSERT(s_moduleLoaderInvoker);
}

Il2CppDelegate* LuaLoader::GetModuleLoader()
{
    return s_ModuleLoader;
}

void LuaLoader::Clear()
{
    s_ModuleLoader = nullptr;
    s_moduleLoaderInvoker = nullptr;
}

static void LoadModuleSource(const char* moduleName, std::string& source)
{
    if (s_ModuleLoader == nullptr)
    {
        LuaException::Throw("Lua module loader is not configured");
    }

    Il2CppString* moduleNameStr = il2cpp::vm::String::New(moduleName);
    void* params[1] = {moduleNameStr};
    Il2CppException* exc = nullptr;
    Il2CppObject* result = il2cpp::vm::Runtime::Invoke(s_moduleLoaderInvoker, s_ModuleLoader, params, &exc);
    if (exc != nullptr)
    {
        LuaException::Throw(exc);
    }

    if (result == nullptr)
    {
        LuaException::Throw("Lua module loader returned null");
    }
    switch (result->klass->byval_arg.type)
    {
    case IL2CPP_TYPE_STRING:
    {
        Il2CppString* sourceStr = (Il2CppString*)result;
        source = il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(sourceStr), il2cpp::utils::StringUtils::GetLength(sourceStr));
        return;
    }
    case IL2CPP_TYPE_SZARRAY:
    {
        Il2CppArray* charArray = (Il2CppArray*)result;
        source = std::string((char*)il2cpp::vm::Array::GetFirstElementAddress(charArray), charArray->max_length);
        return;
    }
    default:
    {
        LuaException::Throw("Unsupported module source type");
    }
    }
}

static int ZLuaLoadModule(lua_State* L)
{
    const char* moduleName = luaL_checkstring(L, 1);
    ZLUA_TRY_BEGIN()
    std::string source;
    LoadModuleSource(moduleName, source);
    lua_pushlstring(L, source.c_str(), source.size());
    return 1;
    ZLUA_TRY_END()
}

void LuaLoader::InstallHooks()
{
    lua_State* L = LuaEnv::GetState();
    lua_pushcfunction(L, ZLuaLoadModule);
    lua_setglobal(L, "__zlua_load_module");

    const char* installSearcherChunk = R"(
local function zlua_module_searcher(modname)
    local src = __zlua_load_module(modname)
    if src == nil then
        return nil
    end
    local name = '@' .. modname:gsub('%.', '/') .. '.lua'
    local chunk, err = (loadstring or load)(src, name)
    if not chunk then
        error(err, 2)
    end
    return chunk
end

local searchers = package.searchers or package.loaders
table.insert(searchers, 2, zlua_module_searcher)
)";
    LuaEnv::DoStringIgnoreResult(installSearcherChunk);
}
} // namespace zlua
