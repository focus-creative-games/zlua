#include "FieldBridge.h"

#include "vm/Class.h"
#include "vm/Field.h"

namespace zlua
{

void* FieldBridge::ComputeStaticFieldAddress(const FieldInfo* field)
{
    il2cpp::vm::Class::Init(field->parent);
    return (uint8_t*)field->parent->static_fields + field->offset;
}

int32_t FieldBridge::ComputeInstanceFieldOffset(const FieldInfo* field)
{
    int32_t offset = (int32_t)il2cpp::vm::Field::GetOffset(const_cast<FieldInfo*>(field));
    if (field->parent->byval_arg.valuetype)
        offset -= (int32_t)sizeof(Il2CppObject);
    return offset;
}

} // namespace zlua
