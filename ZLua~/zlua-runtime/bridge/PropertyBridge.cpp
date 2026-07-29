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
#include "../utils/Collection.h"
#include "../generated/PropertyBridgeStub.h"

#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Property.h"

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

template <typename T>
T PropPopStructByValue(lua_State* L, int idx, const PropertyMarshalCtx* ctx)
{
    IL2CPP_ASSERT(ctx->meta->size == sizeof(T));
    T v;
    StructMarshal::PopValue(L, idx, ctx->valueTypeKlass, &v);
    return v;
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

// Lua→C# byref property get: invoke then push the referred value (not a stack Opaque handle —
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
    PropertyInstanceSetterByValue<bool, &PrimitiveMarshal::PopBool>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int8_t, &PrimitiveMarshal::PushInt8>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int8_t, &PrimitiveMarshal::PopInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint8_t, &PrimitiveMarshal::PushUInt8>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint8_t, &PrimitiveMarshal::PopUInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int16_t, &PrimitiveMarshal::PushInt16>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int16_t, &PrimitiveMarshal::PopInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint16_t, &PrimitiveMarshal::PushUInt16>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint16_t, &PrimitiveMarshal::PopUInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int32_t, &PrimitiveMarshal::PushInt32>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int32_t, &PrimitiveMarshal::PopInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint32_t, &PrimitiveMarshal::PushUInt32>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint32_t, &PrimitiveMarshal::PopUInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<int64_t, &PrimitiveMarshal::PushInt64>(L, target, method, ctx);
}

static void PropertyInstanceSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<int64_t, &PrimitiveMarshal::PopInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uint64_t, &PrimitiveMarshal::PushUInt64>(L, target, method, ctx);
}

static void PropertyInstanceSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uint64_t, &PrimitiveMarshal::PopUInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<float, &PrimitiveMarshal::PushFloat>(L, target, method, ctx);
}

static void PropertyInstanceSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<float, &PrimitiveMarshal::PopFloat>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<double, &PrimitiveMarshal::PushDouble>(L, target, method, ctx);
}

static void PropertyInstanceSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<double, &PrimitiveMarshal::PopDouble>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<intptr_t, &PrimitiveMarshal::PushIntPtr>(L, target, method, ctx);
}

static void PropertyInstanceSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<intptr_t, &PrimitiveMarshal::PopIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValue<uintptr_t, &PrimitiveMarshal::PushUIntPtr>(L, target, method, ctx);
}

static void PropertyInstanceSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValue<uintptr_t, &PrimitiveMarshal::PopUIntPtr>(L, target, valueIdx, method, ctx);
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
    PropertyStaticSetterByValue<bool, &PrimitiveMarshal::PopBool>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int8_t, &PrimitiveMarshal::PushInt8>(L, target, method, ctx);
}

static void PropertyStaticSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int8_t, &PrimitiveMarshal::PopInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint8_t, &PrimitiveMarshal::PushUInt8>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint8_t, &PrimitiveMarshal::PopUInt8>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int16_t, &PrimitiveMarshal::PushInt16>(L, target, method, ctx);
}

static void PropertyStaticSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int16_t, &PrimitiveMarshal::PopInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint16_t, &PrimitiveMarshal::PushUInt16>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint16_t, &PrimitiveMarshal::PopUInt16>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int32_t, &PrimitiveMarshal::PushInt32>(L, target, method, ctx);
}

static void PropertyStaticSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int32_t, &PrimitiveMarshal::PopInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint32_t, &PrimitiveMarshal::PushUInt32>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint32_t, &PrimitiveMarshal::PopUInt32>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<int64_t, &PrimitiveMarshal::PushInt64>(L, target, method, ctx);
}

static void PropertyStaticSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<int64_t, &PrimitiveMarshal::PopInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uint64_t, &PrimitiveMarshal::PushUInt64>(L, target, method, ctx);
}

static void PropertyStaticSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uint64_t, &PrimitiveMarshal::PopUInt64>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<float, &PrimitiveMarshal::PushFloat>(L, target, method, ctx);
}

static void PropertyStaticSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<float, &PrimitiveMarshal::PopFloat>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<double, &PrimitiveMarshal::PushDouble>(L, target, method, ctx);
}

static void PropertyStaticSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<double, &PrimitiveMarshal::PopDouble>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<intptr_t, &PrimitiveMarshal::PushIntPtr>(L, target, method, ctx);
}

static void PropertyStaticSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<intptr_t, &PrimitiveMarshal::PopIntPtr>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValue<uintptr_t, &PrimitiveMarshal::PushUIntPtr>(L, target, method, ctx);
}

static void PropertyStaticSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValue<uintptr_t, &PrimitiveMarshal::PopUIntPtr>(L, target, valueIdx, method, ctx);
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
    PropertyStaticSetterByValueCtx<UnityVector2, &PropPopStructByValue<UnityVector2>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector2(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector2, &PropPushStructByValue<UnityVector2>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector2(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValueCtx<UnityVector2, &PropPopStructByValue<UnityVector2>>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<UnityVector3, &PropPushStructByValue<UnityVector3>>(L, target, method, ctx);
}

static void PropertyStaticSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValueCtx<UnityVector3, &PropPopStructByValue<UnityVector3>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector3, &PropPushStructByValue<UnityVector3>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValueCtx<UnityVector3, &PropPopStructByValue<UnityVector3>>(L, target, valueIdx, method, ctx);
}

static void PropertyStaticGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticGetterByValueCtx<UnityVector4, &PropPushStructByValue<UnityVector4>>(L, target, method, ctx);
}

static void PropertyStaticSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyStaticSetterByValueCtx<UnityVector4, &PropPopStructByValue<UnityVector4>>(L, target, valueIdx, method, ctx);
}

static void PropertyInstanceGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceGetterByValueCtx<UnityVector4, &PropPushStructByValue<UnityVector4>>(L, target, method, ctx);
}

static void PropertyInstanceSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    PropertyInstanceSetterByValueCtx<UnityVector4, &PropPopStructByValue<UnityVector4>>(L, target, valueIdx, method, ctx);
}

AppendOnlyStringHashMap<const PropertyBridgeEntry*> s_nameToBridge;

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
restart:
    switch (type->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterBoolean;
            accessor.setter = PropertyStaticSetterBoolean;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterBoolean;
            accessor.setter = PropertyInstanceSetterBoolean;
        }
        break;
    case IL2CPP_TYPE_I1:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterInt8;
            accessor.setter = PropertyStaticSetterInt8;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterInt8;
            accessor.setter = PropertyInstanceSetterInt8;
        }
        break;
    case IL2CPP_TYPE_U1:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUInt8;
            accessor.setter = PropertyStaticSetterUInt8;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUInt8;
            accessor.setter = PropertyInstanceSetterUInt8;
        }
        break;
    case IL2CPP_TYPE_I2:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterInt16;
            accessor.setter = PropertyStaticSetterInt16;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterInt16;
            accessor.setter = PropertyInstanceSetterInt16;
        }
        break;
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_CHAR:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUInt16;
            accessor.setter = PropertyStaticSetterUInt16;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUInt16;
            accessor.setter = PropertyInstanceSetterUInt16;
        }
        break;
    case IL2CPP_TYPE_I4:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterInt32;
            accessor.setter = PropertyStaticSetterInt32;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterInt32;
            accessor.setter = PropertyInstanceSetterInt32;
        }
        break;
    case IL2CPP_TYPE_U4:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUInt32;
            accessor.setter = PropertyStaticSetterUInt32;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUInt32;
            accessor.setter = PropertyInstanceSetterUInt32;
        }
        break;
    case IL2CPP_TYPE_I8:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterInt64;
            accessor.setter = PropertyStaticSetterInt64;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterInt64;
            accessor.setter = PropertyInstanceSetterInt64;
        }
        break;
    case IL2CPP_TYPE_U8:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUInt64;
            accessor.setter = PropertyStaticSetterUInt64;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUInt64;
            accessor.setter = PropertyInstanceSetterUInt64;
        }
        break;
    case IL2CPP_TYPE_R4:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterFloat;
            accessor.setter = PropertyStaticSetterFloat;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterFloat;
            accessor.setter = PropertyInstanceSetterFloat;
        }
        break;
    case IL2CPP_TYPE_R8:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterDouble;
            accessor.setter = PropertyStaticSetterDouble;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterDouble;
            accessor.setter = PropertyInstanceSetterDouble;
        }
        break;
    case IL2CPP_TYPE_I:
    {
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterIntPtr;
            accessor.setter = PropertyStaticSetterIntPtr;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterIntPtr;
            accessor.setter = PropertyInstanceSetterIntPtr;
        }
        break;
    }
    case IL2CPP_TYPE_U:
    {
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterUIntPtr;
            accessor.setter = PropertyStaticSetterUIntPtr;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterUIntPtr;
            accessor.setter = PropertyInstanceSetterUIntPtr;
        }
        break;
    }
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
    {
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterPointer;
            accessor.setter = PropertyStaticSetterPointer;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterPointer;
            accessor.setter = PropertyInstanceSetterPointer;
        }
        break;
    }
    case IL2CPP_TYPE_STRING:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterString;
            accessor.setter = PropertyStaticSetterString;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterString;
            accessor.setter = PropertyInstanceSetterString;
        }
        break;
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
        if (isStatic)
        {
            accessor.getter = PropertyStaticGetterObject;
            accessor.setter = PropertyStaticSetterObject;
        }
        else
        {
            accessor.getter = PropertyInstanceGetterObject;
            accessor.setter = PropertyInstanceSetterObject;
        }
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
                accessor.getter = isStatic ? bridge->staticGetter : bridge->instanceGetter;
                accessor.setter = isStatic ? bridge->staticSetter : bridge->instanceSetter;
                break;
            }
            if (TryGetIntrinsicPropertyAccessor(klass, isStatic, accessor))
            {
                return accessor;
            }
            // fallback if is nullable value type.
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
            // fallback if is value type.
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
            {
                accessor.getter = PropertyStaticGetterObject;
                accessor.setter = PropertyStaticSetterObject;
            }
            else
            {
                accessor.getter = PropertyInstanceGetterObject;
                accessor.setter = PropertyInstanceSetterObject;
            }
        }
        break;
    }
    default:
        break;
    }
    return accessor;
}

} // namespace zlua
