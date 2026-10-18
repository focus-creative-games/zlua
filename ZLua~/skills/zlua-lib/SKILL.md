---
name: zlua-lib
description: >-
  Uses the global zlua table: type arguments, typeof, make_generic_type, arrays,
  inflate, box, unbox, cast, opaque, and to_delegate. Use when Lua needs a zlua.*
  helper instead of a CSharp member call.
---

# zlua library

| Layer | Job |
|-------|-----|
| `CSharp` | Assemblies, types, members, constructors. See `zlua-lua-call-csharp` |
| `zlua` | Type-argument helpers, opaque, box, arrays, delegates, `inflate` |
| Instance userdata | Members go through the metatable, not through `zlua` |

`zlua` does not replace `CSharp`. There is no `make_generic_method` or `register_method`.

## Type arguments

Most constructors accept the same type argument: `zlua.types.*` (for example `zlua.types.int32`), a type table, a type-name string, or a `System.Type`.

| API | Result |
|-----|--------|
| `zlua.types.int32` and the other presets | A ready type argument |
| `zlua.typeof(typeTable)` | `System.Type`, same as C# `typeof` |
| `zlua.get_type_from_name(name)` | Type table. Accepts assembly-qualified names, generics, and arrays. A miss is an error |

GTAP prefixes accept only `System.Type`. Everything else in this table is fine for `inflate` and `make_generic_type`. See `zlua-generic-method`.

## Common calls

```lua
local ListInt = zlua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List`1'],
    zlua.types.int32)

local arr = zlua.new_szarray_by_element_type(zlua.types.int32, 4)
local t = zlua.to_table(arr)
local bytes = zlua.to_bytes(floatArr)   -- blittable elements; not bool[] or char[]

local GetRb = zlua.inflate(GameObject, "GetComponent", Rigidbody)
local rb = GetRb(go)

local boxed = zlua.box(Color, Color.Red)
local n = zlua.unbox(boxed)
local viewed = zlua.cast(obj, targetType)   -- same object, different facade

local v = zlua.get_opaquevalue(slot)
zlua.set_opaquevalue(slot, newValue)
local copy = zlua.to_user_data(slot)

local del = zlua.to_delegate(fn, closedDelegateType)
```

Deeper rules live in the matching skill: arrays in `zlua-array`, generic methods in `zlua-generic-method`, opaque and byref in `zlua-in-ref-out`, delegates in `zlua-delegate`, signature keys in `zlua-method-overload`.

The native function `__zlua_create_signature(...)` returns a parameter-list string such as `"(System.Int32)"`. It is not wrapped on the `zlua` table yet, and that string alone is not a metatable key. The key is still `Name(Types...)`.
