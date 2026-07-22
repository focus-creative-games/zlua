#include "FastMetatable.h"

#if ZLUA_FAST_METATABLE

#include "InstanceTarget.h"
#include "TypeRegistry.h"

#include "../LuaConsts.h"
#include "../utils/LuaUtil.h"

extern "C"
{
#include "lua/zlua_fastmt.h"
}

namespace zlua
{

FastMemberCtx* FastMetatable::AllocCtx(TypeBinding* binding, FastInstanceKind kind, const MetaInfo& info)
{
    binding->fastMemberStorage.push_back(FastMemberCtx{kind, info});
    return &binding->fastMemberStorage.back();
}

static void* ResolveFieldAddress(lua_State* L, int index, const FastMemberCtx* ctx)
{
    switch (ctx->kind)
    {
    case FastInstanceKind::StructByVal:
        return InstanceTarget::ResolveByValFieldAddress(L, index, ctx->info.field.instanceOffsetIncludingHeader - (int32_t)sizeof(Il2CppObject));
    case FastInstanceKind::StructByObj:
    case FastInstanceKind::ReferenceByObj:
        return InstanceTarget::ResolveByObjFieldAddress(L, index, ctx->info.field.instanceOffsetIncludingHeader);
    case FastInstanceKind::Static:
        return ctx->info.field.staticAddress;
    default:
        IL2CPP_ASSERT(false);
        return nullptr;
    }
}

static void* ResolvePropertyTarget(lua_State* L, int index, FastInstanceKind kind)
{
    switch (kind)
    {
    case FastInstanceKind::Static:
        return nullptr;
    case FastInstanceKind::StructByVal:
        return InstanceTarget::ResolveByValMethodTarget(L, index);
    case FastInstanceKind::StructByObj:
        return InstanceTarget::ResolveByObjValueTypeMethodTarget(L, index);
    case FastInstanceKind::ReferenceByObj:
        return InstanceTarget::ResolveByObjReferenceMethodTarget(L, index);
    default:
        IL2CPP_ASSERT(false);
        return nullptr;
    }
}

} // namespace zlua

extern "C" LUA_API int zlua_index_getter(lua_State* L)
{
    using namespace zlua;
    const FastMemberCtx* ctx = (const FastMemberCtx*)lua_touserdata(L, lua_upvalueindex(1));
    IL2CPP_ASSERT(ctx != nullptr);

    switch (ctx->info.kind)
    {
    case MetaKind::Field:
    {
        const MarshalMetaInfo* meta = ctx->info.field.meta;
        FnMarshalCs2Lua writer = meta->cs2luaWriter;
        if (writer == nullptr)
            return luaL_error(L, "zlua: field is write-only");
        void* fieldPtr = ResolveFieldAddress(L, 1, ctx);
        IL2CPP_ASSERT(fieldPtr != nullptr);
        writer(L, fieldPtr, meta);
        return 1;
    }
    case MetaKind::Property:
    {
        FnPropertyGetter getter = ctx->info.property.getter;
        if (getter == nullptr)
            return luaL_error(L, "zlua: property has no getter");
        void* target = ResolvePropertyTarget(L, 1, ctx->kind);
        getter(L, target, &ctx->info.property);
        return 1;
    }
    default:
        return luaL_error(L, "zlua: invalid index getter");
    }
}

extern "C" LUA_API int zlua_index_setter(lua_State* L)
{
    using namespace zlua;
    const FastMemberCtx* ctx = (const FastMemberCtx*)lua_touserdata(L, lua_upvalueindex(1));
    IL2CPP_ASSERT(ctx != nullptr);

    switch (ctx->info.kind)
    {
    case MetaKind::Field:
    {
        const MarshalMetaInfo* meta = ctx->info.field.meta;
        FnMarshalLua2Cs writer = meta->lua2csWriter;
        if (writer == nullptr)
            return luaL_error(L, "zlua: field is read-only");
        void* fieldPtr = ResolveFieldAddress(L, 1, ctx);
        IL2CPP_ASSERT(fieldPtr != nullptr);
        writer(L, 2, fieldPtr, meta);
        return 0;
    }
    case MetaKind::Property:
    {
        FnPropertySetter setter = ctx->info.property.setter;
        if (setter == nullptr)
            return luaL_error(L, "zlua: property is read-only");
        void* target = ResolvePropertyTarget(L, 1, ctx->kind);
        setter(L, target, 2, &ctx->info.property);
        return 0;
    }
    default:
        return luaL_error(L, "zlua: invalid index setter");
    }
}

namespace zlua
{

void FastMetatable::FillMemberTables(lua_State* L, TypeBinding* binding, const NameMetaMap* map, FastInstanceKind kind, int memberTableIndex,
                                     int setterTableIndex)
{
    for (NameMetaMap::const_iterator it = map->begin(); it != map->end(); ++it)
    {
        const char* name = it->first;
        const MetaInfo& info = it->second;
        switch (info.kind)
        {
        case MetaKind::Method:
            LuaUtil::PushRef(L, info.method.closureRef);
            lua_setfield(L, memberTableIndex, name);
            break;
        case MetaKind::Field:
        case MetaKind::Property:
        {
            const bool hasSetter = (info.kind == MetaKind::Field) ? (info.field.meta != nullptr && info.field.meta->lua2csWriter != nullptr)
                                                                  : (info.property.setter != nullptr);
            /* Always install getter so write-only members error like Dispatch* (not silent nil). */
            FastMemberCtx* ctx = AllocCtx(binding, kind, info);
            lua_pushlightuserdata(L, ctx);
            lua_pushcclosure(L, zlua_index_getter, 1);
            lua_setfield(L, memberTableIndex, name);
            if (hasSetter)
            {
                lua_pushlightuserdata(L, ctx);
                lua_pushcclosure(L, zlua_index_setter, 1);
                lua_setfield(L, setterTableIndex, name);
            }
            break;
        }
        default:
            break;
        }
    }
}

void FastMetatable::SealMetatable(lua_State* L, int mtIndex)
{
    zlua_mt_seal(L, mtIndex);
}

void FastMetatable::RawSetMethodOnIndexTable(lua_State* L, Il2CppClass* klass, bool isStatic, bool isByVal, const char* aliasName, int closureStackIndex)
{
    IL2CPP_ASSERT(klass != nullptr);
    IL2CPP_ASSERT(aliasName != nullptr);
    const int top = lua_gettop(L);
    closureStackIndex = lua_absindex(L, closureStackIndex);

    TypeRegistry::PushInternedTypeTable(L, klass);
    const int typeTableIndex = lua_gettop(L);

    if (isStatic)
    {
        if (lua_getmetatable(L, typeTableIndex) == 0)
        {
            lua_settop(L, top);
            return;
        }
    }
    else
    {
        lua_getfield(L, typeTableIndex, isByVal ? LuaConsts::ByValInstanceMt : LuaConsts::ByObjInstanceMt);
        if (!lua_istable(L, -1))
        {
            lua_settop(L, top);
            return;
        }
    }
    const int mtIndex = lua_gettop(L);

    lua_getfield(L, mtIndex, LuaConsts::MetaIndex);
    if (!lua_istable(L, -1))
    {
        lua_settop(L, top);
        return;
    }
    const int memberTableIndex = lua_gettop(L);

    lua_pushstring(L, aliasName);
    lua_pushvalue(L, closureStackIndex);
    lua_rawset(L, memberTableIndex);

    lua_settop(L, top);
}

} // namespace zlua

#endif /* ZLUA_FAST_METATABLE */
