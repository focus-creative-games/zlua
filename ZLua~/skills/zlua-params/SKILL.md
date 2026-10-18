---
name: zlua-params
description: >-
  Calls C# params T[] from Lua by expanding arguments or passing one T[] userdata.
  Use when a method or constructor ends with params, or when a Lua table or nil
  was incorrectly passed as the whole array.
---

# params T[]

A trailing `params T[]` expands, matching C#. A normal `T[]` parameter does not expand. There is no `LuaMarshalType` for params. The old `ParamsTable` name is gone. Creating a real array is `zlua-array`.

| | Normal `T[]` | `params T[]` |
|--|--------------|--------------|
| `Foo(1, 2, 3)` | Not accepted | Expanded to elements of `T` |
| Omit the array | Depends on optional / null | Empty `T[0]`, not `null` |
| One `T[]` userdata | Accepted | Accepted when that single argument is assignable to `T[]` |
| Lua table as the whole array | Array-shaped table only where normal arrays allow it | Rejected |
| Lone `nil` as a null array | Can be a null reference | Rejected. `nil` is one expanded element |

```csharp
static void Sum(params int[] values) { }
static void Prefix(int head, params int[] tail) { }
static void CountNulls(params string[] values) { }
```

```lua
Demo.Sum(1, 2, 3)
Demo.Sum()                    -- int[0]
Demo.Prefix(0, 1, 2)          -- head = 0, tail = { 1, 2 }
Demo.Sum(arr)                 -- arr is an int[] userdata
Demo.CountNulls(nil)          -- string[] { null }
Demo.CountNulls("a", nil, "b")
```

Do not write `Demo.Sum({ 1, 2, 3 })` or `Demo.Sum(nil)` for a null `int[]`. Build an array, then pass it:

```lua
local arr = zlua.new_szarray_by_element_type(zlua.types.int32, 3)
arr[0], arr[1], arr[2] = 1, 2, 3
Demo.Sum(arr)
```

Overload ranking, after equal conversion scores: a whole userdata array beats expansion; a candidate without `params` beats one with `params`. `params` may take any argument count from `minLuaArity` upward. Constructors follow the same rules.

`params` is not supported on `GetFunction` delegates or on C# to Lua delegate bridges. See `zlua-delegate`.
