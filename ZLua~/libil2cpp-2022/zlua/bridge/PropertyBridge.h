#pragma once

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"

namespace zlua
{


struct PropertyAccessor
{
    FnPropertyGetter getter;
    FnPropertySetter setter;
};

class PropertyBridge
{
  public:
    static void Initialize();
    static PropertyAccessor ResolvePropertyAccessor(const PropertyInfo* property, bool isStatic);
};
} // namespace zlua
