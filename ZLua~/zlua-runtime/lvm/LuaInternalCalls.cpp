#include "LuaInternalCalls.h"

#include "LuaAppDomain.h"
#include "LuaEnv.h"
#include "LuaGlobalRefs.h"

#include "../marshal/DelegateMarshal.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"
#include "../utils/MetadataUtil.h"

#include "vm/InternalCalls.h"
#include "vm/Class.h"
#include "vm/Exception.h"
#include "vm/Type.h"
#include "utils/StringUtils.h"
#include "il2cpp-object-internals.h"

namespace zlua
{
static void LuaIl2CppAppDomain_InitializeInternal(Il2CppDelegate* moduleLoader)
{
    LuaAppDomain::InitializeFromManaged(moduleLoader);
}

static void LuaIl2CppAppDomain_ResetInternal(Il2CppDelegate* moduleLoader)
{
    LuaAppDomain::ResetFromManaged(moduleLoader);
}

static void LuaMethod_AddPendingRef(void* L, int32_t refIndex)
{
    lua_State* active = LuaEnv::GetState();
    if (active == nullptr || active != static_cast<lua_State*>(L))
    {
        return;
    }
    LuaEnv::AddPendingRef(refIndex);
}

static void LuaIl2CppAppDomain_ProcessPendingRefReleases()
{
    LuaEnv::ProcessPendingRefReleases();
}

static Il2CppObject* LuaIl2CppAppDomain_GetFunctionInternal(Il2CppReflectionType* delegateTypeObj, Il2CppString* luaModule,
                                                           Il2CppString* luaMethodName)
{
    if (delegateTypeObj == nullptr)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("delegateType"));
    }
    if (luaModule == nullptr || luaMethodName == nullptr)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentException("luaModule/luaMethodName", "must be non-null"));
    }

    Il2CppClass* delegateClass = il2cpp::vm::Class::FromIl2CppType(delegateTypeObj->type);
    il2cpp::vm::Class::Init(delegateClass);
    if (!il2cpp::vm::Class::IsSubclassOf(delegateClass, il2cpp_defaults.multicastdelegate_class, false))
    {
        LuaException::ThrowFormat("Type '%s' is not a MulticastDelegate", MetadataUtil::GetTypeFullName(delegateClass));
    }

    std::string moduleName = il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(luaModule));
    std::string methodName = il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(luaMethodName));
    if (moduleName.empty() || methodName.empty())
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentException("luaModule/luaMethodName", "must be non-empty"));
    }

    lua_State* L = LuaEnv::GetState();
    if (L == nullptr)
    {
        LuaException::Throw("ZLua is not initialized. Call LuaAppDomain.Initialize first.");
    }

    // Load / resolve shared module function ref, then duplicate so LuaMethod dispose cannot unref the cache.
    int sharedRef = LuaGlobalRefs::GetOrCreateModuleFunctionRef(moduleName.c_str(), methodName.c_str());
    LuaUtil::PushRef(L, sharedRef);
    int ownedRef = LuaUtil::ToLuaRef(L);
    return reinterpret_cast<Il2CppObject*>(DelegateMarshal::CreateFromFuncRef(L, delegateClass, ownedRef));
}

void LuaInternalCalls::RegisterCoreInternalCalls()
{
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::InitializeInternal", (Il2CppMethodPointer)LuaIl2CppAppDomain_InitializeInternal);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::ResetInternal", (Il2CppMethodPointer)LuaIl2CppAppDomain_ResetInternal);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::ProcessPendingRefReleases", (Il2CppMethodPointer)LuaIl2CppAppDomain_ProcessPendingRefReleases);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::GetFunctionInternal", (Il2CppMethodPointer)LuaIl2CppAppDomain_GetFunctionInternal);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaMethod::AddPendingRef", (Il2CppMethodPointer)LuaMethod_AddPendingRef);
}
} // namespace zlua
