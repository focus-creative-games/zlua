#include "PropertyBridgeStub.h"

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

#include "vm/Class.h"
#include "vm/InternalCalls.h"

namespace zlua
{
namespace propertybridge
{
    const PropertyBridgeEntry g_propertyBridges[] = {
        { nullptr, nullptr, nullptr, nullptr, nullptr },
    };
} // namespace propertybridge
} // namespace zlua
