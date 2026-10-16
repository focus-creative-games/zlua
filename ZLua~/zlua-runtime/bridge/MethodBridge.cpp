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
#include "../marshal/ArrayMarshal.h"
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

        if (ctx->paramsParamIndex >= 0 && static_cast<int32_t>(i) == ctx->paramsParamIndex)
        {
            // params T[]: expand remaining slots, or pass through matching szarray userdata.
            IL2CPP_ASSERT(paramMeta->passByValue);
            IL2CPP_ASSERT(paramMeta->typeKlass != nullptr && paramMeta->typeKlass->rank == 1);
            const int available = top >= slot ? top - slot + 1 : 0;
            Il2CppArray* packed = nullptr;
            if (available == 0)
            {
                packed = ArrayMarshal::PopParamsExpand(L, slot, 0, paramMeta->typeKlass);
            }
            else if (available == 1 && ArrayMarshal::LooksLikeArrayUserData(L, slot, paramMeta->typeKlass))
            {
                packed = ArrayMarshal::PopFromArrayObject(L, slot, paramMeta->typeKlass);
                slot += 1;
            }
            else
            {
                packed = ArrayMarshal::PopParamsExpand(L, slot, available, paramMeta->typeKlass);
                slot += available;
            }
            params[i] = packed;
            continue;
        }

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

    // HybridCLR: an interpreted method's methodPointer/invoker_method are filled in
    // lazily. libil2cpp does the same before its own invoker_method calls (see
    // vm::Runtime::InvokeWithThrow). No-op without HybridCLR, and for AOT methods.
    ZLuaEnsureInvokable(method);

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
    // HybridCLR: hot-update methods run on the interpreter and have no AOT entry
    // point, so a specialized stub -- which casts methodPointer to the method's
    // concrete native signature and calls it directly -- would be an ABI mismatch.
    // Force the generic invoker_method path. Checked before the methodPointer tests
    // below because at this point an interpreted method's pointer may still be an
    // unresolved stub; this call also initializes it.
    if (ZLuaRequiresGenericInvoke(method))
    {
        return DefaultInvokeLuaMethod;
    }

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
