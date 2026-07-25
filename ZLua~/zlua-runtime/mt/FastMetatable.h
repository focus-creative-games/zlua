#pragma once

#include "../ZLuaCommon.h"
#include "MetaBinding.h"

#if ZLUA_FAST_METATABLE

namespace zlua
{

class FastMetatable
{
  public:
    static FastMemberCtx* AllocCtx(TypeBinding* binding, FastInstanceKind kind, const MetaInfo& info);

    static void FillMemberTables(lua_State* L, TypeBinding* binding, const NameMetaMap* map, FastInstanceKind kind, int memberTableIndex,
                                 int setterTableIndex);

    static void SealMetatable(lua_State* L, int mtIndex);

    /* Late-bound method alias (zlua.register_method): rawset closure into sealed __index table. */
    static void RawSetMethodOnIndexTable(lua_State* L, Il2CppClass* klass, bool isStatic, bool isByVal, const char* aliasName, int closureStackIndex);
};

} // namespace zlua

#endif /* ZLUA_FAST_METATABLE */
