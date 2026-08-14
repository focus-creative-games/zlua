#include "MethodBridge.h"

#include "../generated/MethodBridgeStub.h"
#include "../utils/Collection.h"
#include "../utils/MetadataUtil.h"

#include "metadata/GenericMethod.h"
#include "vm/Reflection.h"
#include "vm/MetadataCache.h"
#include "vm/GlobalMetadata.h"

namespace zlua
{

static AppendOnlyStringHashMap<FnLua2CsInvoker> s_name2Invokers;

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

int MethodBridge::DefaultInvokeLuaMethod(lua_State* L, void* target, int argStart, const MethodInfo* method, const MethodMarshalCtx* ctx)
{
    void** params = (void**)alloca(method->parameters_count * sizeof(void*));
    int slot = argStart;
    for (uint8_t i = 0; i < method->parameters_count; i++)
    {
        const MarshalMetaInfo* paramMeta = ctx->paramsMeta[i];
        IL2CPP_ASSERT(!paramMeta->passByValue || paramMeta->size == sizeof(void*));
        void* tempStorage = nullptr;
        void* storage = paramMeta->passByValue ? &tempStorage : alloca(paramMeta->size);
        paramMeta->lua2csWriter(L, slot, storage, paramMeta);
        params[i] = paramMeta->passByValue ? tempStorage : storage;
        slot += paramMeta->stackSlots > 0 ? paramMeta->stackSlots : 1;
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
    if (method->methodPointer == nullptr || il2cpp::metadata::GenericMethod::IsAnUnresolvedCallStubWasNotFound(method->methodPointer) ||
        DoesAnyParameterOrReturnTypeHaveNotDefaultMarshal(method))
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
