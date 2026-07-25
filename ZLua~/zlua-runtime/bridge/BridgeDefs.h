#pragma once

#include "../marshal/MarshalDefs.h"

namespace zlua
{
struct PropertyBridgeEntry
{
    const char* stubName;
    FnPropertyGetter staticGetter;
    FnPropertySetter staticSetter;
    FnPropertyGetter instanceGetter;
    FnPropertySetter instanceSetter;
};

struct MethodBridgeEntry
{
    const char* stubName;
    FnLua2CsInvoker lua2CsInvoker;
};

struct DelegateBridgeEntry
{
    const char* stubName;
    Il2CppMethodPointer methodPointer;
};
} // namespace zlua