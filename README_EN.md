# ZLua

ZLua is a modern, concise, and easy-to-use Unity Lua scripting solution with extreme Il2Cpp optimization.

- Documentation: [doc.zlua.cn](https://doc.zlua.cn)
- Demo: [zlua-demo](https://github.com/focus-creative-games/zlua-demo)
- Selection guide: [Why ZLua](https://doc.zlua.cn/docs/concepts/why-zlua)

---

## Why ZLua

Compared with xLua / toLua / SLua, ZLua's core strengths are:

| | |
|--|--|
| **Easier to use** | More modern design; extremely simple; **zero configuration** (no per-type C# Wrap whitelist) |
| **More complete** | Standard and complete C#↔Lua interop, covering nearly all commonly used C# features |
| **More efficient** | Il2Cpp measured: about **98%** of test cases faster than xLua; Lua→C# average about **2.62×**; common field / property / call cases about **4×** |
| **Less GC** | Reference types and structs (including structs with reference fields) are **0 GC** by default; plus OpaqueValue and other strategies |
| **Tiny bridging** | Efficient **C++** stubs with same-signature merging; generated bridge size is about an order of magnitude smaller than xLua-style wrappers translated to C++; supports **0 bridge functions**, and even without generating any bridges, interop is still faster than other solutions' wrapper paths in most cases |
| **Wider version support** | Lua **5.1–5.5** and **LuaJIT 2.x**; Unity **2021.3 / 2022.3 / 6000.0 / 6000.3 / 6000.5**; **Tuanjie Engine** (all releases) |
| **More active maintenance** | Full-time professional team; faster bug response and feature iteration |

For performance details, see [Performance Comparison](https://doc.zlua.cn/docs/compare/PERFORMANCE).

---

## Features

- Extremely easy to use: **zero configuration**, works out of the box. C# and Lua can call each other freely by default—no tedious wrapper lists or complex export setup
- Unified, complete C#↔Lua interop: access nearly all C# features from Lua with a native, C#-like feel
- Highly optimized for Il2Cpp and Lua: about **98%** of test cases faster than xLua; Lua→C# average about **2.62×**; common field / property / call cases about **4×**
- Less GC and stronger, more consistent marshaling: **0 GC** for arbitrary objects, including structs with reference fields that other Lua solutions often cannot pass that way
- Tiny bridge functions: merge same-signature bridges and emit C++ directly; even a full generate is about an order of magnitude smaller than other solutions' wrappers translated to C++. Supports **0** bridge functions—and even then, interop is still faster than other solutions' wrapper paths in most cases
- Built-in **EmmyLua** debugging support
- Dual runtime: **Mono** in development and **Il2Cpp** at ship time—balancing editor productivity and player performance

### Platforms and Versions

| Category | Supported |
|----------|-----------|
| **Lua (PUC-Rio)** | **5.1.x**, **5.2.x**, **5.3.x**, **5.4.x**, **5.5.x** |
| **LuaJIT** | **2.x** (Editor Mono; Il2Cpp shipping **iOS / Android only**) |
| **Unity** | **2021.3.x**, **2022.3.x**, **6000.0.x**, **6000.3.x**, **6000.5.x** |
| **Tuanjie Engine** | All **x.y.z** releases |
| **Runtime** | Editor **Mono** + Player **Il2Cpp** |
| **Platforms (PUC-Rio)** | Platforms supported by Il2Cpp (including WebGL, WeChat Mini Games, HarmonyOS / automotive, etc.) |
| **Platforms (LuaJIT)** | Editor; Player **iOS / Android only** (bring your own static library; see docs) |

---

## Quick Examples

For a complete runnable project, see [zlua-demo](https://github.com/focus-creative-games/zlua-demo).

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

---

## License

MIT. Free to use, modify, and distribute.

## Contact

- GitHub Issues: [focus-creative-games/zlua](https://github.com/focus-creative-games/zlua)
- Email: `zlua@code-philosophy.com`
- QQ group **ZLua**: 824793773
- Discord: <https://discord.gg/htmr44jW6A>
