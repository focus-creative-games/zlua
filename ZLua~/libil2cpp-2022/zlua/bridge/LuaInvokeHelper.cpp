#include "LuaInvokeHelper.h"

#include "../lvm/LuaGlobalRefs.h"
#include "../mt/MetaBinding.h"
#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"

namespace zlua
{
bool LuaInvokeHelper::ResolveInvokeSite(lua_State* L, const char* moduleName, const char* funcName, LuaInvokeSite& outSite)
{
    outSite.funcRef = LUA_NOREF;
    int moduleRef = LuaGlobalRefs::GetModuleRef(moduleName);
    if (moduleRef == LUA_NOREF)
    {
        return false;
    }
    int funcRef = LuaGlobalRefs::GetOrCreateModuleFunctionRef(moduleName, funcName);
    if (funcRef == LUA_NOREF)
    {
        return false;
    }
    outSite.funcRef = funcRef;
    return true;
}

struct MethodParseInfo
{
    std::string typeName;
    std::string methodName;
    std::string signature;
};

// method full signature format: <typename>::<method name>[(<method signature>)]
static bool TryParseMethodFullNameWithSignature(const char* funcFullNameWithSignature, MethodParseInfo& outInfo)
{
    const char* typeNameStart = funcFullNameWithSignature;
    const char* typeNameEnd = strchr(typeNameStart, ':');
    if (typeNameEnd == nullptr)
    {
        return false;
    }
    outInfo.typeName = std::string(typeNameStart, typeNameEnd - typeNameStart);
    if (typeNameEnd[1] != ':')
    {
        return false;
    }
    const char* methodNameStart = typeNameEnd + 2;
    const char* methodNameEnd = strchr(methodNameStart, '(');
    if (methodNameEnd == nullptr)
    {
        outInfo.methodName = std::string(methodNameStart, funcFullNameWithSignature + strlen(funcFullNameWithSignature) - methodNameStart);
        return true;
    }
    outInfo.methodName = std::string(methodNameStart, methodNameEnd - methodNameStart);
    const char* signatureStart = methodNameEnd + 1;
    const char* signatureEnd = strchr(signatureStart, ')');
    if (signatureEnd != nullptr)
    {
        outInfo.signature = std::string(signatureStart, signatureEnd - signatureStart);
    }
    return true;
}

static const MethodInfo* ResolveMethod(Il2CppClass* klass, const char* methodName, const char* signature)
{
    il2cpp::vm::Class::Init(klass);
    if (signature[0] == '\0')
    {
        return il2cpp::vm::Class::GetMethodFromName(klass, methodName, -1);
    }
    std::string tempSignature;
    for (uint16_t i = 0; i < klass->method_count; i++)
    {
        const MethodInfo* method = klass->methods[i];
        if (strcmp(method->name, methodName) != 0)
        {
            continue;
        }
        tempSignature.clear();
        MetadataUtil::CreateMethodParameterSignatureOnly(method, tempSignature);
        if (tempSignature == signature)
        {
            return method;
        }
    }
    return nullptr;
}

const MethodMarshalCtx* LuaInvokeHelper::ResolveMethodMarshalCtx(lua_State* L, const char* moduleName, const char* funcFullNameWithSignature)
{
    MethodParseInfo info;
    if (!TryParseMethodFullNameWithSignature(funcFullNameWithSignature, info))
    {
        return nullptr;
    }
    
    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(moduleName);
    if (assembly == nullptr)
    {
        return nullptr;
    }
    Il2CppClass* klass = MetadataUtil::ResolveType(assembly, info.typeName.c_str());
    if (klass == nullptr)
    {
        return nullptr;
    }
    const MethodInfo* method = ResolveMethod(klass, info.methodName.c_str(), info.signature.c_str());
    if (method == nullptr)
    {
        return nullptr;
    }
    return MetaBinding::CreateMethodMarshalCtx(L, method, false);
}

void LuaInvokeHelper::ResolveInvokeSiteAndMethod(lua_State* L, const char* luaModuleName, const char* luaMethodName, const char* csAssemblyName,
                                                 const char* csMethodFullNameWithSignature, LuaInvokeSite& outSite,
                                                 const MethodMarshalCtx*& outMethodMarshalCtx)
{
    if (!ResolveInvokeSite(L, luaModuleName, luaMethodName, outSite))
    {
        LuaException::ThrowFormat("Failed to resolve invoke site for %s::%s", luaModuleName, luaMethodName);
    }
    outMethodMarshalCtx = ResolveMethodMarshalCtx(L, csAssemblyName, csMethodFullNameWithSignature);
    if (outMethodMarshalCtx == nullptr)
    {
        LuaException::ThrowFormat("Failed to resolve method for %s::%s", csAssemblyName, csMethodFullNameWithSignature);
    }
}
} // namespace zlua