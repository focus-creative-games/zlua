#include "PropertyBridge.h"

#include "../marshal/IntrinsicTypes.h"
#include "../marshal/PrimitiveMarshal.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../mt/MetaBinding.h"
#include "../utils/MetadataUtil.h"
#include "../utils/Collection.h"
#include "../generated/PropertyBridgeStub.h"

#include "vm/Class.h"
#include "vm/Field.h"
#include "vm/Property.h"

namespace zlua
{

static void PropertyInstanceGetterBoolean(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef bool (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    bool value = getValue(target, getter);
    lua_pushboolean(L, value);
}

static void PropertyInstanceSetterBoolean(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, bool value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, lua_toboolean(L, valueIdx) ? 1 : 0, setter);
}

static void PropertyInstanceGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef int8_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int8_t value = getValue(target, getter);
    lua_pushinteger(L, value);
}

static void PropertyInstanceSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, int8_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (int8_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef uint8_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint8_t value = getValue(target, getter);
    lua_pushinteger(L, value);
}

static void PropertyInstanceSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, uint8_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (uint8_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef int16_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int16_t value = getValue(target, getter);
    lua_pushinteger(L, value);
}

static void PropertyInstanceSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, int16_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (int16_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef uint16_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint16_t value = getValue(target, getter);
    lua_pushinteger(L, value);
}

static void PropertyInstanceSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, uint16_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (uint16_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef int32_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int32_t value = getValue(target, getter);
    lua_pushinteger(L, value);
}

static void PropertyInstanceSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, int32_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (int32_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef uint32_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint32_t value = getValue(target, getter);
    lua_pushinteger(L, (lua_Integer)value);
}

static void PropertyInstanceSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, uint32_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (uint32_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef int64_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int64_t value = getValue(target, getter);
    lua_pushinteger(L, (lua_Integer)value);
}

static void PropertyInstanceSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, int64_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (int64_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyInstanceGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef uint64_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint64_t value = getValue(target, getter);
    if (value <= (uint64_t)LUA_MAXINTEGER)
        lua_pushinteger(L, (lua_Integer)value);
    else
        lua_pushnumber(L, (lua_Number)value);
}

static void PropertyInstanceSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, uint64_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    uint64_t v;
    if (lua_isinteger(L, valueIdx))
        v = (uint64_t)lua_tointeger(L, valueIdx);
    else
        v = (uint64_t)lua_tonumber(L, valueIdx);
    setValue(target, v, setter);
}

static void PropertyInstanceGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef float (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    float value = getValue(target, getter);
    lua_pushnumber(L, value);
}

static void PropertyInstanceSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, float value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, (float)lua_tonumber(L, valueIdx), setter);
}

static void PropertyInstanceGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef double (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    double value = getValue(target, getter);
    lua_pushnumber(L, value);
}

static void PropertyInstanceSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, double value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, lua_tonumber(L, valueIdx), setter);
}

static void PropertyInstanceGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef intptr_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    intptr_t value = getValue(target, getter);
    PrimitiveMarshal::PushIntPtr(L, value);
}

static void PropertyInstanceSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, intptr_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, PrimitiveMarshal::PopIntPtr(L, valueIdx), setter);
}

static void PropertyInstanceGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef uintptr_t (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uintptr_t value = getValue(target, getter);
    PrimitiveMarshal::PushUIntPtr(L, value);
}

static void PropertyInstanceSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, uintptr_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, PrimitiveMarshal::PopUIntPtr(L, valueIdx), setter);
}

static void PropertyInstanceGetterPointer(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void* (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    void* value = getValue(target, getter);
    PrimitiveMarshal::PushPointer(L, value);
}

static void PropertyInstanceSetterPointer(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, void* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(target, PrimitiveMarshal::PopPointer(L, valueIdx), setter);
}

static void PropertyInstanceGetterString(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef Il2CppString* (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    Il2CppString* value = getValue(target, getter);
    StringMarshal::Push(L, value);
}

static void PropertyInstanceSetterString(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, Il2CppString* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    Il2CppString* value = StringMarshal::Pop(L, valueIdx);
    setValue(target, value, setter);
}

static void PropertyInstanceGetterObject(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef Il2CppObject* (*FnGetValue)(void* target, const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    Il2CppObject* value = getValue(target, getter);
    ObjectMarshal::Push(L, value, ctx->meta);
}

static void PropertyInstanceSetterObject(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, Il2CppObject* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    Il2CppObject* value = ObjectMarshal::Pop(L, valueIdx, ctx->valueTypeKlass);
    setValue(target, value, setter);
}

static void PropertyInstanceGetterValueType(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    const MethodInfo* getter = method;
    void* ret = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, target, nullptr, ret);
    StructMarshal::PushValue(L, ret, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyInstanceSetterValueType(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    const MethodInfo* setter = method;
    const Il2CppType* type = setter->parameters[0];
    size_t size = ctx->meta->size;
    void* value = alloca(size);

    TypedMarshal::PopByType(L, valueIdx, value, type);

    void* params[1] = {value};
    setter->invoker_method(setter->methodPointer, setter, target, params, nullptr);
}

// static getter and setter

static void PropertyStaticGetterBoolean(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef bool (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    bool value = getValue(getter);
    lua_pushboolean(L, value);
}

static void PropertyStaticSetterBoolean(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(bool value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(lua_toboolean(L, valueIdx) ? 1 : 0, setter);
}

static void PropertyStaticGetterInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef int8_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int8_t value = getValue(getter);
    lua_pushinteger(L, value);
}

static void PropertyStaticSetterInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(int8_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((int8_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterUInt8(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef uint8_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint8_t value = getValue(getter);
    lua_pushinteger(L, value);
}

static void PropertyStaticSetterUInt8(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(uint8_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((uint8_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef int16_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int16_t value = getValue(getter);
    lua_pushinteger(L, value);
}

static void PropertyStaticSetterInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(int16_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((int16_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterUInt16(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef uint16_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint16_t value = getValue(getter);
    lua_pushinteger(L, value);
}

static void PropertyStaticSetterUInt16(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(uint16_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((uint16_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef int32_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int32_t value = getValue(getter);
    lua_pushinteger(L, value);
}

static void PropertyStaticSetterInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(int32_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((int32_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterUInt32(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef uint32_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint32_t value = getValue(getter);
    lua_pushinteger(L, (lua_Integer)value);
}

static void PropertyStaticSetterUInt32(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(uint32_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((uint32_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef int64_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    int64_t value = getValue(getter);
    lua_pushinteger(L, (lua_Integer)value);
}

static void PropertyStaticSetterInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(int64_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((int64_t)lua_tointeger(L, valueIdx), setter);
}

static void PropertyStaticGetterUInt64(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef uint64_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uint64_t value = getValue(getter);
    if (value <= (uint64_t)LUA_MAXINTEGER)
        lua_pushinteger(L, (lua_Integer)value);
    else
        lua_pushnumber(L, (lua_Number)value);
}

static void PropertyStaticSetterUInt64(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(uint64_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    uint64_t v;
    if (lua_isinteger(L, valueIdx))
        v = (uint64_t)lua_tointeger(L, valueIdx);
    else
        v = (uint64_t)lua_tonumber(L, valueIdx);
    setValue(v, setter);
}

static void PropertyStaticGetterFloat(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef float (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    float value = getValue(getter);
    lua_pushnumber(L, value);
}

static void PropertyStaticSetterFloat(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(float value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue((float)lua_tonumber(L, valueIdx), setter);
}

static void PropertyStaticGetterDouble(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef double (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    double value = getValue(getter);
    lua_pushnumber(L, value);
}

static void PropertyStaticSetterDouble(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(double value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(lua_tonumber(L, valueIdx), setter);
}

static void PropertyStaticGetterIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef intptr_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    intptr_t value = getValue(getter);
    PrimitiveMarshal::PushIntPtr(L, value);
}

static void PropertyStaticSetterIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(intptr_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(PrimitiveMarshal::PopIntPtr(L, valueIdx), setter);
}

static void PropertyStaticGetterUIntPtr(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef uintptr_t (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    uintptr_t value = getValue(getter);
    PrimitiveMarshal::PushUIntPtr(L, value);
}

static void PropertyStaticSetterUIntPtr(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(uintptr_t value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(PrimitiveMarshal::PopUIntPtr(L, valueIdx), setter);
}

static void PropertyStaticGetterPointer(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void* (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    void* value = getValue(getter);
    PrimitiveMarshal::PushPointer(L, value);
}

static void PropertyStaticSetterPointer(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(void* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    setValue(PrimitiveMarshal::PopPointer(L, valueIdx), setter);
}

static void PropertyStaticGetterString(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef Il2CppString* (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    Il2CppString* value = getValue(getter);
    StringMarshal::Push(L, value);
}

static void PropertyStaticSetterString(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(Il2CppString* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    Il2CppString* value = StringMarshal::Pop(L, valueIdx);
    setValue(value, setter);
}

static void PropertyStaticGetterObject(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef Il2CppObject* (*FnGetValue)(const MethodInfo* getter);
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    Il2CppObject* value = getValue(getter);
    ObjectMarshal::Push(L, value, ctx->meta);
}

static void PropertyStaticSetterObject(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(Il2CppObject* value, const MethodInfo* setter);
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    Il2CppObject* value = ObjectMarshal::Pop(L, valueIdx, ctx->valueTypeKlass);
    setValue(value, setter);
}

static void PropertyStaticGetterValueType(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* getter = method;
    void* ret = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, nullptr, nullptr, ret);
    StructMarshal::PushValue(L, ret, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyStaticSetterValueType(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* setter = method;
    const Il2CppType* type = setter->parameters[0];
    size_t size = ctx->meta->size;
    void* value = alloca(size);
    TypedMarshal::PopByType(L, valueIdx, value, type);
    void* params[1] = {value};
    setter->invoker_method(setter->methodPointer, setter, nullptr, params, nullptr);
}

static void PropertyStaticGetterNullable(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* getter = method;
    void* value = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, nullptr, nullptr, value);
    StructMarshal::PushNullableValue(L, value, ctx->meta->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyStaticSetterNullable(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* setter = method;
    void* value = alloca(ctx->meta->size);
    StructMarshal::PopNullableValue(L, valueIdx, ctx->meta->typeKlass, value);
}

static void PropertyInstanceGetterNullable(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* getter = method;
    void* value = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, nullptr, nullptr, value);
    StructMarshal::PushNullableValue(L, value, ctx->meta->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyInstanceSetterNullable(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* setter = method;
    void* value = alloca(ctx->meta->size);
    StructMarshal::PopNullableValue(L, valueIdx, ctx->meta->typeKlass, value);
}

static void PropertyStaticGetOpaqueValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* getter = method;
    void* value = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, nullptr, nullptr, value);
    OpaqueValueMarshal::Push(L, value, ctx->meta->type);
}

static void PropertyStaticSetOpaqueValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* setter = method;
    void* value = alloca(ctx->meta->size);
    OpaqueValueMarshal::Pop(L, valueIdx, value, ctx->meta->type);
}

static void PropertyInstanceGetOpaqueValue(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* getter = method;
    void* value = alloca(ctx->meta->size);
    getter->invoker_method(getter->methodPointer, getter, nullptr, nullptr, value);
    OpaqueValueMarshal::Push(L, value, ctx->meta->type);
}

static void PropertyInstanceSetOpaqueValue(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    const MethodInfo* setter = method;
    void* value = alloca(ctx->meta->size);
    OpaqueValueMarshal::Pop(L, valueIdx, value, ctx->meta->type);
}

static void PropertyStaticGetterUnityVector2(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef UnityVector2 (*FnGetValue)(const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector2));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector2 value = getValue(getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyStaticSetterUnityVector2(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(UnityVector2 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector2));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector2 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(value, setter);
}

static void PropertyInstanceGetterUnityVector2(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef UnityVector2 (*FnGetValue)(void* target, const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector2));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector2 value = getValue(target, getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyInstanceSetterUnityVector2(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, UnityVector2 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector2));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector2 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(target, value, setter);
}

static void PropertyStaticGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef UnityVector3 (*FnGetValue)(const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector3));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector3 value = getValue(getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyStaticSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(UnityVector3 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector3));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector3 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(value, setter);
}

static void PropertyInstanceGetterUnityVector3(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef UnityVector3 (*FnGetValue)(void* target, const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector3));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector3 value = getValue(target, getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyInstanceSetterUnityVector3(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, UnityVector3 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector3));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector3 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(target, value, setter);
}

static void PropertyStaticGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef UnityVector4 (*FnGetValue)(const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector4));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector4 value = getValue(getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyStaticSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    (void)target;
    typedef void (*FnSetValue)(UnityVector4 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector4));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector4 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(value, setter);
}

static void PropertyInstanceGetterUnityVector4(lua_State* L, void* target, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef UnityVector4 (*FnGetValue)(void* target, const MethodInfo* getter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector4));
    const MethodInfo* getter = method;
    FnGetValue getValue = reinterpret_cast<FnGetValue>(getter->methodPointer);
    UnityVector4 value = getValue(target, getter);
    StructMarshal::PushValue(L, &value, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));
}

static void PropertyInstanceSetterUnityVector4(lua_State* L, void* target, int valueIdx, const MethodInfo* method, const PropertyMarshalCtx* ctx)
{
    typedef void (*FnSetValue)(void* target, UnityVector4 value, const MethodInfo* setter);
    IL2CPP_ASSERT(ctx->meta->size == sizeof(UnityVector4));
    const MethodInfo* setter = method;
    FnSetValue setValue = reinterpret_cast<FnSetValue>(setter->methodPointer);
    UnityVector4 value;
    StructMarshal::PopValue(L, valueIdx, ctx->valueTypeKlass, &value);
    setValue(target, value, setter);
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
