---
name: zlua-alias
description: >-
  Renames a C# method's Lua key with [LuaAlias] or ZLuaAlias XML. Use when
  the user mentions LuaAlias, ZLuaAlias, luaAliasXmlPaths, a short colon name,
  or a method that should no longer be called by its C# name.
---

# LuaAlias

`[LuaAlias]` replaces the Lua key. It does not add a second name. After an alias, that method is not registered under `MethodInfo.Name`. Full signature keys still use the CLR name (`demo['Run(System.Int32)']`). See `zlua-method-overload`.

| | |
|--|--|
| Attribute | `[LuaAlias("run_i32")]` on the method only. One alias. Not inherited by overrides |
| XML root | `ZLuaAlias` |
| Settings | `luaAliasXmlPaths` |

Do not mix this file with `ZLuaMarshalAs` or `ZLuaExtensions`.

Hot-update assemblies (including HybridCLR) use the attribute only. XML is ignored. Precompiled, AOT, or third-party assemblies that are not hot-update use XML. Il2Cpp bakes XML at Generate. The Player does not read XML. Mono reads the Settings path list at runtime.

## Attribute

```csharp
using ZLua;

[LuaAlias("run_i32")]
public void Run(int value) { }

public void Run(string value) { }
```

```lua
demo:run_i32(10)   -- Run(int). "Run" no longer selects this overload
demo:Run("hi")     -- only overloads that were not renamed
```

The same alias on two methods, or an alias equal to another method's default name, joins one overload group. Callers then dispatch. Static names go on the type table (`Demo.add_i32(...)`). Instance names use a colon. Groups do not cross static and instance, or ByVal and ByObj.

If the final name matches a field or a parameterless property, method lookup still wins.

## XML

`Assembly/@name` is `Assembly.GetName().Name` (for example `Assembly-CSharp`), not a path or `.dll`. `Type/@fullName` is the CLR name. Nested types use `Outer+Inner`. An open generic container is ``Foo`1``. `Method/@name` is `MethodInfo.Name`. `Method/@signature` is parameters only, in parentheses, with no return type: `()`, `(System.Int32)`, `(System.Int32,System.String)`. Byref types end with `&`. `@alias` is required and is the only Lua name.

```xml
<?xml version="1.0" encoding="utf-8"?>
<ZLuaAlias version="1">
  <Assembly name="Assembly-CSharp">
    <Type fullName="Demo">
      <Method name="Run" signature="(System.Int32)" alias="run_i32"/>
    </Type>
  </Assembly>
</ZLuaAlias>
```

`version` must be `"1"`. This file allows only `Assembly` / `Type` / `Method` with `@alias`. No `MarshalAs`, `Param`, `Return`, `Field`, or `Property`.

On the same method, the attribute wins and the XML entry is unused. Two `@alias` values for the same `(assembly, type, method, signature)` fail. Two different methods may share one final name.
