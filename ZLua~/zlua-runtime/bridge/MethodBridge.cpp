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

#include "MethodBridge.h"

#include "../generated/MethodBridgeStub.h"
#include "../utils/CsStringHash.h"
#include "../utils/MetadataUtil.h"

#include <cstring>
#include <unordered_map>

#include "vm/Reflection.h"
#include "vm/MetadataCache.h"
#include "vm/GlobalMetadata.h"
#include "vm/Method.h"
#include "vm/Object.h"

namespace zlua
{

static std::unordered_map<const char*, FnLua2CsInvoker, CsStringHash, CsStringEqual> s_name2Invokers;

void MethodBridge::Initialize()
{
    for (size_t i = 0;; i++)
    {
        const MethodBridgeEntry& entry = methodbridge::g_methodBridges[i];
        if (entry.stubName == nullptr)
        {
            break;
        }
        s_name2Invokers.insert({entry.stubName, entry.lua2CsInvoker});
    }
}

static void FillDefaultParam(const MethodMarshalCtx* ctx, uint8_t paramIndex, void** outParam, void* allocaStorage, void** passByValueTemp)
{
    IL2CPP_ASSERT(ParamHasCachedDefault(ctx, paramIndex));
    const MethodDefaultArgs* defaults = ctx->defaults;
    const uint8_t slot = DefaultSlotIndex(defaults, paramIndex);
    const MarshalMetaInfo* paramMeta = ctx->paramsMeta[paramIndex];
    if (paramMeta->passByValue)
    {
        IL2CPP_ASSERT(defaults->defaultObjectSlots != nullptr);
        *passByValueTemp = defaults->defaultObjectSlots[slot];
        *outParam = *passByValueTemp;
        return;
    }

    IL2CPP_ASSERT(defaults->defaultValueSlots != nullptr && defaults->defaultValueSlots[slot] != nullptr);
    std::memcpy(allocaStorage, defaults->defaultValueSlots[slot], static_cast<size_t>(paramMeta->size));
    *outParam = allocaStorage;
}

int MethodBridge::DefaultInvokeLuaMethod(lua_State* L, void* target, int argStart, const MethodInfo* method, const MethodMarshalCtx* ctx)
{
    void** params = (void**)alloca(method->parameters_count * sizeof(void*));
    const int top = lua_gettop(L);
    int slot = argStart;

    for (uint8_t i = 0; i < method->parameters_count; i++)
    {
        const MarshalMetaInfo* paramMeta = ctx->paramsMeta[i];
        IL2CPP_ASSERT(!paramMeta->passByValue || paramMeta->size == sizeof(void*));
        void* tempStorage = nullptr;
        void* storage = paramMeta->passByValue ? &tempStorage : alloca(paramMeta->size);

        const int need = paramMeta->stackSlots > 0 ? paramMeta->stackSlots : 1;
        const int available = top >= slot ? top - slot + 1 : 0;
        if (available >= need)
        {
            paramMeta->lua2csWriter(L, slot, storage, paramMeta);
            params[i] = paramMeta->passByValue ? tempStorage : storage;
            slot += need;
        }
        else
        {
            FillDefaultParam(ctx, i, &params[i], storage, &tempStorage);
        }
    }

    if (ctx->retMeta != nullptr)
    {
        void* ret = alloca(ctx->retMeta->size);
        method->invoker_method(method->methodPointer, method, target, params, ret);
        ctx->retMeta->cs2luaWriter(L, ret, ctx->retMeta);
        return ctx->retMeta->stackSlots > 0 ? ctx->retMeta->stackSlots : 1;
    }
    else
    {
        method->invoker_method(method->methodPointer, method, target, params, nullptr);
        return 0;
    }
}

static std::string s_methodNameCache;

/// we only need to check LuaMarshalAsAttribute on the method parameters and return type,
/// not on the method itself or the method's declaring type. because we have considered the
//  LuaMarshalAsAttribute on the method itself and the method's declaring type in generating
//  the method bridge code.
static bool DoesAnyParameterOrReturnTypeHaveNotDefaultMarshal(const MethodInfo* method)
{
    const Il2CppImage* image = method->klass->image;
    for (uint8_t i = 0; i < method->parameters_count; i++)
    {
        uint32_t token = MetadataUtil::GetParameterToken(method, i);
        if (token != 0 && MetadataUtil::HasParameterMarshalAsAttribute(image, token))
        {
            return true;
        }
    }
    uint32_t returnToken = MetadataUtil::GetParameterToken(method, -1);
    if (returnToken != 0 && MetadataUtil::HasParameterMarshalAsAttribute(image, returnToken))
    {
        return true;
    }
    return false;
}

FnLua2CsInvoker MethodBridge::ResolveMethodInvoker(const MethodInfo* method)
{
    if (method->methodPointer == nullptr || ZLuaIsAnUnresolvedCallStubWasNotFound(method->methodPointer) || DoesAnyParameterOrReturnTypeHaveNotDefaultMarshal(method))
    {
        return DefaultInvokeLuaMethod;
    }
#if ZLUA_UNITY_VERSION < 20220000
    if (il2cpp::vm::Method::HasFullGenericSharingSignature(method))
    {
        return DefaultInvokeLuaMethod;
    }
#endif
    s_methodNameCache.clear();
    MetadataUtil::CreateStubName(method, s_methodNameCache);
    auto it = s_name2Invokers.find(s_methodNameCache.c_str());
    if (it != s_name2Invokers.end())
    {
        return it->second;
    }
    return DefaultInvokeLuaMethod;
}

} // namespace zlua
