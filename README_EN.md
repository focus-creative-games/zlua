# ZLua

ZLua is a modern, concise, and easy-to-use Unity Lua scripting solution with extreme Il2Cpp optimization.

- Documentation: [doc.zlua.cn](https://doc.zlua.cn)
- Demo: [zlua-demo](https://github.com/focus-creative-games/zlua-demo)
- Selection guide: [Why ZLua](https://doc.zlua.cn/docs/concepts/why-zlua)

---

## Why ZLua

Compared with xLua / toLua / SLua, ZLua's core claims are:

| | |
|--|--|
| **Easier to use** | More modern design; extremely simple; **zero configuration** (no per-type C# Wrap whitelist) |
| **More complete** | Standard and complete C#↔Lua interop, covering nearly all commonly used C# features |
| **More efficient** | Il2Cpp measured: about **98%** of aligned cases faster than xLua; Lua→C# average about **2.62×**; common field / property / call cases about **4×** |
| **Less GC** | Reference types and structs (including structs with reference fields) are **0 GC** by default; plus OpaqueValue and other strategies |
| **Tiny bridging** | Efficient **C++** stubs with same-signature merging; footprint can be an order of magnitude smaller; supports **0 bridge functions** |
| **Wider version support** | Lua **5.1 / 5.2 / 5.3 / 5.4 / 5.5**; Unity **2021.3 / 2022.3 / 6000.0 / 6000.3**; **Tuanjie Engine** (all releases) |
| **More active maintenance** | Full-time professional team; faster bug response and feature iteration |

For performance details, see [Performance Comparison](https://doc.zlua.cn/docs/compare/PERFORMANCE).

---

## Features

### Interop Model

- **C# → Lua**: `LuaAppDomain.GetFunction<T>("module", "fn")`, then invoke the returned delegate
- **Marshal overrides**: `[LuaMarshalAs]`
- **Lua → C#**: Lazy-loaded types under the `CSharp` root table; syntax close to C# (`Type.Static()` / `obj:Method()` / `obj.field`)
- **Dual backends**: Editor **Mono** and Player **Il2Cpp** share the same Lua-visible semantics

### Complete C# Capability

Including but not limited to: class / struct / interface / enum / nullable, static and instance members, generic classes and generic methods, delegates, arrays, method overloads, `ref` / `out` / `in`, Events (`add_` / `remove_`), and more.

### Il2Cpp Performance and Size

- Embedded Lua with a direct C++ bridge: `methodPointer` / field offset, **no** massive C# Wrap files
- Managed object userdata holds the object pointer directly; GCRoots are maintained on the native side
- **Same-signature bridge functions are merged**; fields / parameterless properties may skip dedicated bridges
- Before shipping, run **`ZLua/Generate/All`** (generates **C++ stubs**, not xLua-style C# Wrap)

### Platforms and Versions

| Category | Supported |
|----------|-----------|
| **Lua** | **5.1.x**, **5.2.x**, **5.3.x**, **5.4.x**, **5.5.x** |
| **Unity** | **2021.3.x**, **2022.3.x**, **6000.0.x**, **6000.3.x** |
| **Tuanjie Engine** | All **x.y.z** releases |
| **Runtime** | Editor **Mono** + Player **Il2Cpp** |
| **Platforms** | Platforms supported by Il2Cpp (including WebGL, WeChat Mini Games, HarmonyOS / automotive, etc.) |

---

## Quick Examples

For a complete runnable project, see [zlua-demo](https://github.com/focus-creative-games/zlua-demo). The three steps below highlight ZLua's strengths: **zero Wrap configuration**, **declarative Lua calls**, and **Lua that feels like C#**.

### 1. Initialization (Loader Only)

```csharp
using System.IO;
using System.Text;
using UnityEngine;
using ZLua;

public static class ZLuaBootstrap
{
    static string LoadLuaModule(string module)
    {
#if UNITY_EDITOR
        var path = Path.Combine(Application.dataPath, "..", "LuaScripts", module + ".lua");
#else
        var path = Path.Combine(Application.streamingAssetsPath, "LuaScripts", module + ".lua.txt");
#endif
        return File.Exists(path) ? File.ReadAllText(path, Encoding.UTF8) : null;
    }

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    static void Init() => LuaAppDomain.Initialize(LoadLuaModule);
}
```

No `LuaCallCSharp` list, and no per-type C# Wrap generation.

### 2. C# Calling Lua (`GetFunction`)

```csharp
public class GameEntry : MonoBehaviour
{
    void Start()
    {
        var add = LuaAppDomain.GetFunction<Func<int, int, int>>("app", "add");
        Debug.Log(add(10, 20)); // 30
    }
}
```

```lua
-- LuaScripts/app.lua
local function add(a, b)
    return a + b
end

return { add = add }
```

### 3. Lua Accessing C# (Lazy Loading, C#-like Syntax)

```csharp
// Any public type; no export configuration required
public class Demo
{
    public int x;
    public static int Add(int a, int b) => a + b;
    public void SetX(int v) => x = v;
}
```

```lua
CSharp['AC'] = CSharp['Assembly-CSharp']

print(CSharp.AC.Demo.Add(3, 5))   -- static method

local d = CSharp.AC.Demo()        -- constructor
d.x = 10                          -- field
d:SetX(20)                        -- instance method
print(d.x)
```

**Player release:** Run the menu **`ZLua/Generate/All`**, and sync Lua scripts to `StreamingAssets` (the Demo project already includes a sync script).

---

## Current Status

| Runtime | Status |
|--------|------|
| **Il2Cpp (Player)** | Complete |
| **Mono (Editor)** | Complete (semantics consistent with Il2Cpp) |

Primary validation environment: Unity **2022.3.62f3** + Lua **5.4**. See “Platforms and Versions” above for the supported matrix. Features and semantics follow the [specification](https://doc.zlua.cn/docs/spec/00-OVERVIEW).

---

## License

MIT. Free to use, modify, and distribute.

## Contact

- GitHub Issues: [focus-creative-games/zlua](https://github.com/focus-creative-games/zlua)
- Email: `zlua@code-philosophy.com`
- QQ group **ZLua**: 824793773
- Discord: https://discord.gg/htmr44jW6A
