#pragma once

#include "../marshal/MarshalDefs.h"

namespace zlua
{

class LuaInvokeHelper
{
    public:
    static bool ResolveInvokeSite(lua_State* L, const char* moduleName, const char* funcName, LuaInvokeSite& outSite);
    static const MethodMarshalCtx* ResolveMethodMarshalCtx(lua_State* L, const char* moduleName, const char* funcFullNameWithSignature);
    static void ResolveInvokeSiteAndMethod(lua_State* L, const char* luaModuleName, const char* luaMethodName, const char* csAssemblyName,
        const char* csMethodFullNameWithSignature, LuaInvokeSite& outSite, const MethodMarshalCtx*& outMethodMarshalCtx);
};
} // namespace zlua
