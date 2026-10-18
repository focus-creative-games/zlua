---
name: zlua-default-arguments
description: >-
  Omits trailing C# optional parameters from Lua and fills them from the
  cached default values. Use when a method or constructor has parameters with
  default values, when Lua passes fewer arguments than the C# signature, or
  when an overload is chosen because it uses fewer defaults.
---

# Default arguments

A C# method or constructor may omit a trailing run of parameters that have CLR `HasDefault`. Invoke writes the values cached at bind. The hot path does not read the metadata blob again.

```csharp
public static int Foo(int x, int y = 5) => x + y;
public static string Bar(string s, int a = 1, int b = 2) => $"{s}:{a},{b}";
```

```lua
Demo.Foo(1)          -- 6; y is 5
Demo.Foo(1, 9)       -- 10
Demo.Bar("hi")       -- "hi:1,2"
Demo.Bar("hi", 7)    -- "hi:7,2"
```

| Rule | Detail |
|------|--------|
| Trailing only | Same as C#. `void M(int x = 1, int y)` is not valid C#, and Lua cannot skip a required parameter in the middle |
| Arity | `minLuaArity ≤ argCount ≤ luaArity`. `minLuaArity` is the full Lua slot count minus the slots of the omitted tail |
| Too few | If the call does not cover the first parameter without a default, there is no match (`argument mismatch` or no overload) |
| `out` | An `out` parameter already takes 0 Lua slots. It is not a default argument. See `zlua-in-ref-out` |
| Constructors | `Type(...)` uses the same rule |

```lua
local p = Point(1)     -- Point(int x, int y = 0)
local q = Point()      -- only if every parameter has a default
```

`Type._default()` is zero-initialization of a struct. It does not call a constructor and does not apply parameter defaults.

## Overloads

When conversion scores tie, the candidate that uses fewer default arguments wins.

```csharp
public static int G(int x) => x;
public static int G(int x, int y = 5) => x + y;
```

```lua
Demo.G(1)      -- G(int); optionalUsed is 0
Demo.G(1, 4)   -- G(int, int)
```

Further ties, after that count: a whole `T[]` userdata beats `params` expansion, then a candidate without `params` beats one with `params`. See `zlua-params` and `zlua-method-overload`.
