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
