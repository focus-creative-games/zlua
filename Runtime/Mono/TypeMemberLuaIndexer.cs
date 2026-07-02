using System;

namespace ZLua
{
    /// <summary>
    /// Lua-side __index / __newindex per META_TABLE_SPEC.md (methodTable + field getter/setter tables).
    /// </summary>
    internal static class TypeMemberLuaIndexer
    {
        private const string BootstrapChunk = @"
local rawget = rawget

local function bind_indexer(methodTable, fieldGetterTable, fieldSetterTable, extrasTable, isStatic)
    if type(methodTable) ~= 'table' then
        error('bind_indexer: methodTable must be table, got ' .. type(methodTable))
    end
    if type(fieldGetterTable) ~= 'table' then
        error('bind_indexer: fieldGetterTable must be table, got ' .. type(fieldGetterTable))
    end
    if type(fieldSetterTable) ~= 'table' then
        error('bind_indexer: fieldSetterTable must be table, got ' .. type(fieldSetterTable))
    end
    if extrasTable ~= nil and type(extrasTable) ~= 'table' then
        error('bind_indexer: extrasTable must be table or nil, got ' .. type(extrasTable))
    end

    local newindexPrefix = isStatic
        and 'zlua: static member not writable: '
        or 'zlua: instance member not writable: '

    local function index(obj, key)
        local member = rawget(methodTable, key)
        if member ~= nil then
            return member
        end
        local getter = rawget(fieldGetterTable, key)
        if getter ~= nil then
            return getter(obj)
        end
        if extrasTable ~= nil then
            local extra = rawget(extrasTable, key)
            if extra ~= nil then
                return extra
            end
        end
        return nil
    end

    local function newindex(obj, key, value)
        local setter = rawget(fieldSetterTable, key)
        if setter ~= nil then
            setter(obj, key, value)
            return
        end
        if rawget(fieldGetterTable, key) ~= nil then
            error('zlua: property is read-only: ' .. tostring(key))
        end
        error(newindexPrefix .. tostring(key))
    end

    return index, newindex
end

return bind_indexer
";

        private static int _bindIndexerRef;
        private static bool _loaded;

        internal static void EnsureLoaded(IntPtr luaState)
        {
            if (_loaded)
            {
                return;
            }

            if (LuaDllExtension.dostring(luaState, BootstrapChunk) != 0)
            {
                string error = LuaDllExtension.tostring(luaState, -1) ?? "unknown Lua error";
                LuaDll.lua_pop(luaState, 1);
                throw new InvalidOperationException("zlua: failed to load member indexer bootstrap: " + error);
            }

            if (LuaDll.lua_type(luaState, -1) != LuaDataType.Function)
            {
                throw new InvalidOperationException("zlua: member indexer bootstrap must return a function");
            }

            _bindIndexerRef = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
            _loaded = true;
        }

        internal static void BindInstanceMetatable(
            IntPtr luaState,
            int metatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex)
        {
            BindMetatable(
                luaState,
                metatableIndex,
                methodTableIndex,
                fieldGetterTableIndex,
                fieldSetterTableIndex,
                extrasTableIndex: 0,
                isStatic: false);
        }

        internal static void BindStaticMetatable(
            IntPtr luaState,
            int staticMetatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex)
        {
            BindMetatable(
                luaState,
                staticMetatableIndex,
                methodTableIndex,
                fieldGetterTableIndex,
                fieldSetterTableIndex,
                extrasTableIndex: staticMetatableIndex,
                isStatic: true);
        }

        private static void BindMetatable(
            IntPtr luaState,
            int metatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex,
            int extrasTableIndex,
            bool isStatic)
        {
            EnsureLoaded(luaState);
            int absMetatableIndex = LuaDll.lua_absindex(luaState, metatableIndex);

            int methodRef = RefMemberTable(luaState, methodTableIndex, "methodTable");
            int getterRef = RefMemberTable(luaState, fieldGetterTableIndex, "fieldGetterTable");
            int setterRef = RefMemberTable(luaState, fieldSetterTableIndex, "fieldSetterTable");
            int extrasRef = extrasTableIndex != 0
                ? RefMemberTable(luaState, extrasTableIndex, "extrasTable")
                : 0;

            try
            {
                LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, _bindIndexerRef);
                PushMemberTableRef(luaState, methodRef);
                PushMemberTableRef(luaState, getterRef);
                PushMemberTableRef(luaState, setterRef);
                if (extrasRef != 0)
                {
                    PushMemberTableRef(luaState, extrasRef);
                }
                else
                {
                    LuaDll.lua_pushnil(luaState);
                }

                LuaDll.lua_pushboolean(luaState, isStatic ? 1 : 0);

                if (LuaDll.lua_pcall(luaState, 5, 2, 0) != 0)
                {
                    string error = LuaDllExtension.tostring(luaState, -1) ?? "unknown Lua error";
                    LuaDll.lua_pop(luaState, 1);
                    throw new InvalidOperationException("zlua: bind member indexer failed: " + error);
                }

                LuaDll.lua_setfield(luaState, absMetatableIndex, "__newindex");
                LuaDll.lua_setfield(luaState, absMetatableIndex, "__index");
            }
            finally
            {
                UnrefMemberTable(luaState, methodRef);
                UnrefMemberTable(luaState, getterRef);
                UnrefMemberTable(luaState, setterRef);
                if (extrasRef != 0)
                {
                    UnrefMemberTable(luaState, extrasRef);
                }
            }
        }

        private static int RefMemberTable(IntPtr luaState, int tableIndex, string label)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);
            LuaDataType valueType = LuaDll.lua_type(luaState, absIndex);
            if (valueType != LuaDataType.Table)
            {
                throw new InvalidOperationException(
                    $"zlua: bind member indexer expects {label} to be a table at stack index {absIndex}, got {valueType}");
            }

            LuaDll.lua_pushvalue(luaState, absIndex);
            return LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
        }

        private static void PushMemberTableRef(IntPtr luaState, int tableRef)
        {
            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, tableRef);
        }

        private static void UnrefMemberTable(IntPtr luaState, int tableRef)
        {
            if (tableRef != 0)
            {
                LuaDll.luaL_unref(luaState, LuaConsts.LuaRegistryIndex, tableRef);
            }
        }
    }
}
