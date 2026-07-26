# Native Lua plugins (Editor)

ZLua does **not** ship every series binary as a hard requirement of Install.  
Put / replace the DLL that matches your Settings `luaVersionId` series:

- 5.3.x: `x64/lua53.dll`, define `ZLUA_LUA_5_3`
- 5.4.x: `x64/lua54.dll`, define `ZLUA_LUA_5_4`
- 5.5.x: `x64/lua55.dll`, define `ZLUA_LUA_5_5`
- LuaJIT 2.1: `x64/luajit21.dll`, defines `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_1`
- LuaJIT 2.0: `x64/luajit20.dll`, defines `ZLUA_USE_LUAJIT`, `ZLUA_LUAJIT_2_0`

Naming: `lua` + major + minor → `lua53` for 5.3.x (no patch digit).

## Mono callback gate (Editor)

`x64/zlua_mono_gate.dll` wraps managed Lua→C# callbacks so `lua_error` runs in native code after the managed frame returns (Editor Mono × all Lua series, including LuaJIT SEH). Rebuild: `ZLua~/mono-native/build_zlua_mono_gate.ps1` / `build_zlua_mono_gate_unix.sh`.

| Platform | Binary | Rebuild |
|----------|--------|---------|
| Windows Editor x64 | `x64/zlua_mono_gate.dll` | `build_zlua_mono_gate.ps1` |
| macOS Editor | `macOS/libzlua_mono_gate.dylib` | `build_zlua_mono_gate_unix.sh` |
| Linux Editor | `Linux/libzlua_mono_gate.so` | same |

`DllImport("zlua_mono_gate")`. Does **not** require patching Lua/LuaJIT sources. Editor-only (`ZLua.Mono`).
