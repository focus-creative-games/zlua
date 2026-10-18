---
name: zlua-marshal
description: >-
  Describes default ZLua marshaling between Lua values and C# primitives,
  strings, enums, structs, classes, and Nullable. Use when choosing how a value
  crosses the Lua and C# boundary, or when a table or bare number is the wrong shape.
---

# Default marshal

Use these shapes unless `[LuaMarshalAs]` or MarshalAs XML says otherwise. Attribute and XML syntax is `zlua-marshal-as`. `ref` / `in` / `out` write-back is `zlua-in-ref-out`. Arrays are `zlua-array`. Delegates are `zlua-delegate`.

| C# type | Default Lua shape |
|---------|-------------------|
| `bool`, integers, floats, `char` | Lua boolean or number |
| `string` | Lua string (a copy). A huge string can be forced to ByObj with `LuaMarshalType.UserData` |
| `enum` | Integer / number. The type table has no `__call`. Do not write `Color(...)`. Boxed enums use `zlua.box` |
| `struct` | ByVal userdata from `Type(...)` or `Type._default()`. A Lua table is not a struct |
| `class` / interface / array / delegate | ByObj userdata, or `nil` for null |
| `Nullable<T>` | Same as `T` when it has a value; `nil` when it does not |

```lua
local Point2D = CSharp.AC['MyGame.Point2D']
local p = Point2D(3, 4)     -- ByVal userdata
p.X = 10
host:SetPoint(p)            -- by-value parameter copies

local Color = CSharp.AC['MyGame.Color']
host:SetColor(Color.Red)    -- enum constant is an integer
host:SetColor(1)
```

Passing a struct by value copies it. Passing the same ByVal userdata to `ref` / `in` of that struct type writes back into the userdata.

Do not invent `{ X = 1, Y = 2 }` or `Foo(x, y)` as the default struct form. That requires `LuaMarshalType.Table` (one table) or `UnpackedValues` (N stack slots) on the slot or the type. See `zlua-marshal-as`.

`nil` is null only for reference types and `Nullable<T>`. A non-nullable value type rejects `nil`.
