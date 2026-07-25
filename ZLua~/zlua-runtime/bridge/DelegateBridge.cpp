#include "DelegateBridge.h"

#include "../marshal/MarshalDefs.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../lvm/LuaEnv.h"
#include "../mt/MetaBinding.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/Collection.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"

#include "../generated/DelegateBridgeStub.h"

#include "metadata/Il2CppTypeCompare.h"
#include "vm/Runtime.h"

namespace zlua
{
    AppendOnlyStringHashMap<Il2CppMethodPointer> s_nameToMethodPointer;
    AppendOnlyRawPointerHashMap<MethodInfo, const MethodMarshalCtx*> s_methodToMarshalCtx;

    void DelegateBridge::Initialize()
    {
        for (size_t i = 0; ; i++)
        {
            const DelegateBridgeEntry* entry = &delegatebridge::g_delegateBridges[i];
            if (entry->stubName == nullptr)
                break;
            s_nameToMethodPointer.insert({ entry->stubName, entry->methodPointer });
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
        s_methodToMarshalCtx.insert({ method, ctx });
        return ctx;
    }
}
