# Native Lua plugins (Editor)

ZLua does **not** ship every series binary as a hard requirement of Install.  
Put / replace the DLL that matches your Settings `luaVersionId` series:

- 5.3.x: `x64/lua53.dll`, define `ZLUA_LUA_5_3`
- 5.4.x: `x64/lua54.dll`, define `ZLUA_LUA_5_4`
- 5.5.x: `x64/lua55.dll`, define `ZLUA_LUA_5_5`
- LuaJIT 2.1: `x64/luajit21.dll`, defines `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_1`
- LuaJIT 2.0: `x64/luajit20.dll`, defines `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_0`

Naming: `lua` + major + minor → `lua53` for 5.3.x (no patch digit).

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
  lua51/win32-x64/emmy_core.dll   # Windows shipped
  lua52/win32-x64/…
  lua53/win32-x64/…
  lua54/win32-x64/…
  lua55/                          # Windows + macOS + Linux shipped
    win32-x64/emmy_core.dll
    darwin-arm64/emmy_core.dylib
    darwin-x64/emmy_core.dylib
    linux-x64/emmy_core.so
  luajit/win32-x64/…              # Windows shipped (Emmy jit)
```

**Shipped today:** all series above for **Windows x64**; non-Windows currently **`lua55` only**. Other OS/series: build yourself per EmmyLuaDebugger docs.  
Settings: `enableDebugger` / `debuggerPort` / `debuggerWaitIDE` (spec `build/04-EMMYLUA-DEBUGGER`).  
Missing series dir → **LogError and skip** (does not throw).  
**PluginImporter:** keep all `emmy_core` **disabled** on every platform (Lua loads via `package.cpath` only).

**IDE (VS Code / Cursor):** see spec §10 — `launch.json` (`emmylua_new`, `sourcePaths` = Lua root e.g. `Tests/Lua`, `ideConnectDebugger: true`, host `127.0.0.1`). Keep `debuggerWaitIDE` off (`waitIDE` has **no timeout**).

## Mono callback gate (Editor)

`x64/zlua_mono_gate.dll` wraps managed Lua→C# callbacks so `lua_error` runs in native code after the managed frame returns (Editor Mono × all Lua series, including LuaJIT SEH). Rebuild: `ZLua~/mono-native/build_zlua_mono_gate.ps1` / `build_zlua_mono_gate_unix.sh`.

| Platform | Binary | Rebuild |
|----------|--------|---------|
| Windows Editor x64 | `x64/zlua_mono_gate.dll` | `build_zlua_mono_gate.ps1` |
| macOS Editor | `macOS/libzlua_mono_gate.dylib` | `build_zlua_mono_gate_unix.sh` |
| Linux Editor | `Linux/libzlua_mono_gate.so` | same |

`DllImport("zlua_mono_gate")`. Does **not** require patching Lua/LuaJIT sources. Editor-only (`ZLua.Mono`).
