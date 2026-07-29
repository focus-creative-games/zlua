#include "TypeRegistryCommon.h"

#include "MetaBinding.h"
#include "InstanceTarget.h"

#include "../LuaConsts.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaUtil.h"
#include "../utils/MetadataUtil.h"
#include "../bridge/PropertyBridge.h"
#include "../bridge/MethodBridge.h"
#include "../bridge/DelegateBridge.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/StructRegistry.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/ObjectRegistry.h"

#if ZLUA_FAST_METATABLE
#include "FastMetatable.h"
#endif

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
            TypedMarshal::PushByType(L, value, type);
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

#if ZLUA_FAST_METATABLE
    (void)instanceIndex;
    (void)instanceNewIndex;
#else
    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, instanceIndex, 1);
    lua_setfield(L, mtIndex, LuaConsts::MetaIndex);

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, instanceNewIndex, 1);
    lua_setfield(L, mtIndex, LuaConsts::MetaNewIndex);
#endif

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

#if ZLUA_FAST_METATABLE
static void AttachFastInstanceIndexTables(lua_State* L, TypeBinding* binding, const NameMetaMap* map, FastInstanceKind kind, int mtIndex)
{
    lua_newtable(L); /* memberTable */
    lua_newtable(L); /* setterTable */
    const int setterTableIndex = lua_gettop(L);
    const int memberTableIndex = setterTableIndex - 1;
    FastMetatable::FillMemberTables(L, binding, map, kind, memberTableIndex, setterTableIndex);
    lua_pushvalue(L, memberTableIndex);
    lua_setfield(L, mtIndex, LuaConsts::MetaIndex);
    lua_pushvalue(L, setterTableIndex);
    lua_setfield(L, mtIndex, LuaConsts::MetaNewIndex);
    lua_pop(L, 2);
    FastMetatable::SealMetatable(L, mtIndex);
}
#endif

static int DelegateInstanceCall(lua_State* L)
{
    Il2CppObject* obj = TypeRegistryCommon::GetByObjThis(L, 1);
    IL2CPP_ASSERT(obj != nullptr);
    IL2CPP_ASSERT(MetadataUtil::IsDelegateClass(obj->klass));

    Il2CppDelegate* delegate = reinterpret_cast<Il2CppDelegate*>(obj);
    // TODO: optimize this, use a faster way to get the invoke method
    const MethodInfo* invokeMethod = il2cpp::vm::Runtime::GetDelegateInvoke(obj->klass);
    IL2CPP_ASSERT(invokeMethod != nullptr);

    const MethodMarshalCtx* invokeCtx = DelegateBridge::GetOrCreateMethodMarshalCtx(L, invokeMethod);
    const int luaArgCount = lua_gettop(L) - 1;
    if (luaArgCount != invokeCtx->luaArity)
    {
        return luaL_error(L, "zlua: argument mismatch: delegate invoke expects %d argument(s), got %d", invokeCtx->luaArity, luaArgCount);
    }

    return MethodBridge::InvokeLua2Cs(L, delegate, 2, invokeCtx);
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
            if (info->property.getter == nullptr)
                return luaL_error(L, "zlua: property has no getter: %s", key);
            void* target = resolveMethodTarget(L, 1);
            IL2CPP_ASSERT(target != nullptr);
            PropertyBridge::InvokeGetter(L, target, &info->property);
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
        if (info->property.setter == nullptr)
            return luaL_error(L, "zlua: property is read-only: %s", key);
        void* target = resolveMethodTarget(L, 1);
        IL2CPP_ASSERT(target != nullptr);
        PropertyBridge::InvokeSetter(L, target, 3, &info->property);
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
            if (info->property.getter == nullptr)
                return luaL_error(L, "zlua: property has no getter: %s", key);
            PropertyBridge::InvokeGetter(L, nullptr, &info->property);
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
            return luaL_error(L, "zlua: member not found or read-only: %s", key);
        writer(L, 3, info->field.staticAddress, meta);
        return 0;
    }
    case MetaKind::Property:
    {
        if (info->property.setter == nullptr)
            return luaL_error(L, "zlua: member not found or read-only: %s", key);
        PropertyBridge::InvokeSetter(L, nullptr, 3, &info->property);
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
#if ZLUA_FAST_METATABLE
    AttachFastInstanceIndexTables(L, binding, &binding->byvalInstanceMap, FastInstanceKind::StructByVal, mtIndex);
#endif

    lua_setfield(L, typeTableIndex, LuaConsts::ByValInstanceMt);
}

void TypeRegistryCommon::AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int mtIndex = stackGuard.GetTop();
    FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, LuaConsts::UdKindByObj, ObjectRegistry::OnReleaseObjectUserData, tostring, InstanceByObjIndex,
                          InstanceByObjNewIndex);
#if ZLUA_FAST_METATABLE
    AttachFastInstanceIndexTables(L, binding, &binding->byobjInstanceMap, FastInstanceKind::StructByObj, mtIndex);
#endif
    lua_setfield(L, typeTableIndex, LuaConsts::ByObjInstanceMt);
}

void TypeRegistryCommon::AttachReferenceInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding)
{
    lua_newtable(L);

    LuaStackGuard stackGuard(L);
    const int mtIndex = stackGuard.GetTop();
    FillInstanceMetatable(L, binding, typeTableIndex, mtIndex, LuaConsts::UdKindByObj, ObjectRegistry::OnReleaseObjectUserData, nullptr, InstanceReferenceIndex,
                          InstanceReferenceNewIndex);
#if ZLUA_FAST_METATABLE
    AttachFastInstanceIndexTables(L, binding, &binding->byobjInstanceMap, FastInstanceKind::ReferenceByObj, mtIndex);
#endif
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

#if ZLUA_FAST_METATABLE
    lua_newtable(L); /* memberTable */
    lua_newtable(L); /* setterTable */
    const int setterTableIndex = lua_gettop(L);
    const int memberTableIndex = setterTableIndex - 1;
    FastMetatable::FillMemberTables(L, binding, &binding->staticMap, FastInstanceKind::Static, memberTableIndex, setterTableIndex);
    lua_pushvalue(L, memberTableIndex);
    lua_setfield(L, smtIndex, LuaConsts::MetaIndex);
    lua_pushvalue(L, setterTableIndex);
    lua_setfield(L, smtIndex, LuaConsts::MetaNewIndex);
    lua_pop(L, 2);
    FastMetatable::SealMetatable(L, smtIndex);
#else
    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, StaticIndex, 1);
    lua_setfield(L, smtIndex, LuaConsts::MetaIndex);

    lua_pushlightuserdata(L, binding);
    lua_pushcclosure(L, StaticNewIndex, 1);
    lua_setfield(L, smtIndex, LuaConsts::MetaNewIndex);
#endif

    lua_setmetatable(L, typeTableIndex);
}



} // namespace zlua
