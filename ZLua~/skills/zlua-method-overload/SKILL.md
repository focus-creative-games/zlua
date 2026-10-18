---
name: zlua-method-overload
description: >-
  Resolves C# method overloads from Lua: default dispatch, full signature keys,
  and when to use LuaAlias. Use when several C# methods share a name or a call
  is ambiguous. Optional parameters are zlua-default-arguments.
---

# Method overloads

Lua has no static types. ZLua picks an overload at the call. One public candidate is a direct closure. Two or more candidates are a dispatch closure. There is no API to attach a short name after bind; short names come from `[LuaAlias]` or alias XML (`zlua-alias`).

Open generic methods such as `GetComponent<T>` are not overloads. See `zlua-generic-method`.

## How to call

| Need | Write |
|------|--------|
| Arguments pick one overload | `demo:Run(10)` or `demo:Run("hi")` |
| Name one overload exactly | `demo['Run(System.Int32)'](demo, 5)` |
| Hot path, short colon call | `[LuaAlias]` / `ZLuaAlias`, then `demo:run_i32(5)` |
| Same overload in a loop | `local run = demo['Run(System.Int32)']; run(demo, 5)` |

The signature key is `MethodName(ParamFullName,...)` with no return type. The name is `MethodInfo.Name`, not the alias. Byref parameters keep `&` (`System.Single&`). The first access is a miss: ZLua resolves it, caches a direct closure, and returns it. Constructors are not signature keys; they stay on `Type(...)`.

A signature key does not remove the short name. `Run` can still dispatch while `Run(System.Int32)` is fixed.

Trailing optional parameters, including the "fewer defaults wins" tie-break, are `zlua-default-arguments`. `params T[]` is `zlua-params`.
