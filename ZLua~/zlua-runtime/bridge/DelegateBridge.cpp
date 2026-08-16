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

#include "DelegateBridge.h"

#include "../marshal/MarshalDefs.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../lvm/LuaEnv.h"
#include "../mt/MetaBinding.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/CsStringHash.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"

#include "../generated/DelegateBridgeStub.h"

#include "metadata/Il2CppTypeCompare.h"
#include "vm/Runtime.h"

#include <unordered_map>

namespace zlua
{
std::unordered_map<const char*, Il2CppMethodPointer, CsStringHash, CsStringEqual> s_nameToMethodPointer;
std::unordered_map<const MethodInfo*, const MethodMarshalCtx*> s_methodToMarshalCtx;
std::unordered_map<const Il2CppClass*, const MethodMarshalCtx*> s_delegateKlassToInvokeCtx;

void DelegateBridge::Initialize()
{
    for (size_t i = 0;; i++)
    {
        const DelegateBridgeEntry* entry = &delegatebridge::g_delegateBridges[i];
        if (entry->stubName == nullptr)
            break;
        s_nameToMethodPointer.insert({entry->stubName, entry->methodPointer});
    }
}

static std::string s_tempStubName;

Il2CppMethodPointer DelegateBridge::Resolve(Il2CppClass* delegateClass)
{
    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
    MetadataUtil::CreateDelegateInvokeSignature(invokeMethod, s_tempStubName);
    auto it = s_nameToMethodPointer.find(s_tempStubName.c_str());
    if (it != s_nameToMethodPointer.end())
        return it->second;
    LuaException::ThrowFormat("Delegate bridge not found for %s", MetadataUtil::GetTypeFullName(delegateClass));
    return nullptr;
}

const MethodMarshalCtx* DelegateBridge::GetOrCreateMethodMarshalCtx(lua_State* L, const MethodInfo* method)
{
    auto it = s_methodToMarshalCtx.find(method);
    if (it != s_methodToMarshalCtx.end())
        return it->second;

    const MethodMarshalCtx* ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
    s_methodToMarshalCtx.insert({method, ctx});
    return ctx;
}

const MethodMarshalCtx* DelegateBridge::GetOrCreateInvokeMarshalCtx(lua_State* L, Il2CppClass* delegateClass)
{
    auto it = s_delegateKlassToInvokeCtx.find(delegateClass);
    if (it != s_delegateKlassToInvokeCtx.end())
        return it->second;

    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(delegateClass);
    IL2CPP_ASSERT(invokeMethod != nullptr);
    const MethodMarshalCtx* ctx = GetOrCreateMethodMarshalCtx(L, invokeMethod);
    s_delegateKlassToInvokeCtx.insert({delegateClass, ctx});
    return ctx;
}
} // namespace zlua
