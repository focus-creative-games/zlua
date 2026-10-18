---
name: zlua-in-ref-out
description: >-
  Marshals C# ref, in, and out parameters with ZLua. Use when a method has
  ref, in, or out arguments, when Lua should receive multiple results from out
  parameters, or when a byref write does not show up in the Lua value.
---

# ref, in, and out

C# keeps its own meanings (`in` is readonly). Lua does not treat the three keywords the same on a Lua-to-C# call.

| Direction | Behavior |
|-----------|----------|
| C# to Lua (`GetFunction`, delegate bridge) | `ref` / `in` / `out` push an OpaqueValue by default |
| Lua to C#, `ref` / `in` | Still consume a Lua argument. Write-back depends on that argument's shape |
| Lua to C#, `out` | Consumes no Lua argument. After the call, values are extra results: `return, out1, out2, ...` |

## out

```lua
-- void GetPosition(out float x, out float y, out float z)
local x, y, z = Demo.GetPosition()

-- bool TryGet(out float x, out float y)
local ok, x, y = Demo.TryGet()

-- bool TryParseScaled(string text, out int value)
local ok, value = Demo.TryParseScaled("21")
```

Do not pass a number as a placeholder for `out`. The full signature key still spells the byref type (`GetPosition(System.Single&,System.Single&,System.Single&)`), but the Lua arity does not count those parameters.

## ref and in write-back

| Lua argument | C# writes are visible in that Lua value |
|--------------|-----------------------------------------|
| OpaqueValue (`zlua.get_opaquevalue` / `set_opaquevalue`) | Yes |
| ByVal userdata whose type equals the element type | Yes, including struct fields |
| Bare number, Lua string, or most ByObj userdata | No. The value is copied into a temporary |

```lua
local x = 5
Demo.Increment(x)          -- ref int: x stays 5

local p = Point2D(1, 2)
Demo.Offset(p)             -- ref Point2D: p's fields update
```

Use `ref` when Lua must observe the write. Use `out` when the method only produces values.

## Opaque from C# to Lua

```lua
local v = zlua.get_opaquevalue(slot)
zlua.set_opaquevalue(slot, newValue)
```

An Opaque handle is valid only for the synchronous call chain. Do not store it across `pcall` or across frames. To keep a userdata, copy with `zlua.to_user_data`. Forcing another marshal shape is `[LuaMarshalAs]` / XML; see `zlua-marshal-as`. `ref` / `in` / `out` already default to Opaque on the C# to Lua path, so do not add `OpaqueValue` unless the slot is by value.
