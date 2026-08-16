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

#include "XmlBindingUtil.h"

#include "MetadataUtil.h"

#include "vm/GenericClass.h"

namespace zlua
{
namespace XmlBindingUtil
{
const MethodInfo* NormalizeMethodForToken(const MethodInfo* method)
{
    if (method->is_inflated && method->genericMethod != nullptr && method->genericMethod->methodDefinition != nullptr)
        return method->genericMethod->methodDefinition;

    if (method->klass->generic_class != nullptr)
    {
        Il2CppClass* typeDef = il2cpp::vm::GenericClass::GetTypeDefinition(method->klass->generic_class);
        MetadataUtil::EnsureMethods(typeDef);
        for (uint16_t i = 0; i < typeDef->method_count; ++i)
        {
            const MethodInfo* m = typeDef->methods[i];
            if (m->token == method->token)
                return m;
        }
    }

    return method;
}
} // namespace XmlBindingUtil
} // namespace zlua
