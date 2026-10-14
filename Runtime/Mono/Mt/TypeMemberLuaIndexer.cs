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

using System;
using System.Runtime.InteropServices;
using ZLua.Utils;

namespace ZLua.Mt
{
    /// <summary>
    /// Lua-side __index / __newindex per META_TABLE_SPEC / Docs-New metatable index
    /// (methodTable + fieldGetterTable + fieldSetterTable).
    /// </summary>
    internal static class TypeMemberLuaIndexer
    {
        // setter(obj, value) — matches Docs-New/spec/metatable/02-INDEX.md
        // resolveSig(key): C# upvalue; full-signature miss → lazy method resolve (§3.7).
        private const string BootstrapChunk = @"
local rawget = rawget
local type = type
local string_find = string.find

local function bind_indexer(methodTable, fieldGetterTable, fieldSetterTable, extrasTable, isStatic, typeFullName, resolveSig)
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
    if type(typeFullName) ~= 'string' or typeFullName == '' then
        error('bind_indexer: typeFullName must be non-empty string')
    end
    if type(resolveSig) ~= 'function' then
        error('bind_indexer: resolveSig must be function')
    end

    local function qualified(key)
        return typeFullName .. '.' .. tostring(key)
    end

    local newindexPrefix = isStatic
        and 'zlua: static member not writable: '
        or 'zlua: instance member not writable: '

    local function index(obj, key)
        local member = rawget(methodTable, key)
        if member ~= nil then
            return member
        end
        if type(key) == 'string' and string_find(key, '(', 1, true) then
            local resolved = resolveSig(key)
            if resolved ~= nil then
                return resolved
            end
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
        if rawget(fieldSetterTable, key) ~= nil then
            error('zlua: property has no getter: ' .. qualified(key))
        end
        error('zlua: member not found: ' .. qualified(key))
    end

    local function newindex(obj, key, value)
        local setter = rawget(fieldSetterTable, key)
        if setter ~= nil then
            setter(obj, value)
            return
        end
        if rawget(fieldGetterTable, key) ~= nil then
            error('zlua: property is read-only: ' .. qualified(key))
        end
        error(newindexPrefix .. qualified(key))
    end

    return index, newindex
end

return bind_indexer
";

        private static readonly LuaCSFunction s_resolveSignature = ResolveSignatureMethod;

        private static int _bindIndexerRef = LuaConsts.LuaNoRef;
        private static bool _loaded;

        internal static void Shutdown(IntPtr luaState)
        {
            if (_bindIndexerRef != LuaConsts.LuaNoRef && luaState != IntPtr.Zero)
            {
                LuaDll.luaL_unref(luaState, LuaConsts.LuaRegistryIndex, _bindIndexerRef);
            }

            _bindIndexerRef = LuaConsts.LuaNoRef;
            _loaded = false;
        }

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
            Type type,
            int metatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex,
            bool isByVal)
        {
            BindMetatable(
                luaState,
                type,
                metatableIndex,
                methodTableIndex,
                fieldGetterTableIndex,
                fieldSetterTableIndex,
                extrasTableIndex: 0,
                isStatic: false,
                isByVal: isByVal);
        }

        internal static void BindStaticMetatable(
            IntPtr luaState,
            Type type,
            int staticMetatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex)
        {
            BindMetatable(
                luaState,
                type,
                staticMetatableIndex,
                methodTableIndex,
                fieldGetterTableIndex,
                fieldSetterTableIndex,
                extrasTableIndex: staticMetatableIndex,
                isStatic: true,
                isByVal: false);
        }

        private static void BindMetatable(
            IntPtr luaState,
            Type type,
            int metatableIndex,
            int methodTableIndex,
            int fieldGetterTableIndex,
            int fieldSetterTableIndex,
            int extrasTableIndex,
            bool isStatic,
            bool isByVal)
        {
            EnsureLoaded(luaState);
            int absMetatableIndex = LuaDll.lua_absindex(luaState, metatableIndex);
            int absMethod = LuaDll.lua_absindex(luaState, methodTableIndex);
            int absGetter = LuaDll.lua_absindex(luaState, fieldGetterTableIndex);
            int absSetter = LuaDll.lua_absindex(luaState, fieldSetterTableIndex);
            int absExtras = extrasTableIndex != 0 ? LuaDll.lua_absindex(luaState, extrasTableIndex) : 0;

            ExpectTable(luaState, absMethod, "methodTable");
            ExpectTable(luaState, absGetter, "fieldGetterTable");
            ExpectTable(luaState, absSetter, "fieldSetterTable");
            if (absExtras != 0)
            {
                ExpectTable(luaState, absExtras, "extrasTable");
            }

            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, _bindIndexerRef);
            LuaDll.lua_pushvalue(luaState, absMethod);
            LuaDll.lua_pushvalue(luaState, absGetter);
            LuaDll.lua_pushvalue(luaState, absSetter);
            if (absExtras != 0)
            {
                LuaDll.lua_pushvalue(luaState, absExtras);
            }
            else
            {
                LuaDll.lua_pushnil(luaState);
            }

            LuaDll.lua_pushboolean(luaState, isStatic ? 1 : 0);
            LuaDll.lua_pushstring(luaState, TypeRegistry.GetLuaFullName(type));

            // resolveSig upvalues: type, isStatic, isByVal
            TypeHandleStore.PushLightUserData(luaState, type);
            LuaDll.lua_pushboolean(luaState, isStatic ? 1 : 0);
            LuaDll.lua_pushboolean(luaState, isByVal ? 1 : 0);
            LuaCallbackGate.PushCClosure(
                luaState,
                Marshal.GetFunctionPointerForDelegate(s_resolveSignature),
                3);

            if (LuaDll.lua_pcall(luaState, 7, 2, 0) != 0)
            {
                string error = LuaDllExtension.tostring(luaState, -1) ?? "unknown Lua error";
                LuaDll.lua_pop(luaState, 1);
                throw new InvalidOperationException("zlua: bind member indexer failed: " + error);
            }

            // pcall returns index, newindex — set __newindex first (top), then __index
            LuaDll.lua_setfield(luaState, absMetatableIndex, LuaConsts.MetaNewIndex);
            LuaDll.lua_setfield(luaState, absMetatableIndex, LuaConsts.MetaIndex);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ResolveSignatureMethod(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_type(L, 1) != LuaDataType.String)
                {
                    LuaDll.lua_pushnil(L);
                    return 1;
                }

                string key = LuaDllExtension.tostring(L, 1);
                if (string.IsNullOrEmpty(key) || key.IndexOf('(') < 0)
                {
                    LuaDll.lua_pushnil(L);
                    return 1;
                }

                Type type = TypeHandleStore.ReadLightUserData(
                    L, LuaDll.lua_upvalueindex(LuaCallbackGate.ManagedUpvalueIndex(1)));
                bool isStatic = LuaDll.lua_toboolean(
                    L, LuaDll.lua_upvalueindex(LuaCallbackGate.ManagedUpvalueIndex(2))) != 0;
                bool isByVal = LuaDll.lua_toboolean(
                    L, LuaDll.lua_upvalueindex(LuaCallbackGate.ManagedUpvalueIndex(3))) != 0;

                if (MetaBinding.TryResolveSignatureMethodAndPush(L, type, key, isStatic, isByVal) == 1)
                {
                    return 1;
                }

                LuaDll.lua_pushnil(L);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        private static void ExpectTable(IntPtr luaState, int index, string label)
        {
            if (LuaDll.lua_type(luaState, index) != LuaDataType.Table)
            {
                throw new InvalidOperationException(
                    $"zlua: bind member indexer expects {label} to be a table, got {LuaDll.lua_type(luaState, index)}");
            }
        }
    }
}
