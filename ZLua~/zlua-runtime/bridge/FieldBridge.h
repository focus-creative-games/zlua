#pragma once

#include "../ZLuaCommon.h"

#include "../marshal/IntrinsicTypes.h"

namespace zlua
{
class FieldBridge
{
public:
    static void* ComputeStaticFieldAddress(const FieldInfo* field);
    static int32_t ComputeInstanceFieldOffset(const FieldInfo* field);
};
} // namespace zlua
