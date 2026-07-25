#pragma once

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"

#include "vm/Class.h"
#include "vm/Object.h"

namespace zlua
{
class MetadataUtil
{
  public:
    static void Initialize();
    static const Il2CppAssembly* ResolveAssembly(const char* luaAssemblyName);
    static Il2CppClass* ResolveType(const Il2CppAssembly* assembly, const char* typeName);
    static Il2CppClass* ResolveCorlibType(const char* typeFullName);
    /// Same semantics as System.Type.GetType(string) / zlua.get_type_from_name.
    static Il2CppClass* ResolveTypeFromName(const char* typeFullName);
    static const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int parameterCount, bool isStatic);
    static const MethodInfo* FindConstructor(Il2CppClass* klass, int parameterCount);
    static const MethodInfo* FindMethodByParameterSignature(Il2CppClass* klass, const char* name, const char* parameterSignature, bool isStatic);
    static std::string GetLuaFullName(Il2CppClass* klass);
    static std::string GetLuaFullName(const Il2CppType* type);
    static std::string GetSignatureTypeName(Il2CppClass* klass);
    static std::string CreateMethodNameWithParametersAndGenericArguments(const MethodInfo* method);
    // Mirrors NameUtil.CreateStubName(MethodDef): MethodDesc share reduction + short marshal type tokens.
    static void CreateStubName(const MethodInfo* method, std::string& result);
    static void CreateDelegateInvokeSignature(const MethodInfo* method, std::string& result);
    static void CreateMethodParameterSignatureOnly(const MethodInfo* method, std::string& signature);
    static std::string CreateMethodParameterSignatureOnly(const MethodInfo* method);
    static std::string FormatParameterSignature(const MethodInfo* method);
    static std::string BuildParameterSignature(const Il2CppType** parameterTypes, int count);
    static const char* GetTypeFullName(Il2CppClass* klass);
    static const MethodInfo* GetObjectToStringMethod();
    static Il2CppClass* GetLuaMarshalAsAttributeClass();
    static Il2CppClass* GetLuaAliasAttributeClass();
    static Il2CppClass* GetLuaExceptionClass();
    static Il2CppClass* GetLuaMethodClass();
    static Il2CppClass* GetByteArrayClass();

    static bool TryReadLuaAlias(const MethodInfo* method, std::string& aliasOut);
    static bool MethodParameterHasParamArrayAttribute(const MethodInfo* method, int parameterIndex);
    static bool IsDelegateClass(Il2CppClass* klass);

    static LuaMarshalType ResolveParameterMarshalType(const MethodInfo* method, int paramIndex);
    static LuaMarshalType ResolveReturnMarshalType(const MethodInfo* method);

    static int32_t GetValueSize(const Il2CppType* type);

    static int32_t GetInstanceSizeWithoutHeader(Il2CppClass* klass)
    {
        IL2CPP_ASSERT(klass->size_inited);
        return klass->instance_size - sizeof(Il2CppObject);
    }

    static bool IsStaticMethod(const MethodInfo* method)
    {
        return method->flags & METHOD_ATTRIBUTE_STATIC;
    }

    /* Effectively sealed for virtual dispatch: non-virtual, method final, klass sealed, or by-val this. */
    static bool IsMethodSealed(const MethodInfo* method, bool byVal)
    {
        if (byVal)
            return true;
        const uint16_t flags = method->flags;
        if ((flags & METHOD_ATTRIBUTE_VIRTUAL) == 0)
            return true;
        if ((flags & METHOD_ATTRIBUTE_FINAL) != 0)
            return true;
        if ((method->klass->flags & TYPE_ATTRIBUTE_SEALED) != 0)
            return true;
        return false;
    }

    /* declared MethodInfo + runtime this → final MethodInfo (no-op when sealed). */
    static inline const MethodInfo* ResolveInvokeMethod(const MethodInfo* declared, void* target, bool sealed)
    {
        if (sealed)
            return declared;
        return il2cpp::vm::Object::GetVirtualMethod(reinterpret_cast<Il2CppObject*>(target), declared);
    }

    static bool IsStaticField(const FieldInfo* field)
    {
        return (field->type->attrs & FIELD_ATTRIBUTE_STATIC) != 0;
    }

    static bool IsStaticLiteralField(const FieldInfo* field)
    {
        return (field->type->attrs & FIELD_ATTRIBUTE_STATIC) != 0 && (field->type->attrs & FIELD_ATTRIBUTE_LITERAL) != 0;
    }

    static bool IsNormalStaticField(const FieldInfo* field)
    {
        return (field->type->attrs & FIELD_ATTRIBUTE_STATIC) != 0 && (field->type->attrs & FIELD_ATTRIBUTE_LITERAL) == 0;
    }

    static bool IsPublicMethod(const MethodInfo* method)
    {
        return (method->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK) == METHOD_ATTRIBUTE_PUBLIC;
    }

    static bool IsPublicProperty(const PropertyInfo* property)
    {
        if (property->get != nullptr && IsPublicMethod(property->get))
            return true;
        if (property->set != nullptr && IsPublicMethod(property->set))
            return true;
        //// Auto-implemented properties compile to private get_/set_ accessors; still bind by property name.
        // if (IsIndexerProperty(property))
        //     return false;
        return property->get != nullptr || property->set != nullptr;
    }

    static bool IsVoidType(const Il2CppType* type)
    {
        return type->type == IL2CPP_TYPE_VOID;
    }

    static bool IsValueTypeClass(Il2CppClass* klass)
    {
        return il2cpp::vm::Class::IsValuetype(klass);
    }

    static bool IsReferenceType(const Il2CppType* type);
    static bool IsByteArrayClass(Il2CppClass* klass);
    static bool IsSzArrayClass(Il2CppClass* klass);
    static bool IsSzOrMdArrayClass(Il2CppClass* klass);

    static bool IsIntPtrClass(Il2CppClass* klass);
    static bool IsUIntPtrClass(Il2CppClass* klass);
    static bool IsPointerType(const Il2CppType* type);
    static bool IsUnsupportedMarshalType(Il2CppClass* klass);

    static const Il2CppType* GetPropertyReturnType(const PropertyInfo* property)
    {
        return property->get != nullptr ? property->get->return_type : property->set->parameters[0];
    }

    static bool IsZeroParameterProperty(const PropertyInfo* property)
    {
        if (property->get != nullptr)
        {
            return property->get->parameters_count == 0;
        }
        if (property->set != nullptr)
        {
            return property->set->parameters_count == 1;
        }
        return false;
    }

    static bool IsStaticProperty(const PropertyInfo* property)
    {
        if (property->get)
        {
            return (property->get->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        }
        if (property->set != nullptr)
        {
            return (property->set->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        }
        return false;
    }

    static bool IsStaticEvent(const EventInfo* eventInfo)
    {
        if (eventInfo->add != nullptr)
        {
            return (eventInfo->add->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        }
        if (eventInfo->remove != nullptr)
        {
            return (eventInfo->remove->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        }
        if (eventInfo->raise != nullptr)
        {
            return (eventInfo->raise->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        }
        return false;
    }

    static bool IsCtorOrCCtor(const MethodInfo* method)
    {
        const char* name = method->name;
        return (std::strcmp(name, ".ctor") == 0 || std::strcmp(name, ".cctor") == 0);
    }

    static bool IsCtor(const MethodInfo* method)
    {
        return std::strcmp(method->name, ".ctor") == 0;
    }

    static bool IsCCtor(const MethodInfo* method)
    {
        return std::strcmp(method->name, ".cctor") == 0;
    }

    static void* GetNullableValue(void* dataAddr, Il2CppClass* klass);
    static void InitNullableValue(void* dataAddr, Il2CppClass* klass);
    static void NullableSetHasValue(void* dataAddr, Il2CppClass* klass);

    static int32_t GetMethodGenericParameterCount(const MethodInfo* method)
    {
        return method->is_inflated ? method->genericMethod->context.method_inst->type_argc
                                   : il2cpp::vm::MetadataCache::GetGenericContainerCount(method->genericContainerHandle);
    }

    static uint32_t GetParameterToken(const MethodInfo* method, int paramIndex /* -1 for return type */);
    static bool HasParameterMarshalAsAttribute(const Il2CppImage* image, uint32_t token);
};
} // namespace zlua
