-- ZLua migration adaptor (shared by xLua / toLua / SLua).
-- Spec: docs/spec/12-MIGRATION-ADAPTORS.md
--
-- export_types shape:
--   {
--     top_namespace = "CS",  -- optional; nil/"" → _G
--     types = {
--       ["Asm"] = {
--         { full_name = "Ns.Type" },  -- omit export_name when same as full_name
--         { full_name = "Ns.Outer+Inner", export_name = "Ns.Outer.Inner" },
--       },
--     },
--   }
-- export_name defaults to full_name when omitted.
-- export_name without '.' → eager rawset on root.
-- export_name with '.' → namespace tables + lazy __index.

local M = {}

local pending = {}
local resolved_owners = {}

local function resolve_type(assembly_name, type_full_name)
    local asm = CSharp[assembly_name]
    if asm == nil then
        error(string.format(
            "zlua adaptor: assembly not found: %s (type %s)",
            tostring(assembly_name), tostring(type_full_name)))
    end
    local t = asm[type_full_name]
    if t == nil then
        error(string.format(
            "zlua adaptor: type not found: CSharp[%q][%q]",
            assembly_name, type_full_name))
    end
    return t
end

local function path_segments(export_name)
    local parts = {}
    for part in (export_name or ""):gmatch("[^.]+") do
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
                "zlua adaptor: path conflict at %s (type leaf vs namespace)",
                tostring(name)))
        end
        cur = new_ns_table()
        rawset(parent, name, cur)
        return cur
    end
    if type(cur) ~= "table" then
        error(string.format(
            "zlua adaptor: path conflict at %s (expected namespace table)",
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
                "zlua adaptor: path conflict at %s (namespace vs type %s/%s)",
                tostring(leaf), assembly_name, type_full_name))
        end
        local owners = resolved_owners[ns_table]
        local prev = owners and owners[leaf]
        if prev ~= nil and not same_owner(prev, assembly_name, type_full_name) then
            error(string.format(
                "zlua adaptor: path conflict at %s: %s/%s vs %s/%s",
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
                "zlua adaptor: path conflict at %s: %s/%s vs %s/%s",
                tostring(leaf), prev.asm, prev.full, assembly_name, type_full_name))
        end
        return
    end

    map[leaf] = { asm = assembly_name, full = type_full_name }
end

local function mount_entry(root, assembly_name, full_name, export_name)
    local parts = path_segments(export_name)
    if #parts == 0 then
        error(string.format(
            "zlua adaptor: empty export_name for %s / %s",
            assembly_name, tostring(full_name)))
    end

    if #parts == 1 then
        local leaf = parts[1]
        local type_table = resolve_type(assembly_name, full_name)
        local existing = rawget(root, leaf)
        if existing ~= nil and existing ~= type_table then
            error(string.format(
                "zlua adaptor: path conflict at %s (assembly %s, type %s)",
                leaf, assembly_name, full_name))
        end
        rawset(root, leaf, type_table)
        return
    end

    local parent = root
    for i = 1, #parts - 1 do
        parent = ensure_ns_table(parent, parts[i])
    end
    register_pending(parent, parts[#parts], assembly_name, full_name)
end

local function ensure_root(top_namespace)
    if top_namespace == nil or top_namespace == "" then
        return _G
    end
    if type(top_namespace) ~= "string" then
        error("zlua adaptor: top_namespace must be a string or nil")
    end
    if top_namespace:find(".", 1, true) then
        error("zlua adaptor: top_namespace must be a single segment (got "
            .. top_namespace .. ")")
    end

    local root = rawget(_G, top_namespace)
    if root == nil then
        root = {}
        rawset(_G, top_namespace, root)
        return root
    end
    if type(root) ~= "table" then
        error(string.format(
            "zlua adaptor: global %q exists but is not a table",
            top_namespace))
    end
    return root
end

--- @param export_types table
function M.init(export_types)
    if type(export_types) ~= "table" then
        error("zlua adaptor: export_types must be a table")
    end

    local types = export_types.types
    if type(types) ~= "table" then
        error("zlua adaptor: export_types.types must be a table")
    end

    local root = ensure_root(export_types.top_namespace)

    for assembly_name, entries in pairs(types) do
        if type(entries) ~= "table" then
            error(string.format(
                "zlua adaptor: types[%q] must be an array",
                tostring(assembly_name)))
        end
        for i = 1, #entries do
            local entry = entries[i]
            if type(entry) ~= "table" then
                error(string.format(
                    "zlua adaptor: types[%s][%d] must be a table",
                    tostring(assembly_name), i))
            end
            local full_name = entry.full_name
            if type(full_name) ~= "string" or full_name == "" then
                error(string.format(
                    "zlua adaptor: invalid full_name in types[%s][%d]",
                    tostring(assembly_name), i))
            end
            local export_name = entry.export_name
            if export_name == nil then
                export_name = full_name
            elseif type(export_name) ~= "string" or export_name == "" then
                error(string.format(
                    "zlua adaptor: invalid export_name in types[%s][%d]",
                    tostring(assembly_name), i))
            end
            mount_entry(root, assembly_name, full_name, export_name)
        end
    end
end

return M
