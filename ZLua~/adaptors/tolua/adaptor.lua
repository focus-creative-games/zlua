-- ZLua toLua adaptor: Type.Name → _G[shortName] → CSharp[assembly][typeFullName]
-- Spec: docs/spec/12-MIGRATION-ADAPTORS.md
-- Short name = last CLR name segment (after '.' / '+'), e.g. UnityEngine.GameObject → GameObject.

local M = {}

local function resolve_type(assembly_name, type_full_name)
    local asm = CSharp[assembly_name]
    if asm == nil then
        error(string.format(
            "zlua tolua adaptor: assembly not found: %s (type %s)",
            tostring(assembly_name), tostring(type_full_name)))
    end
    local t = asm[type_full_name]
    if t == nil then
        error(string.format(
            "zlua tolua adaptor: type not found: CSharp[%q][%q]",
            assembly_name, type_full_name))
    end
    return t
end

local function short_name(type_full_name)
    local normalized = (type_full_name or ""):gsub("%+", ".")
    local name = normalized:match("([^%.]+)$")
    return name
end

--- Mount export_types as global short names on _G.
--- @param export_types table  assemblyName → typeFullName[]
function M.init(export_types)
    if type(export_types) ~= "table" then
        error("zlua tolua adaptor: export_types must be a table")
    end

    -- shortName → "assembly\\0fullName" for conflict messages
    local owners = {}

    for assembly_name, type_list in pairs(export_types) do
        if type(type_list) ~= "table" then
            error(string.format(
                "zlua tolua adaptor: export_types[%q] must be an array",
                tostring(assembly_name)))
        end
        for i = 1, #type_list do
            local full = type_list[i]
            if type(full) ~= "string" or full == "" then
                error(string.format(
                    "zlua tolua adaptor: invalid type name in %s[%d]",
                    tostring(assembly_name), i))
            end

            local name = short_name(full)
            if name == nil or name == "" then
                error(string.format(
                    "zlua tolua adaptor: cannot derive short name from %s",
                    full))
            end

            local type_table = resolve_type(assembly_name, full)
            local owner_key = assembly_name .. "\0" .. full
            local prev_owner = owners[name]
            if prev_owner ~= nil and prev_owner ~= owner_key then
                local prev_asm, prev_full = prev_owner:match("^(.-)\0(.*)$")
                error(string.format(
                    "zlua tolua adaptor: short name conflict %q: %s/%s vs %s/%s",
                    name, tostring(prev_asm), tostring(prev_full),
                    assembly_name, full))
            end

            local existing = rawget(_G, name)
            if existing ~= nil and existing ~= type_table then
                error(string.format(
                    "zlua tolua adaptor: global conflict at %q (assembly %s, type %s)",
                    name, assembly_name, full))
            end

            rawset(_G, name, type_table)
            owners[name] = owner_key
        end
    end
end

return M
