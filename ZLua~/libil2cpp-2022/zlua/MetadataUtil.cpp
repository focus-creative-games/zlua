#include "MetadataUtil.h"

#include "vm/Assembly.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "il2cpp-tabledefs.h"

#include <cstring>
#include <string>

namespace zlua
{
    const Il2CppAssembly* MetadataUtil::ResolveAssembly(const char* luaAssemblyName)
    {
        if (luaAssemblyName == nullptr || luaAssemblyName[0] == '\0')
            return nullptr;

        const Il2CppAssembly* assembly = il2cpp::vm::Assembly::GetLoadedAssembly(luaAssemblyName);
        if (assembly != nullptr)
            return assembly;

        assembly = il2cpp::vm::Assembly::Load(luaAssemblyName);
        if (assembly != nullptr)
            return assembly;

        std::string withDll = std::string(luaAssemblyName) + ".dll";
        return il2cpp::vm::Assembly::Load(withDll.c_str());
    }

    Il2CppClass* MetadataUtil::ResolveType(const Il2CppAssembly* assembly, const char* typeName)
    {
        if (assembly == nullptr || typeName == nullptr)
            return nullptr;

        Il2CppImage* image = il2cpp::vm::Assembly::GetImage(assembly);
        if (image == nullptr)
            return nullptr;

        Il2CppClass* klass = il2cpp::vm::Class::FromName(image, "", typeName);
        if (klass != nullptr)
            return klass;

        const char* dot = strrchr(typeName, '.');
        if (dot != nullptr)
        {
            std::string ns(typeName, dot - typeName);
            const char* shortName = dot + 1;
            klass = il2cpp::vm::Class::FromName(image, ns.c_str(), shortName);
        }

        return klass;
    }

    const MethodInfo* MetadataUtil::FindMethod(Il2CppClass* klass, const char* name, int parameterCount, bool isStatic)
    {
        if (klass == nullptr || name == nullptr)
            return nullptr;

        il2cpp::vm::Class::Init(klass);
        for (uint16_t i = 0; i < klass->method_count; ++i)
        {
            const MethodInfo* method = klass->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if (strcmp(method->name, name) != 0)
                continue;
            if (method->parameters_count != parameterCount)
                continue;
            const bool methodIsStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
            if (isStatic != methodIsStatic)
                continue;
            return method;
        }
        return nullptr;
    }

    const MethodInfo* MetadataUtil::FindConstructor(Il2CppClass* klass, int parameterCount)
    {
        return FindMethod(klass, ".ctor", parameterCount, false);
    }

    Il2CppObject* MetadataUtil::CreateInstance(Il2CppClass* klass)
    {
        if (klass == nullptr)
            return nullptr;
        il2cpp::vm::Class::Init(klass);
        return il2cpp::vm::Object::New(klass);
    }

    const char* MetadataUtil::GetTypeFullName(Il2CppClass* klass)
    {
        if (klass == nullptr)
            return "";
        if (klass->namespaze != nullptr && klass->namespaze[0] != '\0')
            return klass->name;
        return klass->name;
    }
}
