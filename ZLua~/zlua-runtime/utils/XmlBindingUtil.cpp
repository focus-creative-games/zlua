#include "XmlBindingUtil.h"

#include "vm/Class.h"
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
        il2cpp::vm::Class::Init(typeDef);
        void* iter = nullptr;
        while (const MethodInfo* m = il2cpp::vm::Class::GetMethods(typeDef, &iter))
        {
            if (m->token == method->token)
                return m;
        }
    }

    return method;
}
} // namespace XmlBindingUtil
} // namespace zlua
