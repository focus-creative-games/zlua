#pragma once

#include "il2cpp-class-internals.h"

struct MethodInfo;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppObject;

namespace zlua
{
    class MetadataUtil
    {
    public:
        static const Il2CppAssembly* ResolveAssembly(const char* luaAssemblyName);
        static Il2CppClass* ResolveType(const Il2CppAssembly* assembly, const char* typeName);
        static const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int parameterCount, bool isStatic);
        static const MethodInfo* FindConstructor(Il2CppClass* klass, int parameterCount);
        static Il2CppObject* CreateInstance(Il2CppClass* klass);
        static const char* GetTypeFullName(Il2CppClass* klass);
    };
}
