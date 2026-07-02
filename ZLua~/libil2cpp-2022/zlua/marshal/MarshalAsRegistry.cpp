#include "MarshalAsRegistry.h"

#include "../MetadataUtil.h"

#if __has_include("generated/MarshalBindings.inc")
#include "generated/MarshalBindings.inc"
#endif

namespace zlua
{
    void MarshalAsRegistry::RegisterGeneratedTables()
    {
#if defined(ZLUA_HAS_MARSHAL_BINDINGS)
        RegisterMarshalBindingTables();
#endif
    }

    LuaMarshalType MarshalAsRegistry::GetParameterMarshalType(const MethodInfo* method, int paramIndex)
    {
        const LuaMarshalSlotDesc* desc = GetParameterSlotDesc(method, paramIndex);
        if (desc != nullptr)
            return desc->marshalType;

        return MetadataUtil::ResolveParameterMarshalType(method, paramIndex);
    }

    const LuaMarshalSlotDesc* MarshalAsRegistry::GetParameterSlotDesc(const MethodInfo* method, int paramIndex)
    {
#if defined(ZLUA_HAS_MARSHAL_BINDINGS)
        if (method != nullptr)
            return LookupMarshalParameterSlot(method, paramIndex);
#else
        (void)method;
        (void)paramIndex;
#endif
        return nullptr;
    }

    LuaMarshalType MarshalAsRegistry::GetReturnMarshalType(const MethodInfo* method)
    {
        const LuaMarshalSlotDesc* desc = GetReturnSlotDesc(method);
        if (desc != nullptr)
            return desc->marshalType;

        return MetadataUtil::ResolveReturnMarshalType(method);
    }

    const LuaMarshalSlotDesc* MarshalAsRegistry::GetReturnSlotDesc(const MethodInfo* method)
    {
#if defined(ZLUA_HAS_MARSHAL_BINDINGS)
        if (method != nullptr)
            return LookupMarshalReturnSlot(method);
#else
        (void)method;
#endif
        return nullptr;
    }
}
