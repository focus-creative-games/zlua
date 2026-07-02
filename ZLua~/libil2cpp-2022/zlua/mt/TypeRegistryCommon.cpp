#include "TypeRegistryCommon.h"

#include "MetaBinding.h"
#include "InstanceTarget.h"

#include "../LuaConsts.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaUtil.h"
#include "../utils/MetadataUtil.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/StructRegistry.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/Marshaling.h"
#include "../marshal/ObjectRegistry.h"

#include "il2cpp-api-types.h"
#include "il2cpp-blob.h"
#include "vm/Field.h"
#include "vm/Runtime.h"

namespace zlua
{

Il2CppObject* TypeRegistryCommon::GetByObjThis(lua_State* L, int index)
{
    Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, index);
    IL2CPP_ASSERT(obj != nullptr);
    return obj;
}

void TypeRegistryCommon::RegisterNativeInstanceMethod(lua_State* L, TypeBinding* binding, const char* name, lua_CFunction fn)
{
    if (binding->byobjInstanceMap.find(name) != binding->byobjInstanceMap.end())
        luaL_error(L, "zlua: duplicate instance method: %s", name);

    lua_pushcfunction(L, fn);
    MetaInfo info = {};
    info.kind = MetaKind::Method;
    info.method.closureRef = luaL_ref(L, LUA_REGISTRYINDEX);
    binding->byobjInstanceMap[name] = info;
}

int TypeRegistryCommon::TypeTableToString(lua_State* L)
{
    lua_getfield(L, -1, LuaConsts::FullName);
    IL2CPP_ASSERT(lua_isstring(L, -1));
    return 1;
}

int TypeRegistryCommon::ObjectInstanceToString(lua_State* L)
{
    Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, 1);
    IL2CPP_ASSERT(obj != nullptr);
    Il2CppException* ex = nullptr;
    Il2CppObject* str = il2cpp::vm::Runtime::Invoke(MetadataUtil::GetObjectToStringMethod(), obj, nullptr, &ex);
    if (ex != nullptr)
    {
        return luaL_error(L, "zlua: failed to invoke ToString method");
    }
    StringMarshal::Push(L, (Il2CppString*)str);
    return 1;
}

void TypeRegistryCommon::WriteCommonTypeFields(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding)
{
    const std::string fullName = MetadataUtil::GetLuaFullName(klass);
    lua_pushstring(L, fullName.c_str());
    lua_setfield(L, typeTableIndex, LuaConsts::FullName);
    lua_pushlightuserdata(L, klass);
    lua_setfield(L, typeTableIndex, LuaConsts::Klass);
}

void TypeRegistryCommon::RegisterStaticLiteralFields(lua_State* L, Il2CppClass* klass, int typeTableIndex)
{
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        FieldInfo* field = klass->fields + i;
        if (!MetadataUtil::IsStaticLiteralField(field))
            continue;
        uint8_t value[8];
        il2cpp::vm::Field::GetDefaultFieldValue(field, value);

        const Il2CppType* type = field->type;
    restart:
        switch (type->type)
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
        case IL2CPP_TYPE_STRING:
        {
            Marshaling::PushByType(L, value, type);
            break;
        }
        case IL2CPP_TYPE_VALUETYPE:
        case IL2CPP_TYPE_GENERICINST:
        {
            Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(field->type);
            if (klass->enumtype)
            {
                type = il2cpp::vm::Class::GetEnumBaseType(klass);
                goto restart;
            }
            continue;
        }
        default:
            continue;
        }
        lua_setfield(L, typeTableIndex, field->name);
    }
}

static void FillInstanceMetatable(lua_State* L, TypeBinding* binding, int typeTableIndex, int mtIndex, const char* userDataKind, lua_CFunction gc,
                                  lua_CFunction tostring, lua_CFunction instanceIndex, lua_CFunction instanceNewIndex)
{
    lua_pushvalue(L, typeTableIndex);
    lua_setfield(L, mtIndex, LuaConsts::Type);

    if (userDataKind != nullptr)
    {
        lua_pushstring(L, userDataKind);
        lua_setfield(L, mtIndex, LuaConsts::UdKind);
    }

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, instanceIndex, 1);
    lua_setfield(L, mtIndex, LuaConsts::MetaIndex);

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, instanceNewIndex, 1);
    lua_setfield(L, mtIndex, LuaConsts::MetaNewIndex);

    if (gc != nullptr)
    {
        lua_pushcfunction(L, gc);
        lua_setfield(L, mtIndex, LuaConsts::MetaGc);
    }

    if (tostring != nullptr)
    {
        lua_pushcfunction(L, tostring);
        lua_setfield(L, mtIndex, LuaConsts::MetaToString);
    }
}

static int DelegateInstanceCall(lua_State* L)
{
    Il2CppObject* obj = TypeRegistryCommon::GetByObjThis(L, 1);
    IL2CPP_ASSERT(obj != nullptr);
    IL2CPP_ASSERT(MetadataUtil::IsDelegateClass(obj->klass));

    Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(obj);
    // TODO: optimize this, use a faster way to get the invoke method
    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(obj->klass);
    IL2CPP_ASSERT(invokeMethod != nullptr);

    const int luaArgCount = lua_gettop(L) - 1;
    if (luaArgCount != invokeMethod->parameters_count)
    {
        return luaL_error(L, "zlua: delegate invoke expects %d argument(s), got %d", invokeMethod->parameters_count, luaArgCount);
    }

    const int paramCount = invokeMethod->parameters_count;
    std::vector<void*> params((size_t)paramCount, nullptr);

    for (int i = 0; i < paramCount; ++i)
    {
        const Il2CppType* paramType = invokeMethod->parameters[i];
        const size_t sz = MetadataUtil::GetValueSize(paramType);
        bool isReferenceOrByRefOrPtr =
            il2cpp::vm::Type::IsReference(paramType) || paramType->byref || paramType->type == IL2CPP_TYPE_PTR || paramType->type == IL2CPP_TYPE_BYREF;
        void* dataPtr;
        if (isReferenceOrByRefOrPtr)
            dataPtr = &params[i];
        else
        {
            dataPtr = alloca(sz);
            params[i] = dataPtr;
        }
        Marshaling::PopByType(L, i + 2, dataPtr, paramType);
    }

    bool isVoidReturn = invokeMethod->return_type->type == IL2CPP_TYPE_VOID;

    void* ret = isVoidReturn ? nullptr : alloca(MetadataUtil::GetValueSize(invokeMethod->return_type));
    // FIXME: handle managed exception
    invokeMethod->invoker_method(invokeMethod->methodPointer, invokeMethod, delegate, params.data(), ret);

    if (isVoidReturn)
    {
        return 0;
    }
    else
    {
        Marshaling::PushByType(L, ret, invokeMethod->return_type);
        return 1;
    }
}

static const MetaInfo* LookupMeta(const NameMetaMap* map, const char* key)
{
    NameMetaMap::const_iterator it = map->find(key);
    if (it == map->end())
        return nullptr;
    return &it->second;
}

using ResolveInstanceFieldFn = void* (*)(lua_State*, int, int32_t);
using ResolveInstanceMethodTargetFn = void* (*)(lua_State*, int);

template <MetaTableKind kind>
void* ResolveInstanceFieldAddress(lua_State* L, int index, const FieldMarshalCtx& ctx)
{
    switch (kind)
    {
    case MetaTableKind::ReferenceByObj:
    case MetaTableKind::StructByObj:
    {
        return InstanceTarget::ResolveByObjFieldAddress(L, index, ctx.instanceOffsetIncludingHeader);
    }
    case MetaTableKind::StructByVal:
    {
        return InstanceTarget::ResolveByValFieldAddress(L, index, ctx.instanceOffsetIncludingHeader - sizeof(Il2CppObject));
    }
    default:
        IL2CPP_ASSERT(false);
        return nullptr;
    }
}

template <ResolveInstanceMethodTargetFn resolveMethodTarget, MetaTableKind kind>
static int DispatchInstanceIndex(lua_State* L)
{
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
    const char* key = luaL_checkstring(L, 2);
    const MetaInfo* info = LookupMeta(kind == MetaTableKind::StructByVal ? &binding->byvalInstanceMap : &binding->byobjInstanceMap, key);
    if (info != nullptr)
    {
        switch (info->kind)
        {
        case MetaKind::Method:
            return LuaUtil::PushRef(L, info->method.closureRef);
        case MetaKind::Field:
        {
            const MarshalMetaInfo* meta = info->field.meta;
            FnMarshalCs2Lua writer = meta->cs2luaWriter;
            if (writer == nullptr)
                return luaL_error(L, "zlua: field is read-only: %s", key);
            void* fieldPtr = ResolveInstanceFieldAddress<kind>(L, 1, info->field);
            IL2CPP_ASSERT(fieldPtr != nullptr);
            writer(L, fieldPtr, meta);
            return 1;
        }
        case MetaKind::Property:
        {
            FnPropertyGetter getter = info->property.getter;
            if (getter == nullptr)
                return luaL_error(L, "zlua: property has no getter: %s", key);
            void* target = resolveMethodTarget(L, 1);
            IL2CPP_ASSERT(target != nullptr);
            getter(L, target, &info->property);
            return 1;
        }
        // case MetaKind::Event:
        //     return PushEventTable(L, *info);
        default:
            break;
        }
    }

    lua_pushnil(L);
    return 1;
}

template <ResolveInstanceMethodTargetFn resolveMethodTarget, MetaTableKind kind>
static int DispatchInstanceNewIndex(lua_State* L)
{
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
    const char* key = lua_tostring(L, 2);
    const MetaInfo* info = LookupMeta(kind == MetaTableKind::StructByVal ? &binding->byvalInstanceMap : &binding->byobjInstanceMap, key);
    if (info == nullptr)
        return luaL_error(L, "zlua: member not found: %s", key != nullptr ? key : "");

    switch (info->kind)
    {
    case MetaKind::Field:
    {
        const MarshalMetaInfo* meta = info->field.meta;
        FnMarshalLua2Cs writer = meta->lua2csWriter;
        if (writer == nullptr)
            return luaL_error(L, "zlua: field is read-only: %s", key);
        void* fieldPtr = ResolveInstanceFieldAddress<kind>(L, 1, info->field);
        IL2CPP_ASSERT(fieldPtr != nullptr);
        writer(L, 3, fieldPtr, meta);
        return 0;
    }
    case MetaKind::Property:
    {
        FnPropertySetter setter = info->property.setter;
        if (setter == nullptr)
            return luaL_error(L, "zlua: property is read-only: %s", key);
        void* target = resolveMethodTarget(L, 1);
        IL2CPP_ASSERT(target != nullptr);
        setter(L, target, 3, &info->property);
        return 0;
    }
    // case MetaKind::Event:
    // {
    //     if (info->event.setterRef == LUA_NOREF)
    //         return luaL_error(L, "zlua: event does not support assignment: %s", key);
    //     const int args[] = {1, 3};
    //     return LuaUtil::PCallClosureRefAt(L, info->event.setterRef, args, 2, 0);
    // }
    default:
        return luaL_error(L, "zlua: cannot assign to method: %s", key);
    }
}

int StaticIndex(lua_State* L)
{
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
    const char* key = luaL_checkstring(L, 2);
    const MetaInfo* info = LookupMeta(&binding->staticMap, key);
    if (info != nullptr)
    {
        switch (info->kind)
        {
        case MetaKind::Method:
            return LuaUtil::PushRef(L, info->method.closureRef);
        case MetaKind::Field:
        {
            const MarshalMetaInfo* meta = info->field.meta;
            FnMarshalCs2Lua writer = meta->cs2luaWriter;
            if (writer == nullptr)
            {
                return luaL_error(L, "zlua: field is write-only: %s", key);
            }
            IL2CPP_ASSERT(info->field.staticAddress != nullptr);
            writer(L, info->field.staticAddress, meta);
            return 1;
        }
        case MetaKind::Property:
        {
            FnPropertyGetter getter = info->property.getter;
            if (getter == nullptr)
                return luaL_error(L, "zlua: property has no getter: %s", key);
            getter(L, nullptr, &info->property);
            return 1;
        }
        // case MetaKind::Event:
        //     return PushEventTable(L, *info);
        default:
            break;
        }
    }

    // Extras such as _default / __call live on the static metatable (SMT), not the type table.
    if (lua_getmetatable(L, 1) == 0)
        return 0;

    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int DispatchStaticNewIndex(lua_State* L, const NameMetaMap* map)
{
    const char* key = lua_tostring(L, 2);
    const MetaInfo* info = LookupMeta(map, key);
    if (info == nullptr)
        return luaL_error(L, "zlua: member not found: %s", key != nullptr ? key : "");

    switch (info->kind)
    {
    case MetaKind::Field:
    {
        const MarshalMetaInfo* meta = info->field.meta;
        FnMarshalLua2Cs writer = meta->lua2csWriter;
        if (writer == nullptr)
            return luaL_error(L, "zlua: field is read-only: %s", key);
        writer(L, 3, info->field.staticAddress, meta);
        return 0;
    }
    case MetaKind::Property:
    {
        FnPropertySetter setter = info->property.setter;
        if (setter == nullptr)
            return luaL_error(L, "zlua: property is read-only: %s", key);
        setter(L, nullptr, 3, &info->property);
        return 0;
    }
    // case MetaKind::Event:
    // {
    //     if (info->event.setterRef == LUA_NOREF)
    //         return luaL_error(L, "zlua: event does not support assignment: %s", key);
    //     const int args[] = {3};
    //     return LuaUtil::PCallClosureRefAt(L, info->event.setterRef, args, 1, 0);
    // }
    default:
        return luaL_error(L, "zlua: cannot assign to method: %s", key);
    }
}

int InstanceByValIndex(lua_State* L)
{
    return DispatchInstanceIndex<InstanceTarget::ResolveByValMethodTarget, MetaTableKind::StructByVal>(L);
}

int InstanceByObjIndex(lua_State* L)
{
    return DispatchInstanceIndex<InstanceTarget::ResolveByObjValueTypeMethodTarget, MetaTableKind::ReferenceByObj>(L);
}

int InstanceReferenceIndex(lua_State* L)
{
    return DispatchInstanceIndex<InstanceTarget::ResolveByObjReferenceMethodTarget, MetaTableKind::ReferenceByObj>(L);
}

int InstanceByValNewIndex(lua_State* L)
{
    return DispatchInstanceNewIndex<InstanceTarget::ResolveByValMethodTarget, MetaTableKind::StructByVal>(L);
}

int InstanceByObjNewIndex(lua_State* L)
{
    return DispatchInstanceNewIndex<InstanceTarget::ResolveByObjValueTypeMethodTarget, MetaTableKind::ReferenceByObj>(L);
}

int InstanceReferenceNewIndex(lua_State* L)
{
    return DispatchInstanceNewIndex<InstanceTarget::ResolveByObjReferenceMethodTarget, MetaTableKind::ReferenceByObj>(L);
}

int StaticNewIndex(lua_State* L)
{
    TypeBinding* binding = (TypeBinding*)lua_touserdata(L, lua_upvalueindex(1));
    return DispatchStaticNewIndex(L, &binding->staticMap);
}

void TypeRegistryCommon::AttachByValInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int mtIndex = stackGuard.GetTop();
    FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, LuaConsts::UdKindByVal, klass->is_blittable ? nullptr : StructRegistry::OnReleaseByValUserData,
                          tostring, InstanceByValIndex, InstanceByValNewIndex);

    lua_setfield(L, typeTableIndex, LuaConsts::ByValInstanceMt);
}

void TypeRegistryCommon::AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int mtIndex = stackGuard.GetTop();
    FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, LuaConsts::UdKindByObj, ObjectRegistry::OnReleaseObjectUserData, tostring, InstanceByObjIndex,
                          InstanceByObjNewIndex);
    lua_setfield(L, typeTableIndex, LuaConsts::ByObjInstanceMt);
}

void TypeRegistryCommon::AttachReferenceInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int mtIndex = stackGuard.GetTop();
    FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, LuaConsts::UdKindByObj, ObjectRegistry::OnReleaseObjectUserData, nullptr, InstanceReferenceIndex,
                          InstanceReferenceNewIndex);
    if (MetadataUtil::IsDelegateClass(klass))
    {
        lua_pushcfunction(L, DelegateInstanceCall);
        lua_setfield(L, mtIndex, LuaConsts::MetaCall);
    }
    lua_setfield(L, typeTableIndex, LuaConsts::ByObjInstanceMt);
}

void TypeRegistryCommon::AttachStaticTypeMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction callFn, lua_CFunction extraFn,
                                                   const char* extraFieldName, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int smtIndex = stackGuard.GetTop();

    if (callFn != nullptr)
    {
        lua_pushlightuserdata(L, binding);
        lua_pushcclosure(L, callFn, 1);
        lua_setfield(L, smtIndex, LuaConsts::MetaCall);
    }

    if (extraFn != nullptr && extraFieldName != nullptr)
    {
        lua_pushlightuserdata(L, klass);
        lua_pushcclosure(L, extraFn, 1);
        lua_setfield(L, smtIndex, extraFieldName);
    }

    lua_pushcfunction(L, TypeTableToString);
    lua_setfield(L, smtIndex, LuaConsts::MetaToString);

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, StaticIndex, 1);
    lua_setfield(L, smtIndex, LuaConsts::MetaIndex);

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, StaticNewIndex, 1);
    lua_setfield(L, smtIndex, LuaConsts::MetaNewIndex);

    lua_setmetatable(L, typeTableIndex);
}



} // namespace zlua
