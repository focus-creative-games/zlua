-- ZLua SLua adaptor: Namespace.Type → _G.Namespace.Type → CSharp[assembly][typeFullName]
-- Spec: docs/spec/12-MIGRATION-ADAPTORS.md
-- No-namespace types: eager rawset on _G at init.
-- Namespaced types: namespace tables only; leaf resolved lazily via __index.
-- Nested CLR names (Outer+Inner) mount as Outer.Inner.

local M = {}

-- [ns_table] = { [leaf_name] = { asm = string, full = string } }
local pending = {}
local resolved_owners = {}

local function resolve_type(assembly_name, type_full_name)
    local asm = CSharp[assembly_name]
    if asm == nil then
        error(string.format(
            "zlua slua adaptor: assembly not found: %s (type %s)",
            tostring(assembly_name), tostring(type_full_name)))
    end
    local t = asm[type_full_name]
    if t == nil then
        error(string.format(
            "zlua slua adaptor: type not found: CSharp[%q][%q]",
            assembly_name, type_full_name))
    end
    return t
end

local function path_segments(type_full_name)
    local normalized = (type_full_name or ""):gsub("%+", ".")
    local parts = {}
    for part in normalized:gmatch("[^.]+") do
        parts[#parts + 1] = part
    end
    return parts
end

local ns_mt = {
    __index = function(t, key)
        local map = pending[t]
        local info = map and map[key]
        if info == nil then
            return nil
        end
        local type_table = resolve_type(info.asm, info.full)
        map[key] = nil
        local owners = resolved_owners[t]
        if owners == nil then
            owners = {}
            resolved_owners[t] = owners
        end
        owners[key] = { asm = info.asm, full = info.full }
        rawset(t, key, type_table)
        return type_table
    end,
}

local function new_ns_table()
    local t = {}
    pending[t] = {}
    setmetatable(t, ns_mt)
    return t
end

local function ensure_ns_table(parent, name)
    local cur = rawget(parent, name)
    if cur == nil then
        local parent_pending = pending[parent]
        if parent_pending and parent_pending[name] ~= nil then
            error(string.format(
                "zlua slua adaptor: path conflict at %s (type leaf vs namespace)",
                tostring(name)))
        end
        cur = new_ns_table()
        rawset(parent, name, cur)
        return cur
    end
    if type(cur) ~= "table" then
        error(string.format(
            "zlua slua adaptor: path conflict at %s (expected namespace table)",
            tostring(name)))
    end
    if pending[cur] == nil then
        pending[cur] = {}
        setmetatable(cur, ns_mt)
    end
    return cur
end

local function same_owner(a, assembly_name, type_full_name)
    return a ~= nil and a.asm == assembly_name and a.full == type_full_name
end

local function register_pending(ns_table, leaf, assembly_name, type_full_name)
    local existing = rawget(ns_table, leaf)
    if existing ~= nil then
        if pending[existing] ~= nil then
            error(string.format(
                "zlua slua adaptor: path conflict at %s (namespace vs type %s/%s)",
                tostring(leaf), assembly_name, type_full_name))
        end
        local owners = resolved_owners[ns_table]
        local prev = owners and owners[leaf]
        if prev ~= nil and not same_owner(prev, assembly_name, type_full_name) then
            error(string.format(
                "zlua slua adaptor: path conflict at %s: %s/%s vs %s/%s",
                tostring(leaf), prev.asm, prev.full, assembly_name, type_full_name))
        end
        if owners == nil then
            owners = {}
            resolved_owners[ns_table] = owners
        end
        owners[leaf] = { asm = assembly_name, full = type_full_name }
        return
    end

    local map = pending[ns_table]
    if map == nil then
        map = {}
        pending[ns_table] = map
        setmetatable(ns_table, ns_mt)
    end

    local prev = map[leaf]
    if prev ~= nil then
        if not same_owner(prev, assembly_name, type_full_name) then
            error(string.format(
                "zlua slua adaptor: path conflict at %s: %s/%s vs %s/%s",
                tostring(leaf), prev.asm, prev.full, assembly_name, type_full_name))
        end
        return
    end

    map[leaf] = { asm = assembly_name, full = type_full_name }
end

local function mount_type(root, assembly_name, type_full_name)
    local parts = path_segments(type_full_name)
    if #parts == 0 then
        error(string.format(
            "zlua slua adaptor: empty type name from %s / %s",
            assembly_name, tostring(type_full_name)))
    end

    if #parts == 1 then
        local leaf = parts[1]
        local type_table = resolve_type(assembly_name, type_full_name)
        local existing = rawget(root, leaf)
        if existing ~= nil and existing ~= type_table then
            error(string.format(
                "zlua slua adaptor: global conflict at %q (assembly %s, type %s)",
                leaf, assembly_name, type_full_name))
        end
        rawset(root, leaf, type_table)
        return
    end

    local parent = root
    for i = 1, #parts - 1 do
        parent = ensure_ns_table(parent, parts[i])
    end
    register_pending(parent, parts[#parts], assembly_name, type_full_name)
end

--- Mount export_types under _G (no CS prefix).
--- @param export_types table  assemblyName → typeFullName[]
function M.init(export_types)
    if type(export_types) ~= "table" then
        error("zlua slua adaptor: export_types must be a table")
    end

    for assembly_name, type_list in pairs(export_types) do
        if type(type_list) ~= "table" then
            error(string.format(
                "zlua slua adaptor: export_types[%q] must be an array",
                tostring(assembly_name)))
        end
        for i = 1, #type_list do
            local full = type_list[i]
            if type(full) ~= "string" or full == "" then
                error(string.format(
                    "zlua slua adaptor: invalid type name in %s[%d]",
                    tostring(assembly_name), i))
            end
            mount_type(_G, assembly_name, full)
        end
    end
end

return M
