#include "MethodOverloadResolver.h"
#include "MarshalDefs.h"
#include "MarshalMeta.h"
#include "PrimitiveMarshal.h"

#include "../mt/MetaBinding.h"
#include "../mt/InstanceTarget.h"
#include "../utils/LuaException.h"

#include "vm/Class.h"

namespace zlua
{

static ConversionKind GetConversionKind(lua_State* L, int index, const MarshalMetaInfo* paramMeta)
{
    int luaType = lua_type(L, index);
    const Il2CppType* paramType = paramMeta->type;
    if (paramType->byref)
    {
        // FIXME: handle byref
        if (luaType != LUA_TLIGHTUSERDATA)
        {
            return ConversionKind::NotConvertible;
        }
        return ConversionKind::Identity;
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
        // FIXME: handle nullable or boxing value type
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
        // FIXME: handle nullable or boxing value type
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
            klass = klass->element_class;
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

static ConversionScore ComputeMethodConversionScore(lua_State* L, int32_t argStart, int32_t argCount, const MethodMarshalCtx* method)
{
    ConversionScore bestConversionScore = {ConversionKind::Identity, 0};
    for (int j = 0; j < argCount; j++)
    {
        const MarshalMetaInfo* paramMeta = method->paramsMeta[j];
        ConversionKind conversionKind = GetConversionKind(L, argStart + j, paramMeta);
        if (conversionKind > bestConversionScore.kind)
        {
            bestConversionScore.kind = conversionKind;
        }
        bestConversionScore.score += static_cast<int>(conversionKind);
    }
    return bestConversionScore;
}

static MethodOverloadResolutionResult FindBestMatchMethod(lua_State* L, const MethodMarshalCtx** methods, size_t methodCount, int32_t argStart,
                                                          int32_t argCount)
{
    const MethodMarshalCtx* bestMethod = nullptr;
    ConversionScore bestConversionScore = {ConversionKind::NotConvertible, 0};
    int bestConversionCount = 0;

    for (size_t i = 0; i < methodCount; i++)
    {
        const MethodMarshalCtx* method = methods[i];
        if (method->method->parameters_count != argCount)
        {
            continue;
        }
        ConversionScore conversionScore = ComputeMethodConversionScore(L, argStart, argCount, method);
        if (conversionScore.kind < bestConversionScore.kind)
        {
            bestConversionScore = conversionScore;
            bestMethod = method;
            bestConversionCount = 1;
        }
        else if (conversionScore.kind == bestConversionScore.kind)
        {
            if (conversionScore.score < bestConversionScore.score)
            {
                bestConversionScore = conversionScore;
                bestMethod = method;
                bestConversionCount = 1;
            }
            else if (conversionScore.score == bestConversionScore.score)
            {
                bestConversionCount++;
            }
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
        if (method->method->parameters_count != argCount)
        {
            return MethodOverloadResolutionResult{MethodOverloadResolutionKind::None, nullptr};
        }
        return MethodOverloadResolutionResult{MethodOverloadResolutionKind::BestMatch, method};
    }

    return FindBestMatchMethod(L, groups->largeArgCountMethods, groups->largeArgCountMethodCount, argStart, argCount);
}

} // namespace zlua