#pragma once

#include "il2cpp-class-internals.h"
#include "zlua/marshal/MarshalAsTypes.h"

struct MethodInfo;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppObject;

struct lua_State;

#include <string>

namespace zlua
{
    class MetadataUtil
    {
    public:
        static const Il2CppAssembly* ResolveAssembly(const char* luaAssemblyName);
        static Il2CppClass* ResolveType(const Il2CppAssembly* assembly, const char* typeName);
        static Il2CppClass* ResolveCorlibType(const char* typeFullName);
        static Il2CppClass* ResolveTypeArg(lua_State* L, int index);
        static const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int parameterCount, bool isStatic);
        static const MethodInfo* FindConstructor(Il2CppClass* klass, int parameterCount);
        static const MethodInfo* FindMethodByParameterSignature(
            Il2CppClass* klass, const char* name, const char* parameterSignature, bool isStatic);
        static Il2CppObject* CreateInstance(Il2CppClass* klass);
        static std::string GetLuaFullName(Il2CppClass* klass);
        static std::string GetSignatureTypeName(Il2CppClass* klass);
        static std::string FormatParameterSignature(const MethodInfo* method);
        static std::string BuildParameterSignature(const Il2CppClass* const* parameterTypes, int count);
        static const char* GetTypeFullName(Il2CppClass* klass);

        static bool TryReadLuaAlias(const MethodInfo* method, std::string& aliasOut);
        static bool MethodParameterHasParamArrayAttribute(const MethodInfo* method, int parameterIndex);
        static bool IsDelegateClass(Il2CppClass* klass);

        static LuaMarshalType ResolveParameterMarshalType(const MethodInfo* method, int paramIndex);
        static LuaMarshalType ResolveReturnMarshalType(const MethodInfo* method);
    };
}
