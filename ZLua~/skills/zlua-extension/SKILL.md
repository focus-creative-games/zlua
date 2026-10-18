---
name: zlua-extension
description: >-
  Registers C# extension methods on the extended type so Lua can call
  obj:ExtFoo. Use when writing [LuaExtension], ZLuaExtensions, luaExtensionXmlPaths,
  or when an extension method is missing from a Lua instance.
---

# Extension methods

ZLua does not scan the assembly for `ExtensionAttribute`. Bind looks up a list of extension classes configured on the extended type, then reflects only those classes. An unconfigured extension is `zlua: member not found`, not a static call on the helper class.

| | |
|--|--|
| Attribute | `[LuaExtension(typeof(ExtClass), ...)]` on the extended type |
| XML root | `ZLuaExtensions` |
| Settings | `luaExtensionXmlPaths` |

Do not put `[LuaExtension]` on the static extension class. Do not mix this XML with `ZLuaAlias` or `ZLuaMarshalAs`.

## Attribute

Editable types, including hot-update assemblies, use the attribute only. XML is ignored for hot-update assemblies (including HybridCLR).

```csharp
using ZLua;

public static class MyClassExt
{
    public static void ResetLocal(this MyClass self) { }
}

[LuaExtension(typeof(MyClassExt), typeof(MyClassTweenExt))]
public class MyClass { }
```

```lua
local obj = CSharp.AC.MyClass()
obj:ResetLocal()
```

`AllowMultiple` is allowed. Several attributes on the same type are unioned. Binding `T` also walks `BaseType` and collects `[LuaExtension]` from base types.

An interface extension list is used only when that interface type itself is configured and the method's `this` accepts the type being bound. Implementing an interface does not import extensions configured only on the interface.

## XML

Use XML when the extended type is precompiled, AOT, or third-party, and is not a hot-update assembly. `this` must match `Type/@fullName`.

```xml
<?xml version="1.0" encoding="utf-8"?>
<ZLuaExtensions version="1">
  <Assembly name="UnityEngine.CoreModule">
    <Type fullName="UnityEngine.Transform">
      <Extension assembly="Assembly-CSharp" fullName="MyGame.TransformExt"/>
    </Type>
  </Assembly>
</ZLuaExtensions>
```

`Assembly/@name` is the extended type's assembly short name. `Extension/@assembly` and `@fullName` name the extension class. This file allows only `Assembly` / `Type` / `Extension`.

Attribute and XML lists are unioned. An unresolvable extension class fails Generate, and fails Mono Initialize or Bind. Il2Cpp bakes the list at Generate. The Player does not read XML. A type that is already bound does not pick up an extension assembly loaded later.

## Which methods appear

From each extension class, keep `public static` methods that have `ExtensionAttribute`, have at least one parameter, and whose first parameter can accept the bound type (`this Base` works on a `Derived` instance). Open generic extension methods are not supported. A class that contributes no matching method is allowed.

The method is an instance method on the extended type. It is not added to that type's static table. Lua arity is the CLR parameter count minus `this`. The full signature key lists only the parameters after `this`. `[LuaAlias]` can still rename it (`zlua-alias`).

Structs get the same methods on both ByVal and ByObj instance tables.

## Same name as an instance method

The extension and a real instance method with the same final Lua name are one overload group. There is no instance-first rule. One candidate is a direct closure. Several candidates dispatch. Disambiguate with a full signature key or `[LuaAlias]` (`zlua-method-overload`, `zlua-alias`).
