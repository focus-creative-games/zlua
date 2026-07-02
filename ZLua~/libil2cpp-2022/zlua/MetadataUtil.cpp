#include "MetadataUtil.h"

#include "marshal/MarshalAsTypes.h"
#include "marshal/PointerMarshal.h"
#include "il2cpp-config.h"
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
#include "il2cpp-tabledefs.h"
#include "utils/StringUtils.h"

#include "lua/lauxlib.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace zlua
{
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

static std::string BuildLuaFullName(Il2CppClass* klass)
{
    std::string fullName;
    BuildLuaFullNameImpl(fullName, &klass->byval_arg);
    return fullName;
}

static const Il2CppAssembly* ResolveMscorlibAssembly()
{
    return il2cpp::vm::Image::GetAssembly(il2cpp_defaults.corlib);
}

const Il2CppAssembly* MetadataUtil::ResolveAssembly(const char* luaAssemblyName)
{
    if (strcmp(luaAssemblyName, "mscorlib") == 0)
        return ResolveMscorlibAssembly();

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

std::string MetadataUtil::GetLuaFullName(Il2CppClass* klass)
{
    return BuildLuaFullName(klass);
}

static Il2CppClass* ResolveLuaAliasAttributeClass()
{
    static Il2CppClass* s_class = nullptr;
    if (s_class != nullptr)
        return s_class;

    const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly("ZLua.Common");
    if (assembly == nullptr)
        return nullptr;

    s_class = il2cpp::vm::Class::FromName(assembly->image, "ZLua", "LuaAliasAttribute");
    return s_class;
}

static Il2CppClass* ResolveParamArrayAttributeClass()
{
    static Il2CppClass* s_class = nullptr;
    if (s_class != nullptr)
        return s_class;
    s_class = il2cpp::vm::Class::FromName(ResolveMscorlibAssembly()->image, "System", "ParamArrayAttribute");
    return s_class;
}

std::string MetadataUtil::GetSignatureTypeName(Il2CppClass* klass)
{
    return BuildLuaFullName(klass);
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
        Il2CppClass* paramClass = il2cpp::vm::Class::FromIl2CppType(method->parameters[i]);
        signature += GetSignatureTypeName(paramClass);
    }
    signature.push_back(')');
    return signature;
}

std::string MetadataUtil::BuildParameterSignature(const Il2CppClass* const* parameterTypes, int count)
{
    if (parameterTypes == nullptr || count <= 0)
        return "()";

    std::string signature = "(";
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
            signature.push_back(',');
        signature += GetSignatureTypeName(const_cast<Il2CppClass*>(parameterTypes[i]));
    }
    signature.push_back(')');
    return signature;
}

Il2CppClass* MetadataUtil::ResolveCorlibType(const char* typeFullName)
{
    const Il2CppAssembly* mscorlib = ResolveMscorlibAssembly();
    return ResolveType(mscorlib, typeFullName);
}

Il2CppClass* MetadataUtil::ResolveTypeArg(lua_State* L, int index)
{
    if (lua_type(L, index) == LUA_TSTRING)
    {
        const char* typeName = lua_tostring(L, index);
        return ResolveCorlibType(typeName);
    }

    if (!lua_istable(L, index))
        return nullptr;

    lua_getfield(L, index, "__klass");
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return klass;
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
    if (method == nullptr)
        return false;

    Il2CppClass* aliasClass = ResolveLuaAliasAttributeClass();
    if (aliasClass == nullptr || !il2cpp::vm::Method::HasAttribute(method, aliasClass))
        return false;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(method->klass->image, il2cpp::vm::Method::GetToken(method));
    Il2CppObject* attr = il2cpp::vm::Reflection::GetCustomAttribute(handle, aliasClass);
    if (attr == nullptr)
        return false;

    const PropertyInfo* aliasProperty = il2cpp::vm::Class::GetPropertyFromName(attr->klass, "Alias");
    if (aliasProperty == nullptr || aliasProperty->get == nullptr)
        return false;

    Il2CppException* exc = nullptr;
    Il2CppObject* aliasValue = il2cpp::vm::Runtime::Invoke(aliasProperty->get, attr, nullptr, &exc);
    if (exc != nullptr || aliasValue == nullptr || aliasValue->klass->byval_arg.type != IL2CPP_TYPE_STRING)
        return false;

    Il2CppString* aliasStr = (Il2CppString*)aliasValue;
    aliasOut = il2cpp::utils::StringUtils::Utf16ToUtf8(il2cpp::utils::StringUtils::GetChars(aliasStr), il2cpp::utils::StringUtils::GetLength(aliasStr));
    return !aliasOut.empty();
}

bool MetadataUtil::MethodParameterHasParamArrayAttribute(const MethodInfo* method, int parameterIndex)
{
    if (method == nullptr || parameterIndex < 0 || parameterIndex >= method->parameters_count)
        return false;

    Il2CppClass* paramArrayClass = ResolveParamArrayAttributeClass();
    if (paramArrayClass == nullptr)
        return false;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(method->klass->image, il2cpp::vm::Method::GetParameterToken(method, parameterIndex));
    return il2cpp::vm::Reflection::HasAttribute(handle, paramArrayClass);
}

const char* MetadataUtil::GetTypeFullName(Il2CppClass* klass)
{
    static thread_local std::string s_fullName;
    s_fullName = BuildLuaFullName(klass);
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

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(image, token);
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
    if (klass == nullptr || PointerMarshal::IsUnsupportedMarshalType(klass))
        return false;

    if (klass->byval_arg.type == IL2CPP_TYPE_PTR)
        return false;

    if (MarshalIsPrimitiveValueClass(klass))
        return true;
    if (klass->byval_arg.type == IL2CPP_TYPE_STRING)
        return true;
    if (PointerMarshal::IsIntPtrClass(klass) || PointerMarshal::IsUIntPtrClass(klass))
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

static LuaMarshalType SanitizeDeclaredMarshalType(
    LuaMarshalType marshalType,
    const Il2CppType* type,
    bool isReturnValue)
{
    if (marshalType == LuaMarshalType::Default)
        return LuaMarshalType::Default;

    return IsMarshalTypeValidForParameter(marshalType, type, isReturnValue)
        ? marshalType
        : LuaMarshalType::Default;
}

static LuaMarshalType ReadDeclaredMarshalTypeFromToken(
    const MethodInfo* method,
    int parameterTokenIndex,
    const Il2CppType* type,
    bool isReturnValue)
{
    if (method == nullptr || method->klass == nullptr)
        return LuaMarshalType::Default;

    const uint32_t token = il2cpp::vm::Method::GetParameterToken(method, parameterTokenIndex);
    LuaMarshalType marshalType = LuaMarshalType::Default;
    if (!TryReadLuaMarshalTypeFromToken(method->klass->image, token, marshalType))
        return LuaMarshalType::Default;

    return SanitizeDeclaredMarshalType(marshalType, type, isReturnValue);
}

static LuaMarshalType ReadDeclaredMarshalTypeFromMethod(
    const MethodInfo* method,
    const Il2CppType* type,
    bool isReturnValue)
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

static LuaMarshalType ReadDeclaredMarshalTypeFromTypeClass(
    Il2CppClass* klass,
    const Il2CppType* type,
    bool isReturnValue)
{
    if (klass == nullptr)
        return LuaMarshalType::Default;

    il2cpp::vm::Class::Init(klass);
    Il2CppClass* marshalAsClass = ResolveLuaMarshalAsAttributeClass();
    if (marshalAsClass == nullptr || !il2cpp::vm::Class::HasAttribute(klass, marshalAsClass))
        return LuaMarshalType::Default;

    Il2CppMetadataCustomAttributeHandle handle =
        il2cpp::vm::MetadataCache::GetCustomAttributeTypeToken(klass->image, klass->token);
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
} // namespace zlua
