// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "PropertyBridge.h"
#include "PropertyBridgeTemplates.h"

#include "../marshal/IntrinsicTypes.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../marshal/TypedMarshal.h"
#include "../mt/MetaBinding.h"
#include "../utils/MetadataUtil.h"
#include "../utils/CsStringHash.h"
#include "../generated/PropertyBridgeStub.h"

#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Property.h"
#include "vm/Method.h"
#include "metadata/GenericMethod.h"

#include <unordered_map>

namespace zlua
{
namespace
{

void PropPushObject(lua_State* L, Il2CppObject* v, const PropertyMarshalCtx* ctx)
{
    ObjectMarshal::Push(L, v, ctx->meta);
}

Il2CppObject* PropPopObject(lua_State* L, int idx, const PropertyMarshalCtx* ctx)
{
    return ObjectMarshal::Pop(L, idx, ctx->valueTypeKlass);
}

template <typename T>
void PropPushStructByValue(lua_State* L, T v, const PropertyMarshalCtx* ctx)
{
    IL2CPP_ASSERT(ctx->meta->size == sizeof(T));
    StructMarshal::PushValue(L, &v, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

// Property set hot path: no ExpectLuaType (matches pre-6636bb1 direct lua_to*).
bool PropPopBool(lua_State* L, int idx)
{
    return lua_toboolean(L, idx) != 0;
}

int8_t PropPopInt8(lua_State* L, int idx)
{
    return (int8_t)lua_tointeger(L, idx);
}

uint8_t PropPopUInt8(lua_State* L, int idx)
{
    return (uint8_t)lua_tointeger(L, idx);
}

int16_t PropPopInt16(lua_State* L, int idx)
{
    return (int16_t)lua_tointeger(L, idx);
}

uint16_t PropPopUInt16(lua_State* L, int idx)
{
    return (uint16_t)lua_tointeger(L, idx);
}

int32_t PropPopInt32(lua_State* L, int idx)
{
    return (int32_t)lua_tointeger(L, idx);
}

uint32_t PropPopUInt32(lua_State* L, int idx)
{
    return (uint32_t)lua_tointeger(L, idx);
}

int64_t PropPopInt64(lua_State* L, int idx)
{
    return zlua_to_int64(L, idx);
}

uint64_t PropPopUInt64(lua_State* L, int idx)
{
    return zlua_to_uint64(L, idx);
}

float PropPopFloat(lua_State* L, int idx)
{
    return (float)lua_tonumber(L, idx);
}

double PropPopDouble(lua_State* L, int idx)
{
    return lua_tonumber(L, idx);
}

intptr_t PropPopIntPtr(lua_State* L, int idx)
{
    return PrimitiveMarshal::PopIntPtr(L, idx);
}

uintptr_t PropPopUIntPtr(lua_State* L, int idx)
{
    return PrimitiveMarshal::PopUIntPtr(L, idx);
}

template <typename T>
void PropPopStructOut(lua_State* L, int idx, const PropertyMarshalCtx* ctx, T* out)
{
    IL2CPP_ASSERT(ctx->meta->size == sizeof(T));
    StructMarshal::PopValue(L, idx, ctx->valueTypeKlass, out);
}

void PropPushValueTypeBuf(lua_State* L, void* buf, const PropertyMarshalCtx* ctx)
{
    StructMarshal::PushValue(L, buf, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

void PropPopValueTypeBuf(lua_State* L, int idx, void* buf, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)ctx;
    TypedMarshal::PopByType(L, idx, buf, method->parameters[0]);
}

void PropPushNullableBuf(lua_State* L, void* buf, const PropertyMarshalCtx* ctx)
{
    StructMarshal::PushNullableValue(L, buf, ctx->meta->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

void PropPopNullableBuf(lua_State* L, int idx, void* buf, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)method;
    StructMarshal::PopNullableValue(L, idx, ctx->meta->typeKlass, buf);
}

// Lua->C# byref property get: invoke then push the referred value (not a stack Opaque handle;
// OpaqueParameterScope cannot keep alloca alive after this C function returns).
void PropPushByRefPropertyValue(lua_State* L, void* retBuf, const PropertyMarshalCtx* ctx)
{
    const Il2CppType* byrefType = ctx->meta->type;
    IL2CPP_ASSERT(byrefType != nullptr && byrefType->byref);
    void* fieldAddr = *(void**)retBuf;
    Il2CppType bare = *byrefType;
    bare.byref = false;
    TypedMarshal::PushByType(L, fieldAddr, &bare);
}

void PropPopOpaqueBuf(lua_State* L, int idx, void* buf, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)method;
    OpaqueValueMarshal::Pop(L, idx, buf, ctx->meta->type);
}

} // namespace

static void PropertyInstanceGetterBoolean(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<bool, &PrimitiveMarshal::PushBool>(L, target, method, ctx);
}

static void PropertyInstanceSetterBoolean(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<bool, &PropPopBool>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int8_t, &PrimitiveMarshal::PushInt8>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int8_t, &PropPopInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint8_t, &PrimitiveMarshal::PushUInt8>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint8_t, &PropPopUInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int16_t, &PrimitiveMarshal::PushInt16>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int16_t, &PropPopInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint16_t, &PrimitiveMarshal::PushUInt16>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint16_t, &PropPopUInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int32_t, &PrimitiveMarshal::PushInt32>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int32_t, &PropPopInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint32_t, &PrimitiveMarshal::PushUInt32>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint32_t, &PropPopUInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int64_t, &PrimitiveMarshal::PushInt64>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int64_t, &PropPopInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint64_t, &PrimitiveMarshal::PushUInt64>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint64_t, &PropPopUInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<float, &PrimitiveMarshal::PushFloat>(L, target, method, ctx);
}

static void PropertyInstanceSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<float, &PropPopFloat>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<double, &PrimitiveMarshal::PushDouble>(L, target, method, ctx);
}

static void PropertyInstanceSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<double, &PropPopDouble>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<intptr_t, &PrimitiveMarshal::PushIntPtr>(L, target, method, ctx);
}

static void PropertyInstanceSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<intptr_t, &PropPopIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uintptr_t, &PrimitiveMarshal::PushUIntPtr>(L, target, method, ctx);
}

static void PropertyInstanceSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uintptr_t, &PropPopUIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterPointer(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<void*, &PrimitiveMarshal::PushPointer>(L, target, method, ctx);
}

static void PropertyInstanceSetterPointer(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<void*, &PrimitiveMarshal::PopPointer>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterString(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<Il2CppString*, &StringMarshal::Push>(L, target, method, ctx);
}

static void PropertyInstanceSetterString(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<Il2CppString*, &StringMarshal::Pop>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterObject(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<Il2CppObject*, PropPushObject>(L, target, method, ctx);
}

static void PropertyInstanceSetterObject(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValueCtx<Il2CppObject*, PropPopObject>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterValueType(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterInvokerBuf<PropPushValueTypeBuf>(L, target, method, ctx);
}

static void PropertyInstanceSetterValueType(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterInvokerBuf<PropPopValueTypeBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterBoolean(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<bool, &PrimitiveMarshal::PushBool>(L, target, method, ctx);
}

static void PropertyStaticSetterBoolean(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<bool, &PropPopBool>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int8_t, &PrimitiveMarshal::PushInt8>(L, target, method, ctx);
}

static void PropertyStaticSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int8_t, &PropPopInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint8_t, &PrimitiveMarshal::PushUInt8>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint8_t, &PropPopUInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int16_t, &PrimitiveMarshal::PushInt16>(L, target, method, ctx);
}

static void PropertyStaticSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int16_t, &PropPopInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint16_t, &PrimitiveMarshal::PushUInt16>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint16_t, &PropPopUInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int32_t, &PrimitiveMarshal::PushInt32>(L, target, method, ctx);
}

static void PropertyStaticSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int32_t, &PropPopInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint32_t, &PrimitiveMarshal::PushUInt32>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint32_t, &PropPopUInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int64_t, &PrimitiveMarshal::PushInt64>(L, target, method, ctx);
}

static void PropertyStaticSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int64_t, &PropPopInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint64_t, &PrimitiveMarshal::PushUInt64>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint64_t, &PropPopUInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<float, &PrimitiveMarshal::PushFloat>(L, target, method, ctx);
}

static void PropertyStaticSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<float, &PropPopFloat>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<double, &PrimitiveMarshal::PushDouble>(L, target, method, ctx);
}

static void PropertyStaticSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<double, &PropPopDouble>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<intptr_t, &PrimitiveMarshal::PushIntPtr>(L, target, method, ctx);
}

static void PropertyStaticSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<intptr_t, &PropPopIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uintptr_t, &PrimitiveMarshal::PushUIntPtr>(L, target, method, ctx);
}

static void PropertyStaticSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uintptr_t, &PropPopUIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterPointer(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<void*, &PrimitiveMarshal::PushPointer>(L, target, method, ctx);
}

static void PropertyStaticSetterPointer(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<void*, &PrimitiveMarshal::PopPointer>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterString(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<Il2CppString*, &StringMarshal::Push>(L, target, method, ctx);
}

static void PropertyStaticSetterString(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<Il2CppString*, &StringMarshal::Pop>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterObject(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<Il2CppObject*, PropPushObject>(L, target, method, ctx);
}

static void PropertyStaticSetterObject(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValueCtx<Il2CppObject*, PropPopObject>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterValueType(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterInvokerBuf<PropPushValueTypeBuf>(L, target, method, ctx);
}

static void PropertyStaticSetterValueType(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterInvokerBuf<PropPopValueTypeBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterNullable(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterInvokerBuf<PropPushNullableBuf>(L, target, method, ctx);
}

static void PropertyStaticSetterNullable(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterInvokerBuf<PropPopNullableBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterNullable(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterInvokerBuf<PropPushNullableBuf>(L, target, method, ctx);
}

static void PropertyInstanceSetterNullable(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterInvokerBuf<PropPopNullableBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetOpaqueValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterInvokerBuf<PropPushByRefPropertyValue>(L, target, method, ctx);
}

static void PropertyStaticSetOpaqueValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterInvokerBuf<PropPopOpaqueBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetOpaqueValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterInvokerBuf<PropPushByRefPropertyValue>(L, target, method, ctx);
}

static void PropertyInstanceSetOpaqueValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterInvokerBuf<PropPopOpaqueBuf>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUnityVector2(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<UnityVector2, &PropPushStructByValue<UnityVector2>>(L, target, method, ctx);
}

static void PropertyStaticSetterUnityVector2(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterOutParamCtx<UnityVector2, &PropPopStructOut<UnityVector2>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector2(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector2, &PropPushStructByValue<UnityVector2>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector2(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterOutParamCtx<UnityVector2, &PropPopStructOut<UnityVector2>>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<UnityVector3, &PropPushStructByValue<UnityVector3>>(L, target, method, ctx);
}

static void PropertyStaticSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterOutParamCtx<UnityVector3, &PropPopStructOut<UnityVector3>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector3, &PropPushStructByValue<UnityVector3>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterOutParamCtx<UnityVector3, &PropPopStructOut<UnityVector3>>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<UnityVector4, &PropPushStructByValue<UnityVector4>>(L, target, method, ctx);
}

static void PropertyStaticSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterOutParamCtx<UnityVector4, &PropPopStructOut<UnityVector4>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector4, &PropPushStructByValue<UnityVector4>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterOutParamCtx<UnityVector4, &PropPopStructOut<UnityVector4>>(L, target, valueIdx, method, ctx);
}

// ---------------------------------------------------------------------------
// Full generic sharing accessors (invoker_method; not direct methodPointer)
// ---------------------------------------------------------------------------

#define ZLUA_FGS_PRIMITIVE_ACCESSORS(Name, T, PushFn, PopFn)                                                                                                                       \
    static void FullGenericSharingPropertyStaticGetter##Name(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)                                  \
    {                                                                                                                                                                              \
        PropertyStaticGetterInvokerPrimitiveBuf<T, PushFn>(L, target, method, ctx);                                                                                                \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyStaticSetter##Name(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)                    \
    {                                                                                                                                                                              \
        PropertyStaticSetterInvokerPrimitiveBuf<T, PopFn>(L, target, valueIdx, method, ctx);                                                                                       \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyInstanceGetter##Name(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)                                \
    {                                                                                                                                                                              \
        PropertyInstanceGetterInvokerPrimitiveBuf<T, PushFn>(L, target, method, ctx);                                                                                              \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyInstanceSetter##Name(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)                  \
    {                                                                                                                                                                              \
        PropertyInstanceSetterInvokerPrimitiveBuf<T, PopFn>(L, target, valueIdx, method, ctx);                                                                                     \
    }

ZLUA_FGS_PRIMITIVE_ACCESSORS(Boolean, bool, &PrimitiveMarshal::PushBool, &PrimitiveMarshal::PopBool)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Int8, int8_t, &PrimitiveMarshal::PushInt8, &PrimitiveMarshal::PopInt8)
ZLUA_FGS_PRIMITIVE_ACCESSORS(UInt8, uint8_t, &PrimitiveMarshal::PushUInt8, &PrimitiveMarshal::PopUInt8)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Int16, int16_t, &PrimitiveMarshal::PushInt16, &PrimitiveMarshal::PopInt16)
ZLUA_FGS_PRIMITIVE_ACCESSORS(UInt16, uint16_t, &PrimitiveMarshal::PushUInt16, &PrimitiveMarshal::PopUInt16)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Int32, int32_t, &PrimitiveMarshal::PushInt32, &PrimitiveMarshal::PopInt32)
ZLUA_FGS_PRIMITIVE_ACCESSORS(UInt32, uint32_t, &PrimitiveMarshal::PushUInt32, &PrimitiveMarshal::PopUInt32)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Int64, int64_t, &PrimitiveMarshal::PushInt64, &PrimitiveMarshal::PopInt64)
ZLUA_FGS_PRIMITIVE_ACCESSORS(UInt64, uint64_t, &PrimitiveMarshal::PushUInt64, &PrimitiveMarshal::PopUInt64)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Float, float, &PrimitiveMarshal::PushFloat, &PrimitiveMarshal::PopFloat)
ZLUA_FGS_PRIMITIVE_ACCESSORS(Double, double, &PrimitiveMarshal::PushDouble, &PrimitiveMarshal::PopDouble)
ZLUA_FGS_PRIMITIVE_ACCESSORS(IntPtr, intptr_t, &PrimitiveMarshal::PushIntPtr, &PrimitiveMarshal::PopIntPtr)
ZLUA_FGS_PRIMITIVE_ACCESSORS(UIntPtr, uintptr_t, &PrimitiveMarshal::PushUIntPtr, &PrimitiveMarshal::PopUIntPtr)

#undef ZLUA_FGS_PRIMITIVE_ACCESSORS

#define ZLUA_FGS_REFLIKE_ACCESSORS(Name, T)                                                                                                                                        \
    static void FullGenericSharingPropertyStaticGetter##Name(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)                                  \
    {                                                                                                                                                                              \
        PropertyStaticGetterInvokerRefLike<T>(L, target, method, ctx);                                                                                                             \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyStaticSetter##Name(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)                    \
    {                                                                                                                                                                              \
        PropertyStaticSetterInvokerRefLike<T>(L, target, valueIdx, method, ctx);                                                                                                   \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyInstanceGetter##Name(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)                                \
    {                                                                                                                                                                              \
        PropertyInstanceGetterInvokerRefLike<T>(L, target, method, ctx);                                                                                                           \
    }                                                                                                                                                                              \
    static void FullGenericSharingPropertyInstanceSetter##Name(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)                  \
    {                                                                                                                                                                              \
        PropertyInstanceSetterInvokerRefLike<T>(L, target, valueIdx, method, ctx);                                                                                                 \
    }

ZLUA_FGS_REFLIKE_ACCESSORS(String, Il2CppString*)
ZLUA_FGS_REFLIKE_ACCESSORS(Object, Il2CppObject*)
ZLUA_FGS_REFLIKE_ACCESSORS(Pointer, void*)

#undef ZLUA_FGS_REFLIKE_ACCESSORS

std::unordered_map<const char*, const PropertyBridgeEntry*, CsStringHash, CsStringEqual> s_nameToBridge;

void PropertyBridge::Initialize()
{
    for (size_t i = 0;; i++)
    {
        const PropertyBridgeEntry* bridge = &propertybridge::g_propertyBridges[i];
        if (bridge->stubName == nullptr)
        {
            break;
        }
        s_nameToBridge.insert({bridge->stubName, bridge});
    }
}

static const PropertyBridgeEntry* GetPropertyBridgeEntry(Il2CppClass* klass)
{
    const char* typeFullName = MetadataUtil::GetTypeFullName(klass);
    auto it = s_nameToBridge.find(typeFullName);
    return it != s_nameToBridge.end() ? it->second : nullptr;
}

static bool TryGetIntrinsicPropertyAccessor(Il2CppClass* klass, bool isStatic, PropertyAccessor& accessor)
{
    // UnityEngine.Vector2/3/4: direct methodPointer call (avoid invoker_method).
    switch (GetIntrinsicType(klass->namespaze, klass->name))
    {
    case IntrinsicTypeKind::Vector2:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUnityVector2;
            accessor.setter = PropertyStaticSetterUnityVector2;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUnityVector2;
            accessor.setter = PropertyInstanceSetterUnityVector2;
        }
        return true;
    case IntrinsicTypeKind::Vector3:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUnityVector3;
            accessor.setter = PropertyStaticSetterUnityVector3;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUnityVector3;
            accessor.setter = PropertyInstanceSetterUnityVector3;
        }
        return true;
    case IntrinsicTypeKind::Vector4:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUnityVector4;
            accessor.setter = PropertyStaticSetterUnityVector4;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUnityVector4;
            accessor.setter = PropertyInstanceSetterUnityVector4;
        }
        return true;
    default:
        return false;
    }
}

PropertyAccessor PropertyBridge::ResolvePropertyAccessor(const PropertyInfo* property, bool isStatic)
{
    PropertyAccessor accessor = {nullptr, nullptr};
    const Il2CppType* type = property->get != nullptr ? property->get->return_type : property->set->parameters[0];
    if (type->byref)
    {
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetOpaqueValue;
            accessor.setter = PropertyStaticSetOpaqueValue;
        }
        else
        {
            accessor.getter = PropertyInstanceGetOpaqueValue;
            accessor.setter = PropertyInstanceSetOpaqueValue;
        }
        return accessor;
    }

#if ZLUA_UNITY_VERSION < 20220000
    const bool getterHasFullGenericSharing = property->get != nullptr && il2cpp::vm::Method::HasFullGenericSharingSignature(property->get);
    const bool setterHasFullGenericSharing = property->set != nullptr && il2cpp::vm::Method::HasFullGenericSharingSignature(property->set);
#else
    const bool getterHasFullGenericSharing = false;
    const bool setterHasFullGenericSharing = false;
#endif

#define ZLUA_SELECT_PROP_ACCESSOR(FgsG, FgsS, NormG, NormS)                                                                                                                        \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        accessor.getter = getterHasFullGenericSharing ? (FgsG) : (NormG);                                                                                                          \
        accessor.setter = setterHasFullGenericSharing ? (FgsS) : (NormS);                                                                                                          \
    } while (0)

restart:
    switch (type->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterBoolean, FullGenericSharingPropertyStaticSetterBoolean, PropertyStaticGetterBoolean,
                                      PropertyStaticSetterBoolean);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterBoolean, FullGenericSharingPropertyInstanceSetterBoolean, PropertyInstanceGetterBoolean,
                                      PropertyInstanceSetterBoolean);
        break;
    case IL2CPP_TYPE_I1:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterInt8, FullGenericSharingPropertyStaticSetterInt8, PropertyStaticGetterInt8, PropertyStaticSetterInt8);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterInt8, FullGenericSharingPropertyInstanceSetterInt8, PropertyInstanceGetterInt8,
                                      PropertyInstanceSetterInt8);
        break;
    case IL2CPP_TYPE_U1:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterUInt8, FullGenericSharingPropertyStaticSetterUInt8, PropertyStaticGetterUInt8,
                                      PropertyStaticSetterUInt8);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterUInt8, FullGenericSharingPropertyInstanceSetterUInt8, PropertyInstanceGetterUInt8,
                                      PropertyInstanceSetterUInt8);
        break;
    case IL2CPP_TYPE_I2:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterInt16, FullGenericSharingPropertyStaticSetterInt16, PropertyStaticGetterInt16,
                                      PropertyStaticSetterInt16);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterInt16, FullGenericSharingPropertyInstanceSetterInt16, PropertyInstanceGetterInt16,
                                      PropertyInstanceSetterInt16);
        break;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterUInt16, FullGenericSharingPropertyStaticSetterUInt16, PropertyStaticGetterUInt16,
                                      PropertyStaticSetterUInt16);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterUInt16, FullGenericSharingPropertyInstanceSetterUInt16, PropertyInstanceGetterUInt16,
                                      PropertyInstanceSetterUInt16);
        break;
    case IL2CPP_TYPE_I4:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterInt32, FullGenericSharingPropertyStaticSetterInt32, PropertyStaticGetterInt32,
                                      PropertyStaticSetterInt32);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterInt32, FullGenericSharingPropertyInstanceSetterInt32, PropertyInstanceGetterInt32,
                                      PropertyInstanceSetterInt32);
        break;
    case IL2CPP_TYPE_U4:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterUInt32, FullGenericSharingPropertyStaticSetterUInt32, PropertyStaticGetterUInt32,
                                      PropertyStaticSetterUInt32);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterUInt32, FullGenericSharingPropertyInstanceSetterUInt32, PropertyInstanceGetterUInt32,
                                      PropertyInstanceSetterUInt32);
        break;
    case IL2CPP_TYPE_I8:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterInt64, FullGenericSharingPropertyStaticSetterInt64, PropertyStaticGetterInt64,
                                      PropertyStaticSetterInt64);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterInt64, FullGenericSharingPropertyInstanceSetterInt64, PropertyInstanceGetterInt64,
                                      PropertyInstanceSetterInt64);
        break;
    case IL2CPP_TYPE_U8:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterUInt64, FullGenericSharingPropertyStaticSetterUInt64, PropertyStaticGetterUInt64,
                                      PropertyStaticSetterUInt64);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterUInt64, FullGenericSharingPropertyInstanceSetterUInt64, PropertyInstanceGetterUInt64,
                                      PropertyInstanceSetterUInt64);
        break;
    case IL2CPP_TYPE_R4:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterFloat, FullGenericSharingPropertyStaticSetterFloat, PropertyStaticGetterFloat,
                                      PropertyStaticSetterFloat);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterFloat, FullGenericSharingPropertyInstanceSetterFloat, PropertyInstanceGetterFloat,
                                      PropertyInstanceSetterFloat);
        break;
    case IL2CPP_TYPE_R8:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterDouble, FullGenericSharingPropertyStaticSetterDouble, PropertyStaticGetterDouble,
                                      PropertyStaticSetterDouble);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterDouble, FullGenericSharingPropertyInstanceSetterDouble, PropertyInstanceGetterDouble,
                                      PropertyInstanceSetterDouble);
        break;
    case IL2CPP_TYPE_I:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterIntPtr, FullGenericSharingPropertyStaticSetterIntPtr, PropertyStaticGetterIntPtr,
                                      PropertyStaticSetterIntPtr);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterIntPtr, FullGenericSharingPropertyInstanceSetterIntPtr, PropertyInstanceGetterIntPtr,
                                      PropertyInstanceSetterIntPtr);
        break;
    case IL2CPP_TYPE_U:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterUIntPtr, FullGenericSharingPropertyStaticSetterUIntPtr, PropertyStaticGetterUIntPtr,
                                      PropertyStaticSetterUIntPtr);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterUIntPtr, FullGenericSharingPropertyInstanceSetterUIntPtr, PropertyInstanceGetterUIntPtr,
                                      PropertyInstanceSetterUIntPtr);
        break;
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterPointer, FullGenericSharingPropertyStaticSetterPointer, PropertyStaticGetterPointer,
                                      PropertyStaticSetterPointer);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterPointer, FullGenericSharingPropertyInstanceSetterPointer, PropertyInstanceGetterPointer,
                                      PropertyInstanceSetterPointer);
        break;
    case IL2CPP_TYPE_STRING:
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterString, FullGenericSharingPropertyStaticSetterString, PropertyStaticGetterString,
                                      PropertyStaticSetterString);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterString, FullGenericSharingPropertyInstanceSetterString, PropertyInstanceGetterString,
                                      PropertyInstanceSetterString);
        break;
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        // class / object / delegate / array share the object RefLike FGS path
        if (isStatic)
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterObject, FullGenericSharingPropertyStaticSetterObject, PropertyStaticGetterObject,
                                      PropertyStaticSetterObject);
        else
            ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterObject, FullGenericSharingPropertyInstanceSetterObject, PropertyInstanceGetterObject,
                                      PropertyInstanceSetterObject);
        break;
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type);
        if (MetadataUtil::IsValueTypeClass(klass))
        {
            if (klass->enumtype)
            {
                type = &klass->element_class->byval_arg;
                goto restart;
            }
            const PropertyBridgeEntry* bridge = GetPropertyBridgeEntry(klass);
            if (bridge != nullptr)
            {
                // Specialized stubs call methodPointer; FGS accessors fall back to ValueType invoker path.
                FnPropertyGetter normG = isStatic ? bridge->staticGetter : bridge->instanceGetter;
                FnPropertySetter normS = isStatic ? bridge->staticSetter : bridge->instanceSetter;
                FnPropertyGetter fgsG = isStatic ? PropertyStaticGetterValueType : PropertyInstanceGetterValueType;
                FnPropertySetter fgsS = isStatic ? PropertyStaticSetterValueType : PropertyInstanceSetterValueType;
                ZLUA_SELECT_PROP_ACCESSOR(fgsG, fgsS, normG, normS);
                break;
            }
            if (TryGetIntrinsicPropertyAccessor(klass, isStatic, accessor))
            {
                FnPropertyGetter normG = accessor.getter;
                FnPropertySetter normS = accessor.setter;
                FnPropertyGetter fgsG = isStatic ? PropertyStaticGetterValueType : PropertyInstanceGetterValueType;
                FnPropertySetter fgsS = isStatic ? PropertyStaticSetterValueType : PropertyInstanceSetterValueType;
                ZLUA_SELECT_PROP_ACCESSOR(fgsG, fgsS, normG, normS);
                break;
            }
            // Nullable / ValueType already use invoker_method — no separate FGS variants.
            if (klass->nullabletype)
            {
                if (isStatic)
                {
                    accessor.getter = PropertyStaticGetterNullable;
                    accessor.setter = PropertyStaticSetterNullable;
                }
                else
                {
                    accessor.getter = PropertyInstanceGetterNullable;
                    accessor.setter = PropertyInstanceSetterNullable;
                }
                break;
            }
            if (isStatic)
            {
                accessor.getter = PropertyStaticGetterValueType;
                accessor.setter = PropertyStaticSetterValueType;
            }
            else
            {
                accessor.getter = PropertyInstanceGetterValueType;
                accessor.setter = PropertyInstanceSetterValueType;
            }
        }
        else
        {
            if (isStatic)
                ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyStaticGetterObject, FullGenericSharingPropertyStaticSetterObject, PropertyStaticGetterObject,
                                          PropertyStaticSetterObject);
            else
                ZLUA_SELECT_PROP_ACCESSOR(FullGenericSharingPropertyInstanceGetterObject, FullGenericSharingPropertyInstanceSetterObject, PropertyInstanceGetterObject,
                                          PropertyInstanceSetterObject);
        }
        break;
    }
    default:
        break;
    }

#undef ZLUA_SELECT_PROP_ACCESSOR
    return accessor;
}

} // namespace zlua
