#include "LuaInternalCalls.h"

#include "LuaAppDomain.h"
#include "LuaEnv.h"

#include "vm/InternalCalls.h"
#include "il2cpp-object-internals.h"

namespace zlua
{
static void LuaIl2CppAppDomain_InitializeInternal(Il2CppDelegate* moduleLoader)
{
    LuaAppDomain::InitializeFromManaged(moduleLoader);
}

static void LuaMethod_AddPendingRef(void* /*L*/, int32_t refIndex)
{
    LuaEnv::AddPendingRef(refIndex);
}

static void LuaIl2CppAppDomain_ProcessPendingRefReleases()
{
    LuaEnv::ProcessPendingRefReleases();
}

void LuaInternalCalls::RegisterCoreInternalCalls()
{
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::InitializeInternal", (Il2CppMethodPointer)LuaIl2CppAppDomain_InitializeInternal);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaIl2CppAppDomain::ProcessPendingRefReleases", (Il2CppMethodPointer)LuaIl2CppAppDomain_ProcessPendingRefReleases);
    il2cpp::vm::InternalCalls::Add("ZLua.LuaMethod::AddPendingRef", (Il2CppMethodPointer)LuaMethod_AddPendingRef);
}
} // namespace zlua
