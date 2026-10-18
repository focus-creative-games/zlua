---
name: zlua-array
description: >-
  Creates and indexes C# arrays from Lua with zlua szarray and mdarray helpers,
  to_table, and to_bytes. Use when Lua needs a T[] or a multidimensional array,
  or when converting an array to a Lua table or a byte string.
---

# Arrays

C# arrays are ByObj userdata. Indexes are 0-based, matching C#. `List<T>` is a generic type, not an array; see `zlua-generic-method`. `params T[]` is not this API; see `zlua-params`.

## Szarray (`T[]`)

```lua
local arr = zlua.new_szarray_by_element_type(zlua.types.int32, 4)

local IntArray = zlua.make_szarray_type(zlua.types.int32)
local arr2 = zlua.new_szarray_by_szarray_type(IntArray, 4)

arr[0] = 10
arr[1] = 20
print(#arr)   -- Length
```

Reference elements may be `nil`. An index outside `0 .. Length-1` is a C# bounds error.

Element type arguments accept `zlua.types.*`, a type table, a type-name string, or a `System.Type` from `zlua.typeof`. See `zlua-lib`.

## Table and bytes

```lua
local t = zlua.to_table(arr)   -- Lua table is 1-based; the C# array stays 0-based
local bytes = zlua.to_bytes(floatArr)
```

`to_bytes` copies raw memory. The element type must be blittable (primitive, or a struct with no reference fields). It does not accept `bool[]` or `char[]`.

## Multidimensional arrays

```lua
local IntMatrix = zlua.make_mdarray_type(zlua.types.int32, 2)
local matrix = zlua.new_mdarray_by_mdarray_type(
    IntMatrix,
    {0, 0},    -- lowbounds
    {3, 4}     -- sizes
)
```

`#` does not work on an mdarray. Use `GetLength(dimension)`.

A Lua table is not automatically a `T[]` argument, except where an array-shaped table is explicitly accepted for a normal `T[]` parameter. `params T[]` rejects that table. See `zlua-params`.
