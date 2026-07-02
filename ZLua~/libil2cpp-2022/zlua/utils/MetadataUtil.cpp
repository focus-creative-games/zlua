#include "MetadataUtil.h"
#include "Collection.h"

#include "../LuaConsts.h"

#include "vm/Assembly.h"
#include "vm/Class.h"
#include "vm/MetadataCache.h"
#include "vm/Object.h"
#include "vm/Property.h"
#include "vm/Reflection.h"
#include "vm/Runtime.h"
#include "vm/Method.h"
#include "vm/Type.h"
#include "vm/Image.h"
#include "vm/GlobalMetadata.h"
#include "utils/StringUtils.h"
#include "utils/HashUtils.h"


namespace zlua
{

static Il2CppClass* s_paramArrayAttributeClass = nullptr;
static Il2CppClass* s_luaMarshalAsAttributeClass = nullptr;
static Il2CppClass* s_luaAliasAttributeClass = nullptr;
static Il2CppClass* s_luaExceptionClass = nullptr;
static Il2CppClass* s_luaMethodClass = nullptr;
static Il2CppClass* s_byteArrayClass = nullptr;
static const MethodInfo* s_ObjectToStringMethod = nullptr;

static void ResolveParamArrayAttributeClass()
{
    s_paramArrayAttributeClass = il2cpp::vm::Class::FromName(il2cpp::vm::Image::GetCorlib(), "System", "ParamArrayAttribute");
    IL2CPP_ASSERT(s_paramArrayAttributeClass != nullptr);
}

static void ResolveLuaClasses()
{
    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("ZLua.Common");
    IL2CPP_ASSERT(assembly != nullptr);
    s_luaAliasAttributeClass = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaAliasAttribute");
    IL2CPP_ASSERT(s_luaAliasAttributeClass != nullptr);
    s_luaExceptionClass = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaScriptException");
    IL2CPP_ASSERT(s_luaExceptionClass != nullptr);
    s_luaMarshalAsAttributeClass = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaMarshalAsAttribute");
    IL2CPP_ASSERT(s_luaMarshalAsAttributeClass != nullptr);
}

static void ResolveLuaMethodClass()
{
    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("ZLua.Il2Cpp");
    s_luaMethodClass = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaMethod");
    IL2CPP_ASSERT(s_luaMethodClass != nullptr);
    il2cpp::vm::Class::Init(s_luaMethodClass);
    IL2CPP_ASSERT(s_luaMethodClass->instance_size == sizeof(LuaMethod));
}

static void ResolveByteArrayClass()
{
    s_byteArrayClass = il2cpp::metadata::ArrayMetadata::GetBoundedArrayClass(il2cpp_defaults.byte_class, 1, false);
    IL2CPP_ASSERT(s_byteArrayClass != nullptr);
}

static void ResolveObjectToStringMethod()
{
    s_ObjectToStringMethod = il2cpp::vm::Class::GetMethodFromName(il2cpp_defaults.object_class, "ToString", 0);
    IL2CPP_ASSERT(s_ObjectToStringMethod != nullptr);
}

Il2CppClass* MetadataUtil::GetLuaMarshalAsAttributeClass()
{
    return s_luaMarshalAsAttributeClass;
}

Il2CppClass* MetadataUtil::GetLuaAliasAttributeClass()
{
    return s_luaAliasAttributeClass;
}

Il2CppClass* MetadataUtil::GetLuaExceptionClass()
{
    return s_luaExceptionClass;
}

Il2CppClass* MetadataUtil::GetLuaMethodClass()
{
    return s_luaMethodClass;
}

Il2CppClass* MetadataUtil::GetByteArrayClass()
{
    return s_byteArrayClass;
}

const MethodInfo* MetadataUtil::GetObjectToStringMethod()
{
    return s_ObjectToStringMethod;
}

void MetadataUtil::Initialize()
{
    ResolveParamArrayAttributeClass();
    ResolveLuaClasses();
    ResolveLuaMethodClass();
    ResolveByteArrayClass();
    ResolveObjectToStringMethod();
}

static void BuildLuaFullNameImpl(std::string& str, const Il2CppType* type)
{
    switch (type->type)
    {
    case IL2CPP_TYPE_ARRAY:
    {
        Il2CppClass* arrayClass = il2cpp::vm::Class::FromIl2CppType(type);
        Il2CppClass* elementClass = il2cpp::vm::Class::GetElementClass(arrayClass);
        BuildLuaFullNameImpl(str, &elementClass->byval_arg);
        str += '[';

        if (arrayClass->rank == 1)
            str += '*';

        for (int32_t i = 1; i < arrayClass->rank; i++)
            str += ',';

        str += ']';

        if (type->byref)
            str += '&';
        break;
    }

    case IL2CPP_TYPE_SZARRAY:
    {
        Il2CppClass* elementClass = il2cpp::vm::Class::FromIl2CppType(type->data.type);
        BuildLuaFullNameImpl(str, &elementClass->byval_arg);
        str += "[]";

        if (type->byref)
            str += '&';
        break;
    }

    case IL2CPP_TYPE_PTR:
    {
        BuildLuaFullNameImpl(str, type->data.type);
        str += '*';

        if (type->byref)
            str += '&';
        break;
    }

    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
    {
        str += il2cpp::vm::MetadataCache::GetGenericParameterName(il2cpp::vm::Type::GetGenericParameterHandle(type));
        if (type->byref)
            str += '&';
        break;
    }
    default:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        il2cpp::vm::Class::Init(klass);

        Il2CppClass* declaringType = il2cpp::vm::Class::GetDeclaringType(klass);
        if (declaringType)
        {
            BuildLuaFullNameImpl(str, &declaringType->byval_arg);
            str += '+';
        }
        else if (*klass->namespaze)
        {
            str += klass->namespaze;
            str += '.';
        }

        str += klass->name;

        if (klass->generic_class)
        {
            Il2CppGenericClass* gclass = klass->generic_class;
            const Il2CppGenericInst* inst = gclass->context.class_inst;
            str += '[';

            for (uint32_t i = 0; i < inst->type_argc; i++)
            {
                if (i)
                    str += ',';

                BuildLuaFullNameImpl(str, inst->type_argv[i]);
            }

            str += ']';
        }
        if (type->byref)
            str += '&';
        break;
    }
    }
}

const Il2CppAssembly* MetadataUtil::ResolveAssembly(const char* luaAssemblyName)
{
    return il2cpp::vm::Assembly::GetLoadedAssembly(luaAssemblyName);
}

Il2CppClass* MetadataUtil::ResolveType(const Il2CppAssembly* assembly, const char* typeName)
{
    IL2CPP_ASSERT(assembly != nullptr);
    IL2CPP_ASSERT(typeName != nullptr);

    const Il2CppImage* image = assembly->image;
    IL2CPP_ASSERT(image != nullptr);

    const char* nestSep = strrchr(typeName, '+');
    // fast path: no nested type
    if (nestSep == nullptr)
    {
        const char* dot = strrchr(typeName, '.');
        if (dot != nullptr)
        {
            std::string ns(typeName, dot - typeName);
            return il2cpp::vm::Class::FromName(image, ns.c_str(), dot + 1);
        }
        else
        {
            return il2cpp::vm::Class::FromName(image, "", typeName);
        }
    }

    std::string parentName(typeName, nestSep - typeName);
    Il2CppClass* parent = ResolveType(assembly, parentName.c_str());
    if (parent == nullptr)
        return nullptr;
    il2cpp::vm::Class::Init(parent);

    const char* childName = nestSep + 1;
    void* iter = nullptr;
    while (Il2CppClass* nestedType = il2cpp::vm::Class::GetNestedTypes(parent, &iter))
    {
        if (strcmp(nestedType->name, nestSep + 1) == 0)
            return nestedType;
    }
    return nullptr;
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

std::string MetadataUtil::GetLuaFullName(Il2CppClass* klass)
{
    return GetLuaFullName(&klass->byval_arg);
}

std::string MetadataUtil::GetLuaFullName(const Il2CppType* type)
{
    std::string fullName;
    BuildLuaFullNameImpl(fullName, type);
    return fullName;
}

std::string MetadataUtil::GetSignatureTypeName(Il2CppClass* klass)
{
    return GetLuaFullName(&klass->byval_arg);
}

std::string MetadataUtil::CreateMethodNameWithParametersAndGenericArguments(const MethodInfo* method)
{
    std::string name = method->name;
    if (method->is_generic)
    {
        name += "<";
        int32_t genericArgumentCount = MetadataUtil::GetMethodGenericParameterCount(method);
        IL2CPP_ASSERT(genericArgumentCount > 0);
        for (int32_t i = 1; i < genericArgumentCount; i++)
        {
            name += ',';
        }
        name += ">";
    }
    name += "(";
    for (int i = 0; i < method->parameters_count; i++)
    {
        if (i > 0)
        {
            name += ',';
        }
        name += MetadataUtil::GetLuaFullName(method->parameters[i]);
    }
    name += ")";
    return name;
}

static void AppendShortReducedClassMarshalTypeName(Il2CppClass* klass, std::string& result)
{
    if (klass->parent == il2cpp_defaults.multicastdelegate_class)
    {
        result += 'D';
    }
    else
    {
        result += 'O';
    }
}

// NameUtil.CreateShortReducedMarshalTypeName after MethodDesc ToShareTypeSig + ToSharedTypeSig.
static void AppendShortReducedMarshalTypeName(const Il2CppType* type, std::string& result)
{
    IL2CPP_ASSERT(type != nullptr);

    if (type->byref)
    {
        result += "&R";
        return;
    }

    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
        result += 'V';
        break;
    case IL2CPP_TYPE_BOOLEAN:
        result += "B";
        break;
    case IL2CPP_TYPE_CHAR:
        result += "U2";
        break;
    case IL2CPP_TYPE_I1:
        result += "I1";
        break;
    case IL2CPP_TYPE_U1:
        result += "U1";
        break;
    case IL2CPP_TYPE_I2:
        result += "I2";
        break;
    case IL2CPP_TYPE_U2:
        result += "U2";
        break;
    case IL2CPP_TYPE_I4:
        result += "I4";
        break;
    case IL2CPP_TYPE_U4:
        result += "U4";
        break;
    case IL2CPP_TYPE_I8:
        result += "I8";
        break;
    case IL2CPP_TYPE_U8:
        result += "U8";
        break;
    case IL2CPP_TYPE_R4:
        result += "R4";
        break;
    case IL2CPP_TYPE_R8:
        result += "R8";
        break;
    case IL2CPP_TYPE_I:
        result += 'I';
        break;
    case IL2CPP_TYPE_U:
        result += 'U';
        break;
    case IL2CPP_TYPE_TYPEDBYREF:
        result += "TypedByRef";
        break;
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        result += 'P';
        break;
    case IL2CPP_TYPE_OBJECT:
        result += 'O';
        break;
    case IL2CPP_TYPE_STRING:
        result += 'S';
        break;
    case IL2CPP_TYPE_SZARRAY:
    case IL2CPP_TYPE_ARRAY:
        result += 'A';
        break;
    case IL2CPP_TYPE_CLASS:
        AppendShortReducedClassMarshalTypeName(il2cpp::vm::Class::FromIl2CppType(type), result);
        break;
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
        result += 'O';
        break;
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        if (klass->enumtype)
        {
            AppendShortReducedMarshalTypeName(il2cpp::vm::Class::GetEnumBaseType(klass), result);
            break;
        }
        if (!il2cpp::vm::Class::IsValuetype(klass))
        {
            AppendShortReducedClassMarshalTypeName(klass, result);
            break;
        }
        result += '$';
        if (klass->generic_class == nullptr)
        {
            BuildLuaFullNameImpl(result, &klass->byval_arg);
        }
        else
        {
            const Il2CppGenericClass* genericClass = klass->generic_class;
            BuildLuaFullNameImpl(result, genericClass->type);
            result += '<';
            for (uint32_t i = 0; i < genericClass->context.class_inst->type_argc; ++i)
            {
                if (i > 0)
                    result += ',';
                AppendShortReducedMarshalTypeName(genericClass->context.class_inst->type_argv[i], result);
            }
            result += '>';
        }
        break;
    }
    default:
        IL2CPP_ASSERT(false && "unsupported type for CreateStubName");
        break;
    }
}

void MetadataUtil::CreateStubName(const MethodInfo* method, std::string& result)
{
    IL2CPP_ASSERT(method != nullptr);
    result.clear();
    AppendShortReducedMarshalTypeName(method->return_type, result);
    result += '(';
    for (uint16_t i = 0; i < method->parameters_count; ++i)
    {
        if (i > 0)
            result += ',';
        AppendShortReducedMarshalTypeName(method->parameters[i], result);
    }
    result += ')';
}

void MetadataUtil::CreateDelegateInvokeSignature(const MethodInfo* method, std::string& result)
{
    IL2CPP_ASSERT(method != nullptr);
    result.clear();
    BuildLuaFullNameImpl(result, method->return_type);
    result += '(';
    for (uint16_t i = 0; i < method->parameters_count; ++i)
    {
        if (i > 0)
            result += ',';
        BuildLuaFullNameImpl(result, method->parameters[i]);
    }
    result += ')';
}

void MetadataUtil::CreateMethodParameterSignatureOnly(const MethodInfo* method, std::string& signature)
{
    for (uint16_t i = 0; i < method->parameters_count; ++i)
    {
        if (i > 0)
            signature.push_back(',');
        signature += GetLuaFullName(method->parameters[i]);
    }
}

std::string MetadataUtil::CreateMethodParameterSignatureOnly(const MethodInfo* method)
{
    std::string signature;
    CreateMethodParameterSignatureOnly(method, signature);
    return signature;
}

std::string MetadataUtil::FormatParameterSignature(const MethodInfo* method)
{
    if (method == nullptr || method->parameters_count == 0)
        return "()";

    std::string signature = "(";
    for (int i = 0; i < method->parameters_count; ++i)
    {
        if (i > 0)
            signature.push_back(',');
        signature += GetLuaFullName(method->parameters[i]);
    }
    signature.push_back(')');
    return signature;
}

std::string MetadataUtil::BuildParameterSignature(const Il2CppType** parameterTypes, int count)
{
    if (parameterTypes == nullptr || count <= 0)
        return "()";

    std::string signature = "(";
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
            signature.push_back(',');
        signature += GetLuaFullName(parameterTypes[i]);
    }
    signature.push_back(')');
    return signature;
}

Il2CppClass* MetadataUtil::ResolveCorlibType(const char* typeFullName)
{
    const Il2CppAssembly* mscorlib = il2cpp::vm::Image::GetCorlib()->assembly;
    return ResolveType(mscorlib, typeFullName);
}

const MethodInfo* MetadataUtil::FindMethodByParameterSignature(Il2CppClass* klass, const char* name, const char* parameterSignature, bool isStatic)
{
    if (klass == nullptr || name == nullptr || parameterSignature == nullptr)
        return nullptr;

    il2cpp::vm::Class::Init(klass);
    for (Il2CppClass* cursor = klass; cursor != nullptr; cursor = cursor->parent)
    {
        for (uint16_t i = 0; i < cursor->method_count; ++i)
        {
            const MethodInfo* method = cursor->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if (strcmp(method->name, name) != 0)
                continue;
            const bool methodIsStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
            if (methodIsStatic != isStatic)
                continue;
            if (FormatParameterSignature(method) == parameterSignature)
                return method;
        }
    }
    return nullptr;
}

bool MetadataUtil::TryReadLuaAlias(const MethodInfo* method, std::string& aliasOut)
{
    aliasOut.clear();

    Il2CppClass* aliasClass = s_luaAliasAttributeClass;
    if (!il2cpp::vm::Method::HasAttribute(method, aliasClass))
        return false;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(method->klass->image, il2cpp::vm::Method::GetToken(method));
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, aliasClass);
    IL2CPP_ASSERT(attr != nullptr);
    const PropertyInfo* aliasProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "Alias");
    IL2CPP_ASSERT(aliasProperty != nullptr && aliasProperty->get != nullptr);

    Il2CppException* exc = nullptr;
    Il2CppObject* aliasValue = il2cpp::vm::Runtime::Invoke(aliasProperty->get, attr, nullptr, &exc);
    IL2CPP_ASSERT(exc == nullptr && aliasValue != nullptr && aliasValue->klass->byval_arg.type == IL2CPP_TYPE_STRING);
    Il2CppString* aliasStr = (Il2CppString*)aliasValue;
    aliasOut = il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(aliasStr), il2cpp::utils::StringUtils::GetLength(aliasStr));
    return !aliasOut.empty();
}

bool MetadataUtil::MethodParameterHasParamArrayAttribute(const MethodInfo* method, int parameterIndex)
{
    if (method == nullptr || parameterIndex < 0 || parameterIndex >= method->parameters_count)
        return false;

    Il2CppClass* paramArrayClass = s_paramArrayAttributeClass;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(method->klass->image, il2cpp::vm::Method::GetParameterToken(method, parameterIndex));
    return il2cpp::vm::Reflection::HasAttribute(handle, paramArrayClass);
}

const char* MetadataUtil::GetTypeFullName(Il2CppClass* klass)
{
    static thread_local std::string s_fullName;
    s_fullName = GetLuaFullName(&klass->byval_arg);
    return s_fullName.c_str();
}

bool MetadataUtil::IsDelegateClass(Il2CppClass* klass)
{
    return il2cpp::vm::Class::IsAssignableFrom(il2cpp_defaults.multicastdelegate_class, klass);
}

static Il2CppClass* ResolveLuaMarshalAsAttributeClass()
{
    static Il2CppClass* s_class = nullptr;
    if (s_class != nullptr)
        return s_class;

    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("ZLua.Common");
    if (assembly == nullptr)
        return nullptr;

    s_class = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaMarshalAsAttribute");
    return s_class;
}

static bool TryReadLuaMarshalTypeFromAttributeObject(Il2CppObject* attr, LuaMarshalType& marshalTypeOut)
{
    marshalTypeOut = LuaMarshalType::Default;
    if (attr == nullptr)
        return false;

    const PropertyInfo* marshalTypeProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "LuaMarshalType");
    if (marshalTypeProperty == nullptr || marshalTypeProperty->get == nullptr)
        return false;

    Il2CppException* exc = nullptr;
    Il2CppObject* enumValue = il2cpp::vm::Runtime::Invoke(marshalTypeProperty->get, attr, nullptr, &exc);
    if (exc != nullptr || enumValue == nullptr || !enumValue->klass->enumtype)
        return false;

    const int32_t rawValue = *reinterpret_cast<int32_t*>(il2cpp::vm::Object::Unbox(enumValue));
    if (rawValue < 0 || rawValue > static_cast<int32_t>(LuaMarshalType::ParamsTable))
        return false;

    marshalTypeOut = static_cast<LuaMarshalType>(rawValue);
    return marshalTypeOut != LuaMarshalType::Default;
}

static bool TryReadLuaMarshalTypeFromToken(const Il2CppImage* image, uint32_t token, LuaMarshalType& marshalTypeOut)
{
    marshalTypeOut = LuaMarshalType::Default;
    if (image == nullptr || token == 0)
        return false;

    Il2CppClass* marshalAsClass = ResolveLuaMarshalAsAttributeClass();
    if (marshalAsClass == nullptr)
        return false;

    Il2CppMetadataCustomAttributeHandle handle = il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(image, token);
    if (!il2cpp::vm::Reflection::HasAttribute(handle, marshalAsClass))
        return false;

    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, marshalAsClass);
    return TryReadLuaMarshalTypeFromAttributeObject(attr, marshalTypeOut);
}

static bool IsStructClass(Il2CppClass* klass)
{
    return klass != nullptr && klass->byval_arg.valuetype && !klass->enumtype;
}

static bool MarshalIsPrimitiveValueClass(Il2CppClass* klass)
{
    if (klass == nullptr || klass->enumtype)
        return false;

    switch (klass->byval_arg.type)
    {
    case IL2CPP_TYPE_BOOLEAN:
    case IL2CPP_TYPE_CHAR:
    case IL2CPP_TYPE_I1:
    case IL2CPP_TYPE_U1:
    case IL2CPP_TYPE_I2:
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_I4:
    case IL2CPP_TYPE_U4:
    case IL2CPP_TYPE_I8:
    case IL2CPP_TYPE_U8:
    case IL2CPP_TYPE_R4:
    case IL2CPP_TYPE_R8:
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U:
        return true;
    default:
        return false;
    }
}

static bool IsUserDataAllowed(Il2CppClass* klass)
{
    if (klass == nullptr || MetadataUtil::IsUnsupportedMarshalType(klass))
        return false;

    if (klass->byval_arg.type == IL2CPP_TYPE_PTR)
        return false;

    if (MarshalIsPrimitiveValueClass(klass))
        return true;
    if (klass->byval_arg.type == IL2CPP_TYPE_STRING)
        return true;
    if (MetadataUtil::IsIntPtrClass(klass) || MetadataUtil::IsUIntPtrClass(klass))
        return true;
    if (klass->enumtype)
        return true;
    if (IsStructClass(klass))
        return true;
    if (IL2CPP_CLASS_IS_ARRAY(klass))
        return true;
    if (klass->byval_arg.type == IL2CPP_TYPE_OBJECT)
        return true;
    if (MetadataUtil::IsDelegateClass(klass))
        return true;
    if (klass->byval_arg.type == IL2CPP_TYPE_CLASS)
        return true;
    if (il2cpp::vm::Class::IsInterface(klass))
        return true;

    return false;
}

static bool IsBytesAllowed(Il2CppClass* klass)
{
    if (klass == nullptr)
        return false;

    if (klass->byval_arg.type == IL2CPP_TYPE_STRING)
        return true;

    if (IL2CPP_CLASS_IS_ARRAY(klass))
    {
        Il2CppClass* elementClass = klass->element_class;
        return elementClass != nullptr && elementClass->byval_arg.type == IL2CPP_TYPE_U1;
    }

    return false;
}

static Il2CppClass* GetEffectiveMarshalClass(const Il2CppType* type)
{
    if (type == nullptr)
        return nullptr;

    Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type, false);
    if (klass == nullptr)
        return nullptr;

    il2cpp::vm::Class::Init(klass);
    if (il2cpp::vm::Class::IsNullable(klass))
        klass = il2cpp::vm::Class::GetNullableArgument(klass);

    return klass;
}

static bool IsMarshalTypeValidForParameter(LuaMarshalType marshalType, const Il2CppType* type, bool isReturnValue)
{
    Il2CppClass* klass = GetEffectiveMarshalClass(type);
    if (klass == nullptr)
        return false;

    switch (marshalType)
    {
    case LuaMarshalType::Default:
        return true;
    case LuaMarshalType::UserData:
        return IsUserDataAllowed(klass);
    case LuaMarshalType::Bytes:
        return IsBytesAllowed(klass);
    case LuaMarshalType::OpaqueLightUserData:
        return isReturnValue && IsStructClass(klass);
    case LuaMarshalType::Table:
    case LuaMarshalType::UnpackedValues:
        if (il2cpp::vm::Class::IsInterface(klass))
            return false;
        return IsStructClass(klass) || klass->byval_arg.type == IL2CPP_TYPE_CLASS;
    case LuaMarshalType::ParamsTable:
        return false;
    default:
        return false;
    }
}

static LuaMarshalType SanitizeDeclaredMarshalType(LuaMarshalType marshalType, const Il2CppType* type, bool isReturnValue)
{
    if (marshalType == LuaMarshalType::Default)
        return LuaMarshalType::Default;

    return IsMarshalTypeValidForParameter(marshalType, type, isReturnValue) ? marshalType : LuaMarshalType::Default;
}

static LuaMarshalType ReadDeclaredMarshalTypeFromToken(const MethodInfo* method, int parameterTokenIndex, const Il2CppType* type, bool isReturnValue)
{
    if (method == nullptr || method->klass == nullptr)
        return LuaMarshalType::Default;

    const uint32_t token = il2cpp::vm::Method::GetParameterToken(method, parameterTokenIndex);
    LuaMarshalType marshalType = LuaMarshalType::Default;
    if (!TryReadLuaMarshalTypeFromToken(method->klass->image, token, marshalType))
        return LuaMarshalType::Default;

    return SanitizeDeclaredMarshalType(marshalType, type, isReturnValue);
}

static LuaMarshalType ReadDeclaredMarshalTypeFromMethod(const MethodInfo* method, const Il2CppType* type, bool isReturnValue)
{
    if (method == nullptr || method->klass == nullptr)
        return LuaMarshalType::Default;

    Il2CppClass* marshalAsClass = ResolveLuaMarshalAsAttributeClass();
    if (marshalAsClass == nullptr || !il2cpp::vm::Method::HasAttribute(method, marshalAsClass))
        return LuaMarshalType::Default;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(method->klass->image, il2cpp::vm::Method::GetToken(method));
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, marshalAsClass);
    LuaMarshalType marshalType = LuaMarshalType::Default;
    if (!TryReadLuaMarshalTypeFromAttributeObject(attr, marshalType))
        return LuaMarshalType::Default;

    return SanitizeDeclaredMarshalType(marshalType, type, isReturnValue);
}

static LuaMarshalType ReadDeclaredMarshalTypeFromTypeClass(Il2CppClass* klass, const Il2CppType* type, bool isReturnValue)
{
    if (klass == nullptr)
        return LuaMarshalType::Default;

    il2cpp::vm::Class::Init(klass);
    Il2CppClass* marshalAsClass = ResolveLuaMarshalAsAttributeClass();
    if (marshalAsClass == nullptr || !il2cpp::vm::Class::HasAttribute(klass, marshalAsClass))
        return LuaMarshalType::Default;

    Il2CppMetadataCustomAttributeHandle handle = il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(klass->image, klass->token);
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, marshalAsClass);
    LuaMarshalType marshalType = LuaMarshalType::Default;
    if (!TryReadLuaMarshalTypeFromAttributeObject(attr, marshalType))
        return LuaMarshalType::Default;

    return SanitizeDeclaredMarshalType(marshalType, type, isReturnValue);
}

LuaMarshalType MetadataUtil::ResolveParameterMarshalType(const MethodInfo* method, int paramIndex)
{
    if (method == nullptr || paramIndex < 0 || paramIndex >= method->parameters_count)
        return LuaMarshalType::Default;

    const Il2CppType* paramType = method->parameters[paramIndex];
    Il2CppClass* paramClass = GetEffectiveMarshalClass(paramType);

    LuaMarshalType marshalType = ReadDeclaredMarshalTypeFromToken(method, paramIndex, paramType, false);
    if (marshalType != LuaMarshalType::Default)
        return marshalType;

    marshalType = ReadDeclaredMarshalTypeFromMethod(method, paramType, false);
    if (marshalType != LuaMarshalType::Default)
        return marshalType;

    return ReadDeclaredMarshalTypeFromTypeClass(paramClass, paramType, false);
}

LuaMarshalType MetadataUtil::ResolveReturnMarshalType(const MethodInfo* method)
{
    if (method == nullptr || method->return_type == nullptr || method->return_type->type == IL2CPP_TYPE_VOID)
        return LuaMarshalType::Default;

    const Il2CppType* returnType = method->return_type;
    Il2CppClass* returnClass = GetEffectiveMarshalClass(returnType);

    LuaMarshalType marshalType = ReadDeclaredMarshalTypeFromToken(method, -1, returnType, true);
    if (marshalType != LuaMarshalType::Default)
        return marshalType;

    marshalType = ReadDeclaredMarshalTypeFromMethod(method, returnType, true);
    if (marshalType != LuaMarshalType::Default)
        return marshalType;

    return ReadDeclaredMarshalTypeFromTypeClass(returnClass, returnType, true);
}

int32_t MetadataUtil::GetValueSize(const Il2CppType* type)
{
    if (type->byref)
    {
        return sizeof(void*);
    }

    switch (type->type)
    {
    case IL2CPP_TYPE_VOID:
        return 0;
    case IL2CPP_TYPE_BOOLEAN:
        return sizeof(bool);
    case IL2CPP_TYPE_CHAR:
        return sizeof(uint16_t);
    case IL2CPP_TYPE_I1:
        return sizeof(int8_t);
    case IL2CPP_TYPE_U1:
        return sizeof(uint8_t);
    case IL2CPP_TYPE_I2:
        return sizeof(int16_t);
    case IL2CPP_TYPE_U2:
        return sizeof(uint16_t);
    case IL2CPP_TYPE_I4:
        return sizeof(int32_t);
    case IL2CPP_TYPE_U4:
        return sizeof(uint32_t);
    case IL2CPP_TYPE_I8:
        return sizeof(int64_t);
    case IL2CPP_TYPE_U8:
        return sizeof(uint64_t);
    case IL2CPP_TYPE_R4:
        return sizeof(float);
    case IL2CPP_TYPE_R8:
        return sizeof(double);
    case IL2CPP_TYPE_I:
        return sizeof(intptr_t);
    case IL2CPP_TYPE_U:
        return sizeof(uintptr_t);
    case IL2CPP_TYPE_STRING:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        return sizeof(Il2CppArray*);
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        return sizeof(void*);
    case IL2CPP_TYPE_TYPEDBYREF:
        return sizeof(Il2CppTypedRef);
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        if (il2cpp::vm::Class::IsValuetype(klass))
        {
            return il2cpp::vm::Class::GetValueSize(klass, nullptr);
        }
        else
        {
            return sizeof(Il2CppObject*);
        }
    }
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_MVAR:
        return sizeof(void*);
    default:
        IL2CPP_ASSERT(false);
        return 0;
    }
}

bool MetadataUtil::IsReferenceType(const Il2CppType* type)
{
    return il2cpp::vm::Type::IsReference(type);
}

bool MetadataUtil::IsByteArrayClass(Il2CppClass* klass)
{
    return klass == MetadataUtil::GetByteArrayClass();
}

bool MetadataUtil::IsSzArrayClass(Il2CppClass* klass)
{
    return klass->byval_arg.type == IL2CPP_TYPE_SZARRAY;
}

bool MetadataUtil::IsSzOrMdArrayClass(Il2CppClass* klass)
{
    Il2CppTypeEnum type = klass->byval_arg.type;
    return type == IL2CPP_TYPE_SZARRAY || type == IL2CPP_TYPE_ARRAY;
}

bool MetadataUtil::IsIntPtrClass(Il2CppClass* klass)
{
    return klass == il2cpp_defaults.int_class;
}

bool MetadataUtil::IsUIntPtrClass(Il2CppClass* klass)
{
    return klass == il2cpp_defaults.uint_class;
}

bool MetadataUtil::IsPointerType(const Il2CppType* type)
{
    return type != nullptr && type->type == IL2CPP_TYPE_PTR;
}

static bool IsByRefLikeClass(Il2CppClass* klass)
{
    if (klass == nullptr || !klass->byval_arg.valuetype)
        return false;

    const char* fullName = MetadataUtil::GetTypeFullName(klass);
    if (fullName == nullptr)
        return false;

    return strncmp(fullName, "System.Span`", 12) == 0 || strncmp(fullName, "System.ReadOnlySpan`", 20) == 0;
}

static bool IsSystemDecimal(Il2CppClass* klass)
{
    return (klass->image == il2cpp_defaults.corlib && strcmp(klass->namespaze, "System") == 0 && strcmp(klass->name, "Decimal") == 0);
}

bool MetadataUtil::IsUnsupportedMarshalType(Il2CppClass* klass)
{
    if (klass == nullptr)
        return false;

    if (klass == il2cpp_defaults.typed_reference_class || IsSystemDecimal(klass))
        return true;

    return IsByRefLikeClass(klass);
}

void* MetadataUtil::GetNullableValue(void* dataAddr, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass->fields[0].type->type == IL2CPP_TYPE_BOOLEAN);
    return reinterpret_cast<uint8_t*>(dataAddr) + klass->fields[1].offset - sizeof(Il2CppObject);
}

void MetadataUtil::InitNullableValue(void* dataAddr, Il2CppClass* klass)
{
    std::memset(dataAddr, 0, klass->instance_size - sizeof(Il2CppObject));
}

void MetadataUtil::NullableSetHasValue(void* dataAddr, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass->fields[0].type->type == IL2CPP_TYPE_BOOLEAN);
    *(reinterpret_cast<uint8_t*>(dataAddr)) = 1;
}

uint32_t MetadataUtil::GetParameterToken(const MethodInfo* method, int paramIndex /* -1 for return type */)
{
    Il2CppMetadataMethodDefinitionHandle methodHandle = method->is_inflated ? method->genericMethod->methodDefinition->methodMetadataHandle : method->methodMetadataHandle;
    if (!methodHandle)
    {
        return 0;
    }
    if (paramIndex >= 0)
    {
        Il2CppMetadataParameterInfo paramInfo = il2cpp::vm::MetadataCache::GetParameterInfo(method->klass, methodHandle, paramIndex);
        return paramInfo.token;
    }
    else
    {
        return il2cpp::vm::MetadataCache::GetReturnParameterToken(methodHandle);
    }
}

bool MetadataUtil::HasParameterMarshalAsAttribute(const Il2CppImage* image, uint32_t token)
{
    Il2CppMetadataCustomAttributeHandle customAttributeHandle = il2cpp::vm::GlobalMetadata::GetCustomAttributeTypeToken(image, token);
    return il2cpp::vm::Reflection::HasAttribute(customAttributeHandle, s_luaMarshalAsAttributeClass);
}

} // namespace zlua
