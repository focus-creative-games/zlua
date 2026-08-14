#include "DelegateBridgeStub.h"

#include "../lvm/LuaEnv.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/ArrayMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/DelegateMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"

namespace zlua
{
namespace delegatebridge
{
    const DelegateBridgeEntry g_delegateBridges[] = {
        { nullptr, nullptr },
    };
} // namespace delegatebridge
} // namespace zlua
