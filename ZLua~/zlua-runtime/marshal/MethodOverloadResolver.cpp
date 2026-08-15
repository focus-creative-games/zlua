#include "MethodOverloadResolver.h"
#include "MarshalDefs.h"
#include "MarshalMeta.h"
#include "PrimitiveMarshal.h"

#include "../mt/MetaBinding.h"
#include "../mt/InstanceTarget.h"
#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"

#include "vm/Class.h"
#include "il2cpp-class-internals.h"

#include <climits>

namespace zlua
{

static ConversionKind GetConversionKind(lua_State* L, int index, const MarshalMetaInfo* paramMeta)
{
    int luaType = lua_type(L, index);
    const Il2CppType* paramType = paramMeta->type;
    if (paramType->byref)
    {
        // Lua→C# byref: Opaque lightuserdata, ByVal payload of A, or temporary-copyable by-val forms.
        if (luaType == LUA_TLIGHTUSERDATA)
            return ConversionKind::Identity;

        Il2CppType elemType = *paramType;
        elemType.byref = false;
        Il2CppClass* elemKlass = il2cpp::vm::Class::FromIl2CppType(&elemType);
        il2cpp::vm::Class::Init(elemKlass);

        if (luaType == LUA_TUSERDATA)
        {
            UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
            if (userDataInfo.klass == elemKlass && userDataInfo.kind == UserDataKind::ByVal)
                return ConversionKind::Identity;
        }

        MarshalMetaInfo elemMeta = {};
        elemMeta.type = &elemKlass->byval_arg;
        elemMeta.typeKlass = elemKlass;
        elemMeta.marshalType = LuaMarshalType::Default;
        elemMeta.stackSlots = 1;
        return GetConversionKind(L, index, &elemMeta);
    }
    switch (paramType->type)
    {
    case IL2CPP_TYPE_VOID:
    {
        if (luaType == LUA_TNIL)
        {
            return ConversionKind::Identity;
        }
        break;
    }
    case IL2CPP_TYPE_BOOLEAN:
    {
        if (luaType == LUA_TBOOLEAN)
        {
            return ConversionKind::Identity;
        }
        break;
    }
    case IL2CPP_TYPE_CHAR:
    case IL2CPP_TYPE_I1:
    case IL2CPP_TYPE_U1:
    case IL2CPP_TYPE_I2:
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_I4:
    case IL2CPP_TYPE_U4:
    {
        if (luaType == LUA_TNUMBER)
        {
            return lua_isinteger(L, index) ? ConversionKind::Identity : ConversionKind::ImplicitNumeric;
        }
        break;
    }
    case IL2CPP_TYPE_I8:
    case IL2CPP_TYPE_U8:
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U:
    {
        if (luaType == LUA_TNUMBER)
        {
            return ConversionKind::ImplicitExtendedInteger;
        }
        break;
    }
    case IL2CPP_TYPE_R4:
    case IL2CPP_TYPE_R8:
    {
        if (luaType == LUA_TNUMBER)
        {
            return lua_isinteger(L, index) ? ConversionKind::ImplicitNumeric : ConversionKind::Identity;
        }
        break;
    }
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
    {
        if (luaType == LUA_TLIGHTUSERDATA)
        {
            return ConversionKind::Identity;
        }
        if (luaType == LUA_TNIL)
        {
            return ConversionKind::NullLiteral;
        }
        break;
    }
    case IL2CPP_TYPE_STRING:
    {
        if (luaType == LUA_TSTRING)
        {
            return ConversionKind::Identity;
        }
        if (luaType == LUA_TNIL)
        {
            return ConversionKind::NullLiteral;
        }
        if (luaType == LUA_TUSERDATA)
        {
            UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
            if (paramMeta->typeKlass == userDataInfo.klass)
            {
                IL2CPP_ASSERT(userDataInfo.kind == UserDataKind::ByObj);
                return ConversionKind::Identity;
            }
        }
        break;
    }
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_CLASS:
    handle_reference_type:
    {
        if (luaType == LUA_TUSERDATA)
        {
            UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
            if (paramMeta->typeKlass == userDataInfo.klass)
            {
                if (userDataInfo.kind == UserDataKind::ByVal)
                {
                    return ConversionKind::ImplicitBoxing;
                }
                if (userDataInfo.kind == UserDataKind::ByObj)
                {
                    return ConversionKind::Identity;
                }
            }
            else if (il2cpp::vm::Class::IsAssignableFrom(paramMeta->typeKlass, userDataInfo.klass))
            {
                if (userDataInfo.kind == UserDataKind::ByVal)
                {
                    return ConversionKind::ImplicitBoxing;
                }
                if (userDataInfo.kind == UserDataKind::ByObj)
                {
                    return ConversionKind::ImplicitReference;
                }
            }
        }
        else if (luaType == LUA_TNIL)
        {
            return ConversionKind::NullLiteral;
        }
        else if (luaType == LUA_TFUNCTION)
        {
            // Lua function → any Delegate (Action / Action<T> both applicable → Ambiguous).
            if (MetadataUtil::IsDelegateClass(paramMeta->typeKlass))
            {
                return ConversionKind::ImplicitReference;
            }
        }
        // Spec §3.3: primitive / string → object (ImplicitBoxing). Not to arbitrary class/interface.
        if (paramMeta->typeKlass == il2cpp_defaults.object_class)
        {
            if (luaType == LUA_TNUMBER || luaType == LUA_TBOOLEAN || luaType == LUA_TSTRING)
                return ConversionKind::ImplicitBoxing;
        }
        break;
    }
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
    {
        if (luaType == LUA_TUSERDATA)
        {
            UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
            if (paramMeta->typeKlass == userDataInfo.klass)
            {
                return ConversionKind::Identity;
            }
            else if (il2cpp::vm::Class::IsAssignableFrom(paramMeta->typeKlass, userDataInfo.klass))
            {
                return ConversionKind::ImplicitReference;
            }
        }
        else if (luaType == LUA_TTABLE)
        {
            return ConversionKind::ImplicitArray;
        }
        else if (luaType == LUA_TNIL)
        {
            return ConversionKind::NullLiteral;
        }
        break;
    }
    case IL2CPP_TYPE_VALUETYPE:
    handle_value_type:
    {
        Il2CppClass* klass = paramMeta->typeKlass;
        if (klass->enumtype)
        {
            if (luaType == LUA_TNUMBER)
            {
                return ConversionKind::ImplicitEnum;
            }
        }
        else if (klass->nullabletype)
        {
            if (luaType == LUA_TNIL)
            {
                return ConversionKind::NullLiteral;
            }
            // Match against Nullable<T> userdata or underlying T (temporary / ByVal T).
            if (luaType == LUA_TUSERDATA)
            {
                UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
                if (userDataInfo.klass == klass)
                {
                    if (userDataInfo.kind == UserDataKind::ByVal)
                        return ConversionKind::Identity;
                    if (userDataInfo.kind == UserDataKind::ByObj)
                        return ConversionKind::ImplicitBoxing;
                }
            }
            MarshalMetaInfo underlyingMeta = {};
            underlyingMeta.type = &klass->element_class->byval_arg;
            underlyingMeta.typeKlass = klass->element_class;
            underlyingMeta.marshalType = LuaMarshalType::Default;
            underlyingMeta.stackSlots = 1;
            return GetConversionKind(L, index, &underlyingMeta);
        }
        if (luaType == LUA_TUSERDATA)
        {
            UserDataInfo userDataInfo = InstanceTarget::GetUserDataInfo(L, index);
            if (paramMeta->typeKlass == userDataInfo.klass)
            {
                if (userDataInfo.kind == UserDataKind::ByVal)
                {
                    return ConversionKind::Identity;
                }
                if (userDataInfo.kind == UserDataKind::ByObj)
                {
                    return ConversionKind::ImplicitBoxing;
                }
            }
        }
        break;
    }
    case IL2CPP_TYPE_GENERICINST:
    {
        if (il2cpp::vm::Class::IsValuetype(paramMeta->typeKlass))
        {
            goto handle_value_type;
        }
        else
        {
            goto handle_reference_type;
        }
        break;
    }
    case IL2CPP_TYPE_TYPEDBYREF:
    {
        return ConversionKind::NotConvertible;
    }
    default:
    {
        LuaException::ThrowFormat("zlua: unsupported type: %d", paramType->type);
    }
    }

    return ConversionKind::NotConvertible;
}

struct ConversionScore
{
    ConversionKind kind;
    int score;
};

static ConversionKind GetCompositeConversionKind(lua_State* L, int32_t slot, const MarshalMetaInfo* paramMeta)
{
    if (paramMeta->marshalType == LuaMarshalType::Table)
    {
        const int luaType = lua_type(L, slot);
        if (paramMeta->typeKlass != nullptr && paramMeta->typeKlass->nullabletype && luaType == LUA_TNIL)
            return ConversionKind::NullLiteral;
        if (luaType == LUA_TTABLE)
            return ConversionKind::Identity;
        return ConversionKind::NotConvertible;
    }

    if (paramMeta->marshalType == LuaMarshalType::UnpackedValues)
    {
        ConversionKind worst = ConversionKind::Identity;
        for (uint16_t k = 0; k < paramMeta->memberCount; ++k)
        {
            ConversionKind kind = GetConversionKind(L, slot + static_cast<int32_t>(k), paramMeta->members[k].memberMeta);
            if (kind == ConversionKind::NotConvertible)
                return ConversionKind::NotConvertible;
            if (kind > worst)
                worst = kind;
        }
        return worst;
    }

    return GetConversionKind(L, slot, paramMeta);
}

static ConversionScore ComputeMethodConversionScore(lua_State* L, int32_t argStart, int32_t argCount, const MethodMarshalCtx* method, int* outOptionalUsed)
{
    ConversionScore bestConversionScore = {ConversionKind::Identity, 0};
    int optionalUsed = 0;
    int32_t slot = argStart;
    int32_t slotsLeft = argCount;
    const uint8_t paramStart = method->isExtension ? 1 : 0;
    for (uint8_t j = paramStart; j < method->method->parameters_count; j++)
    {
        const MarshalMetaInfo* paramMeta = method->paramsMeta[j];
        const int need = paramMeta->stackSlots > 0 ? paramMeta->stackSlots : 1;
        if (slotsLeft >= need)
        {
            ConversionKind conversionKind = GetCompositeConversionKind(L, slot, paramMeta);
            if (conversionKind == ConversionKind::NotConvertible)
            {
                bestConversionScore.kind = ConversionKind::NotConvertible;
                bestConversionScore.score = 0;
                *outOptionalUsed = 0;
                return bestConversionScore;
            }
            if (conversionKind > bestConversionScore.kind)
            {
                bestConversionScore.kind = conversionKind;
            }
            bestConversionScore.score += static_cast<int>(conversionKind);
            slot += need;
            slotsLeft -= need;
            continue;
        }

        if (ParamHasCachedDefault(method, j))
        {
            optionalUsed++;
            continue;
        }

        bestConversionScore.kind = ConversionKind::NotConvertible;
        bestConversionScore.score = 0;
        *outOptionalUsed = 0;
        return bestConversionScore;
    }

    *outOptionalUsed = optionalUsed;
    return bestConversionScore;
}

static MethodOverloadResolutionResult FindBestMatchMethod(lua_State* L, const MethodMarshalCtx** methods, size_t methodCount, int32_t argStart,
                                                          int32_t argCount)
{
    const MethodMarshalCtx* bestMethod = nullptr;
    ConversionScore bestConversionScore = {ConversionKind::NotConvertible, 0};
    int bestOptionalUsed = INT_MAX;
    int bestConversionCount = 0;

    for (size_t i = 0; i < methodCount; i++)
    {
        const MethodMarshalCtx* method = methods[i];
        if (argCount < GetMinLuaArity(method) || argCount > method->luaArity)
        {
            continue;
        }
        int optionalUsed = 0;
        ConversionScore conversionScore = ComputeMethodConversionScore(L, argStart, argCount, method, &optionalUsed);
        if (conversionScore.kind == ConversionKind::NotConvertible)
        {
            continue;
        }

        const bool betterKind = conversionScore.kind < bestConversionScore.kind;
        const bool betterScore = conversionScore.kind == bestConversionScore.kind && conversionScore.score < bestConversionScore.score;
        const bool betterOptional = conversionScore.kind == bestConversionScore.kind && conversionScore.score == bestConversionScore.score
                                    && optionalUsed < bestOptionalUsed;

        if (bestMethod == nullptr || betterKind || betterScore || betterOptional)
        {
            bestConversionScore = conversionScore;
            bestOptionalUsed = optionalUsed;
            bestMethod = method;
            bestConversionCount = 1;
        }
        else if (conversionScore.kind == bestConversionScore.kind && conversionScore.score == bestConversionScore.score
                 && optionalUsed == bestOptionalUsed)
        {
            bestConversionCount++;
        }
    }
    if (bestMethod != nullptr)
    {
        return MethodOverloadResolutionResult{bestConversionCount == 1 ? MethodOverloadResolutionKind::BestMatch : MethodOverloadResolutionKind::Ambiguous,
                                              bestMethod};
    }
    return MethodOverloadResolutionResult{MethodOverloadResolutionKind::None, nullptr};
}

MethodOverloadResolutionResult MethodOverloadResolver::Resolve(lua_State* L, const MethodGroups* groups, int32_t argStart, int32_t argCount)
{
    if (argCount <= kMaxSmallArgCount)
    {
        const MethodGroup* group = groups->smallArgCountMethodGroups[argCount];
        if (group == nullptr)
        {
            return MethodOverloadResolutionResult{MethodOverloadResolutionKind::None, nullptr};
        }
        if (group->methodCount == 1)
        {
            return MethodOverloadResolutionResult{MethodOverloadResolutionKind::BestMatch, group->methods[0]};
        }
        return FindBestMatchMethod(L, group->methods, group->methodCount, argStart, argCount);
    }

    if (groups->largeArgCountMethodCount == 1)
    {
        const MethodMarshalCtx* method = groups->largeArgCountMethods[0];
        if (argCount < GetMinLuaArity(method) || argCount > method->luaArity)
        {
            return MethodOverloadResolutionResult{MethodOverloadResolutionKind::None, nullptr};
        }
        return MethodOverloadResolutionResult{MethodOverloadResolutionKind::BestMatch, method};
    }

    return FindBestMatchMethod(L, groups->largeArgCountMethods, groups->largeArgCountMethodCount, argStart, argCount);
}

} // namespace zlua
