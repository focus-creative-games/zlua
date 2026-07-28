# Native Lua plugins (Editor)

ZLua does **not** ship every series binary as a hard requirement of Install.  
Put / replace the binary that matches your Settings `luaVersionId` series under `Plugins/lua/<series>/`:

```text
Plugins/lua/
  lua51/{lua51.dll, lua51.dylib}
  lua52/{lua52.dll, lua52.dylib}
  lua53/{lua53.dll, lua53.dylib}
  lua54/{lua54.dll, lua54.dylib}
  lua55/{lua55.dll, lua55.dylib}
  luajit20/{luajit20.dll, luajit20.dylib}  # macOS: x86_64 only (2.0 has no arm64)
  luajit21/{luajit21.dll, luajit21.dylib}
```

| Series folder | Define(s) |
|---------------|-----------|
| `lua51` … `lua55` | `ZLUA_LUA_5_1` … `ZLUA_LUA_5_5` |
| `luajit21` | `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_1` |
| `luajit20` | `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_0` |

Naming: `lua` + major + minor → `lua53` for 5.3.x (no patch digit).  
Windows: `*.dll` (Editor x86_64). macOS: `*.dylib` (universal when available).

## EmmyLua debugger (Editor)

Upstream: [EmmyLua/EmmyLuaDebugger](https://github.com/EmmyLua/EmmyLuaDebugger)  
(`EMMY_LUA_VERSION` default **55**; build options / usage → that README.)

**Series folder naming:**

| Engine | Folder |
|--------|--------|
| Official Lua | `lua{major}{minor}` → e.g. `lua55`, `lua54`, `lua53` |
| LuaJIT | **`luajit`** (2.0 / 2.1 share one folder; Emmy `-DEMMY_LUA_VERSION=jit`) |

One build per **PUC major series** (not per patch); all LuaJIT 2.x share **`luajit/`**. Example layout:

```text
emmylua/
  lua51/{win32-x64,darwin-arm64,darwin-x64}/…
  lua52/{win32-x64,darwin-arm64,darwin-x64}/…
  lua53/{win32-x64,darwin-arm64,darwin-x64}/…
  lua54/{win32-x64,darwin-arm64,darwin-x64}/…
  lua55/{win32-x64,darwin-arm64,darwin-x64,linux-x64}/…
  luajit/{win32-x64,darwin-arm64,darwin-x64}/…  # 2.0/2.1 share one folder
```

**Shipped today:** Windows x64 for `lua51`–`lua55` + `luajit`; macOS arm64+x64 for `lua51`–`lua55` + `luajit`; Linux x64 for `lua55`.  
(`luajit20` macOS Editor lib is **x86_64 only** — LuaJIT 2.0 has no arm64.)  
Settings: `enableDebugger` / `debuggerPort` / `debuggerWaitIDE` (spec `build/04-EMMYLUA-DEBUGGER`).  
Missing series dir → **LogError and skip** (does not throw).  
**PluginImporter:** keep all `emmy_core` **disabled** on every platform (Lua loads via `package.cpath` only).

**IDE (VS Code / Cursor):** see spec §10 — `launch.json` (`emmylua_new`, `sourcePaths` = Lua root e.g. `Tests/Lua`, `ideConnectDebugger: true`, host `127.0.0.1`). Keep `debuggerWaitIDE` off (`waitIDE` has **no timeout**).

## Mono callback gate (Editor)

`Plugins/lua/zlua_mono_gate.dll` / `Plugins/lua/libzlua_mono_gate.dylib` wrap managed Lua→C# callbacks so `lua_error` runs in native code after the managed frame returns (Editor Mono × all Lua series, including LuaJIT SEH). Rebuild: `ZLua~/mono-native/build_zlua_mono_gate.ps1` / `build_zlua_mono_gate_unix.sh`.

| Platform | Binary | Rebuild |
|----------|--------|---------|
| Windows Editor x64 | `Plugins/lua/zlua_mono_gate.dll` | `build_zlua_mono_gate.ps1` |
| macOS Editor | `Plugins/lua/libzlua_mono_gate.dylib` (universal arm64+x86_64) | `build_zlua_mono_gate_unix.sh` |
| Linux Editor | `Plugins/lua/libzlua_mono_gate.so` | same |

`DllImport("zlua_mono_gate")`. Does **not** require patching Lua/LuaJIT sources. Editor-only (`ZLua.Mono`).
