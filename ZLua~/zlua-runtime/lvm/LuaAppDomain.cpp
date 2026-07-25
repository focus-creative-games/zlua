#include "LuaAppDomain.h"

#include "LuaEnv.h"
#include "LuaInternalCalls.h"
#include "LuaLoader.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../bridge/PropertyBridge.h"
#include "../bridge/MethodBridge.h"
#include "../bridge/DelegateBridge.h"
#include "../generated/MarshalBindings.h"

#include "vm/Exception.h"

namespace zlua
{
void LuaAppDomain::Initialize()
{
    LuaMetadataAlloc::Initialize();
    MetadataUtil::Initialize();
    PropertyBridge::Initialize();
    MethodBridge::Initialize();
    DelegateBridge::Initialize();
    RegisterMarshalBindingTables();
    LuaInternalCalls::RegisterCoreInternalCalls();

    LuaLoader::RegisterRoots();
    LuaEnv::Initialize();
}

void LuaAppDomain::InitializeFromManaged(Il2CppDelegate* moduleLoaderDelegate)
{
    if (moduleLoaderDelegate == nullptr)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("loader delegate is null"));
    }
    LuaLoader::SetModuleLoader(moduleLoaderDelegate);
}
} // namespace zlua
