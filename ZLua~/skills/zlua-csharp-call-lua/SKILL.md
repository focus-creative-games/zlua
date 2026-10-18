---
name: zlua-csharp-call-lua
description: >-
  Calls Lua from C# with LuaAppDomain.Initialize and GetFunction. Use when
  writing a module loader, binding Action or Func to a Lua module method,
  or reloading scripts with Reset.
---

# C# calls Lua

Editor and Player use the same API. Initialize once, then bind delegates by module name and method name.

## Initialize

```csharp
using System.IO;
using System.Text;
using UnityEngine;
using ZLua;

public class Bootstrap : MonoBehaviour
{
    private static string LoadLuaModule(string module)
    {
#if UNITY_EDITOR
        string path = Path.Combine(Application.dataPath, "..", "LuaScripts", module + ".lua");
#else
        string path = Path.Combine(
            Application.streamingAssetsPath, "LuaScripts", module + ".lua.txt");
#endif
        return File.Exists(path) ? File.ReadAllText(path, Encoding.UTF8) : null;
    }

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    private static void InitZLuaOnStartup()
    {
        LuaAppDomain.Initialize(LoadLuaModule);
    }
}
```

`module` is a logical name with no path and no extension (`"app"`, or `"battle.ai"` which the loader maps with `module.Replace('.', '/')`). Return UTF-8 source, or `null` if the module is missing. Do not create a `LuaState` or register a wrap by hand.

## GetFunction

The Lua module must `return` a table. Keys are the method names passed to `GetFunction`.

```lua
local function add(a, b)
    return a + b
end

return {
    main = function() print("lua main start") end,
    add = add,
}
```

```csharp
Action appMain = LuaAppDomain.GetFunction<Action>("app", "main");
Func<int, int, int> appAdd = LuaAppDomain.GetFunction<Func<int, int, int>>("app", "add");
appMain();
Debug.Log(appAdd(10, 20));
```

| Rule | Detail |
|------|--------|
| Order | `Initialize` first. `GetFunction` before that throws |
| `T` | A `MulticastDelegate`: `Action`, `Func`, or a custom delegate |
| Cache | Keep the delegate on the hot path. `GetFunction` may return a new instance each time |
| Timing | Do not bind from a static field initializer on the same type as `RuntimeInitializeOnLoadMethod` |

`GetFunction` is how C# calls a named Lua export. A Lua `function` passed into a C# `Action` / `Func` parameter is implicit marshal; see `zlua-delegate`. `params` on a `GetFunction` delegate is not supported; see `zlua-params`.

## Reload

`LuaAppDomain.Reset(loader)` clears the script world at end of frame. Delegates from the previous domain are dead and must be bound again. There is no API that only swaps the loader on an already initialized domain. On Android, StreamingAssets is read-only.

| Symptom | Fix |
|---------|-----|
| `module 'app' not found` | Editor path versus Player `.lua.txt`, or the sync step was skipped |
| Call does nothing | Initialize did not run, or the `return` table key does not match |
| Stale script in the Player | StreamingAssets was not re-synced |
