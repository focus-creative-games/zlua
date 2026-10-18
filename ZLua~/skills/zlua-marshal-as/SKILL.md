---
name: zlua-marshal-as
description: >-
  Overrides default ZLua marshaling with [LuaMarshalAs] or ZLuaMarshalAs XML.
  Use when the user mentions LuaMarshalAs, ZLuaMarshalAs, marshalAsXmlPaths,
  Bytes, Table, UnpackedValues, OpaqueValue, UserData, or ActualTypeUserData.
---

# LuaMarshalAs

Overrides the default marshal on a parameter, return, field, property, or type (class or struct). Never put it on a method. Do not put it on a slot whose type still has an unbound generic parameter. Default shapes are `zlua-marshal`.

| | |
|--|--|
| Attribute | `[LuaMarshalAs(...)]`, or `[return: LuaMarshalAs(...)]` |
| XML root | `ZLuaMarshalAs` |
| Settings | `marshalAsXmlPaths` |

Do not mix this file with `ZLuaAlias` or `ZLuaExtensions`.

Hot-update assemblies (including HybridCLR) use the attribute only. XML is ignored. Precompiled, AOT, or third-party assemblies that are not hot-update use XML. Il2Cpp bakes XML at Generate. The Player does not read XML. Mono reads the Settings path list at runtime.

## Types

```csharp
using ZLua;

public void Send([LuaMarshalAs(LuaMarshalType.Bytes)] byte[] payload) { }

public void Move(
    [LuaMarshalAs(LuaMarshalType.UnpackedValues, Members = new[] { "X", "Y" })]
    Vec2 delta) { }

[return: LuaMarshalAs(LuaMarshalType.UnpackedValues, Members = new[] { "X", "Y" })]
public Vec2 Origin() => default;

public void Submit(
    [LuaMarshalAs(LuaMarshalType.Table, Members = new[] { "Id", "X", "Y", "Tag?" })]
    Packet p) { }
```

| `LuaMarshalType` | Use |
|------------------|-----|
| `Default` | No override |
| `Bytes` | `byte[]` or `string` as a Lua string of raw bytes |
| `OpaqueValue` | C# to Lua only. `ref` / `in` / `out` already default to this. See `zlua-in-ref-out` |
| `UnpackedValues` | Struct, not Nullable or class. N stack slots. `Members` required |
| `Table` | Struct or `Nullable<struct>`, not class. One table. `Members` required. A name ending in `?` may be omitted. `nil` is an empty `Nullable<struct>` |
| `UserData` | Almost only `string`, to keep a ByObj userdata instead of copying a huge Lua string |
| `ActualTypeUserData` | Return only. ByObj facade uses the runtime type |

```lua
host:Send("\0\1\2")
m:Move(3.0, 4.0)
local x, y = m:Origin()
host:Submit({ Id = 1, X = 2, Y = 3 })
```

A type-level attribute applies to every default-marshal slot of that type. Do not put a type-level rule on an open generic such as `MyList<T>`. `params T[]` is not a marshal type. See `zlua-params`.

## XML

`Param/@index` is 0-based and excludes `this`. Do not select parameters by name. `MarshalAs/@type` uses the names above (`OpaqueValue`, not older names). `members` is comma-separated. Unknown `@type` fails. Do not put `MarshalAs` directly under `Method`.

```xml
<?xml version="1.0" encoding="utf-8"?>
<ZLuaMarshalAs version="1">
  <Assembly name="UnityEngine.CoreModule">
    <Type fullName="UnityEngine.Vector3">
      <MarshalAs type="Table" members="x,y,z"/>
    </Type>
    <Type fullName="UnityEngine.Transform">
      <Method name="LookAt" signature="(UnityEngine.Vector3)">
        <Param index="0">
          <MarshalAs type="UnpackedValues" members="x,y,z"/>
        </Param>
      </Method>
      <Method name="get_position" signature="()">
        <Return>
          <MarshalAs type="Table" members="x,y,z"/>
        </Return>
      </Method>
    </Type>
    <Type fullName="MyGame.Net.Packet">
      <Field name="Body">
        <MarshalAs type="Bytes"/>
      </Field>
      <Property name="Title">
        <MarshalAs type="UserData"/>
      </Property>
    </Type>
  </Assembly>
</ZLuaMarshalAs>
```

`Assembly/@name` is `Assembly.GetName().Name`. `Type/@fullName` is the CLR name. Nested types use `Outer+Inner`. Method `@signature` is parameters only, with `&` for byref. A generic declaration used as a container is the open name (``MyList`1``), and only for Field, Param, or Return rules. Do not write a closed name such as ``List`1[System.Int32]`` as that container.

Slot order: parameter, return, field, or property attribute, then XML, then the type-level attribute, then the built-in default. The attribute wins over XML. A wrong type or direction falls back to `Default` and logs in the Editor. Missing `Members` for Table or UnpackedValues fails at bind or Generate.
