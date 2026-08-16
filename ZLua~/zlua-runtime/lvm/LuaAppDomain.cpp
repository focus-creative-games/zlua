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

#include "LuaAppDomain.h"

#include "LuaEnv.h"
#include "LuaInternalCalls.h"
#include "LuaLoader.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaMetadataAlloc.h"
#include "../utils/LuaException.h"
#include "../bridge/PropertyBridge.h"
#include "../bridge/MethodBridge.h"
#include "../bridge/DelegateBridge.h"
#include "../generated/MarshalBindings.h"
#include "../generated/CompositeSpecializedBindings.h"
#include "../generated/AliasBindings.h"
#include "../generated/ExtensionBindings.h"

#include "vm/Exception.h"

namespace zlua
{
static bool s_processInitialized = false;
static bool s_hostInitialized = false;

void LuaAppDomain::InitializeProcessOnce()
{
    if (s_processInitialized)
    {
        return;
    }

    LuaMetadataAlloc::Initialize();
    MetadataUtil::Initialize();
    PropertyBridge::Initialize();
    MethodBridge::Initialize();
    DelegateBridge::Initialize();
    RegisterMarshalBindingTables();
    RegisterCompositeSpecializedWriters();
    RegisterAliasBindingTables();
    RegisterExtensionBindingTables();
    LuaInternalCalls::RegisterCoreInternalCalls();
    LuaLoader::RegisterRoots();
    s_processInitialized = true;
}

void LuaAppDomain::InitializeState()
{
    IL2CPP_ASSERT(LuaEnv::GetState() == nullptr);
    LuaEnv::Initialize();
}

void LuaAppDomain::ShutdownState()
{
    if (LuaEnv::GetState() == nullptr)
    {
        s_hostInitialized = false;
        return;
    }

    LuaEnv::Shutdown();
    s_hostInitialized = false;
}

void LuaAppDomain::Initialize()
{
    InitializeProcessOnce();
    if (LuaEnv::GetState() == nullptr)
    {
        InitializeState();
    }
}

void LuaAppDomain::InitializeFromManaged(Il2CppDelegate* moduleLoaderDelegate)
{
    if (moduleLoaderDelegate == nullptr)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("loader delegate is null"));
    }

    InitializeProcessOnce();
    if (LuaEnv::GetState() == nullptr)
    {
        InitializeState();
    }
    else if (s_hostInitialized)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetInvalidOperationException(
            "ZLua is already initialized. Call LuaAppDomain.Reset to rebuild the Lua domain."));
    }

    LuaLoader::SetModuleLoader(moduleLoaderDelegate);
    s_hostInitialized = true;
}

void LuaAppDomain::ResetFromManaged(Il2CppDelegate* moduleLoaderDelegate)
{
    if (moduleLoaderDelegate == nullptr)
    {
        il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("loader delegate is null"));
    }

    InitializeProcessOnce();
    ShutdownState();
    InitializeState();
    LuaLoader::SetModuleLoader(moduleLoaderDelegate);
    s_hostInitialized = true;
}
} // namespace zlua
