---
name: zlua-setup
description: >-
  Installs and builds ZLua in a Unity project: UPM package, Lua version,
  ZLua/Install, ZLua/Generate/All, Editor versus Il2Cpp Player script paths,
  and HybridCLR install order. Use when the user adds the package, changes
  luaVersionId, builds a Player, or hits a missing-Install or module-not-found error.
---

# ZLua setup

The package does not ship a full `libil2cpp` or Lua source tree. Editor Play and Il2Cpp Player share Lua-visible semantics. Player builds need Install, Generate, and a script sync.

## Add the package

```json
{
  "dependencies": {
    "com.code-philosophy.zlua": "https://github.com/focus-creative-games/zlua.git"
  }
}
```

Pin a tag with a URL fragment such as `#v0.0.1-alpha.2` when a fixed release is required.

## Settings and Install

Menu `ZLua/Settings...` writes `ProjectSettings/ZLua.asset`.

| Field | Role |
|-------|------|
| Enable | Turn ZLua on |
| Lua Version Id | Empty means `lua-5.5.0`. PUC-Rio is `lua-X.Y.Z`. LuaJIT is `luajit-M.N` (clone the sources yourself; Il2Cpp only Android/iOS). Luau is `luau-M.N` on the official `luau` branch (Il2Cpp currently Windows x64) |
| MarshalAs / Lua Alias / Lua Extension Xml Paths | Optional. Non-hot-update assemblies only. See `zlua-marshal-as`, `zlua-alias`, and `zlua-extension` |

Then run `ZLua/Install...`. It copies the Editor `libil2cpp`, applies patches, installs the selected Lua under `libil2cpp/lua`, and copies `ZLua~/zlua-runtime` to `libil2cpp/zlua`. Changing Unity or the Lua version means Install again. If the Editor native DLL or scripting defines change, restart the Editor.

HybridCLR: install `com.code-philosophy.hybridclr` (v9+) as well, run `HybridCLR/Installer` first, then `ZLua/Install`.

## Player build

1. Install has already succeeded.
2. `ZLua/Generate/All` writes C++ stubs. This is not a C# wrap. Editor iteration does not require Generate every time. Run it again after public API changes or a Lua series change.
3. Sync `LuaScripts` to `StreamingAssets/LuaScripts/*.lua.txt` before the Player build (`app` becomes `app.lua.txt`).
4. Build with the Il2Cpp scripting backend.

| | Editor (Mono) | Player (Il2Cpp) |
|--|---------------|-----------------|
| Lua semantics | Same | Same |
| Generate | Not required to Play | Required |
| Scripts | `{ProjectRoot}/LuaScripts/*.lua` | `StreamingAssets/LuaScripts/*.lua.txt` |

## Failures

| Symptom | Fix |
|---------|-----|
| Package fetch failed | Git access, or a local `file:` path |
| Not installed | `ZLua/Install...` |
| Lua download failed | Check `luaVersionId` and network. PUC-Rio cache is `Library/ZLua/LuaSrcCache/` |
| Player `module not found` | Sync missed, or the `.lua.txt` suffix is wrong |
| Editor works, Player crashes | Generate was skipped |
| Odd behavior after switching Lua series | Restart the Editor, then Install and Generate |
