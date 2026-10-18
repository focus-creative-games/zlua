---
name: zlua-lua-call-csharp
description: >-
  Calls C# from Lua through the CSharp root: assemblies, types, constructors,
  static and instance members, fields, properties, and events. Use when writing
  Lua that touches CSharp, constructs a C# object, or reads and writes C# members.
---

# Lua calls C#

Access public types through the global `CSharp` table. Types bind lazily on first access. A missing member is an error (`zlua: member not found`), not `nil`. There is no runtime walk of the inheritance chain; base members are flattened onto the type at bind time.

## Types

```lua
CSharp['AC'] = CSharp['Assembly-CSharp']

local Demo = CSharp.AC.Demo
local Panel = CSharp.AC['MyGame.UI.Panel']   -- dotted names need a string key
local Nested = CSharp.AC['Outer+Nested']      -- nested types use +
local Int32 = CSharp.mscorlib['System.Int32']
```

Do not write `CSharp.AC.MyGame.UI.Panel`. Only public members are visible. Static and instance metadata are separate tables.

## Members

```lua
CSharp.AC.Demo.Add(3, 5)    -- static method
CSharp.AC.Demo.s_x = 10     -- static field or parameterless property

local demo = CSharp.AC.Demo()   -- constructor
demo:SetX(10)                   -- instance method; colon passes self
demo.x = 20                     -- public field or parameterless property
print(demo.x)
```

| Syntax | Meaning |
|--------|---------|
| `Type.Add(a, b)` | Static method |
| `Type.Field` / `Type.Prop` | Static field or parameterless property |
| `obj:Method()` | Instance method |
| `obj.field` | Instance field or parameterless property |

Indexers (`this[int]`) dispatch as methods. Do not write `obj[i]` for them. Arrays are the exception; see `zlua-array`.

`nil` maps to a reference-type `null`, and the reverse. A non-nullable struct cannot be `nil`. Subclass instances can call public instance members declared on a base type. Static members are reached through the declaring type's table.

## Events

There is no event metatable and no `.get` / `.set` sugar.

```lua
host:add_OnHealthChanged(onChanged)
host:remove_OnHealthChanged(onChanged)  -- same function reference
```

Static events use `Type.add_Foo(handler)`. The handler is a Lua function. If code expects `OnX.get`, replace that xLua-style sugar with `add_` / `remove_`.

## Related skills

Overloads, default arguments, and signature keys: `zlua-method-overload`. `out` / `ref` / `in`: `zlua-in-ref-out`. Passing structs and marshal attributes: `zlua-marshal`.
