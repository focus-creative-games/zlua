---
name: zlua-generic-method
description: >-
  Closes C# generic types and generic methods from Lua with make_generic_type,
  zlua.inflate, and GTAP typeof prefixes. Use when Lua calls List<T>, Dictionary,
  GetComponent<T>, or any method that still has its own type parameters.
---

# Generic types and generic methods

A closed generic type is an ordinary type table. A method that still has its own type parameters is not an overload; close it with `zlua.inflate` or a GTAP prefix. There is no `make_generic_method` or `register_method`.

## Closed generic types

The open definition name includes the arity backtick: ``List`1``, ``Dictionary`2``.

```lua
local ListInt = zlua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List`1'],
    zlua.types.int32)

local list = ListInt()
list:Add(10)

local DictStrInt = zlua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.Dictionary`2'],
    zlua.types.string,
    zlua.types.int32)
```

The first argument is the open type table. The rest are type arguments (`zlua.types.*`, a type table, a type name, or `System.Type`). `List<int>.Add` is a normal method. Do not inflate it.

If `List` is missing, the key must be the full name ``System.Collections.Generic.List`1`` in brackets. The number of type arguments must match the arity.

## Generic methods

`zlua.inflate` closes only the method's type parameters. If the declaring type is still open, call `make_generic_type` first. Cache the closure on a hot path.

GTAP puts N `System.Type` values in front of a short-name call. Those prefixes must be `zlua.typeof(...)`. A type table, a string, or `zlua.types.*` is not a GTAP prefix. `inflate` accepts the full type-argument set.

```lua
local Rigidbody = CSharp['UnityEngine.CoreModule']['UnityEngine.Rigidbody']

local GetRb = zlua.inflate(GameObject, "GetComponent", Rigidbody)
local rb = GetRb(go)

local rb2 = go:GetComponent(zlua.typeof(Rigidbody))
```

When several short names match, pass a full signature key as the second argument to `inflate`. See `zlua-method-overload`.

The return value uses the declared type's facade. To view it as the runtime type, use `zlua.cast` or `ActualTypeUserData` (`zlua-marshal-as`).

| Mistake | Fix |
|---------|-----|
| Wrong arity | Match `` `1 `` / `` `2 `` to the argument count |
| `inflate` on `List<int>.Add` | Inflate only methods that declare their own type parameters |
| GTAP given a type table | Wrap it with `zlua.typeof`, or use `inflate` |
