---
name: zlua-delegate
description: >-
  Marshals Lua functions and C# delegates: implicit Action and Func parameters,
  calling a callback from Lua, zlua.to_delegate, and add_/remove_ events.
  Use when a C# API takes or returns a delegate, Action, Func, or event handler.
---

# Delegates

Three paths. Do not collapse them into one.

| Path | How |
|------|-----|
| Lua function passed into a C# parameter | Pass `function(...) end`. ZLua builds a closed delegate from the parameter type |
| C# callback handed to Lua | Call it: `handler(...)`. It may be a Lua function or delegate userdata |
| C# calls a named Lua export | `LuaAppDomain.GetFunction<T>(module, method)`. See `zlua-csharp-call-lua` |

## Lua function as a parameter

```csharp
public void RegisterCallback(System.Action<int> onValue)
{
    onValue?.Invoke(42);
}
```

```lua
host:RegisterCallback(function(v)
    print("callback:", v)
end)

logic.Combine = function(a, b) return a + b end
```

| Lua argument | Result |
|--------------|--------|
| `function` | New closed delegate of the parameter type |
| `nil` | `null` |
| Existing delegate userdata | Passed through |
| Anything else | Type error |

Do not call `zlua.to_delegate` for this case. Use `zlua.to_delegate(fn, closedDelegateType)` only when Lua already has a function and the delegate type is not fixed by a C# parameter. The type argument must be a closed type such as `Action<int>`, not an open generic.

## Callbacks returned to Lua

```lua
local handler = host:GetHandler()
handler(42)
```

| Origin | `type(handler)` | Call |
|--------|-----------------|------|
| C# delegate | userdata with `__call` | `handler(...)` |
| Lua function that round-tripped through C# | `function` | `handler(...)` |

Do not use `handler:Invoke(...)`. That fails on a raw Lua function. An open delegate (`target == null`) is not supported.

`params` on a delegate bridge or on `GetFunction` is not supported. See `zlua-params`.

Events use `add_` / `remove_` with the same function reference. See `zlua-lua-call-csharp`.
