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
