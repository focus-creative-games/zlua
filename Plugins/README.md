# Native Lua plugins (Editor)

ZLua does **not** ship every series binary as a hard requirement of Install.  
Put / replace the DLL that matches your Settings `luaVersionId` series:

| Series | Windows file | Define |
|--------|--------------|--------|
| 5.3.x  | `x64/lua53.dll` | `ZLUA_LUA_5_3` |
| 5.4.x  | `x64/lua54.dll` | `ZLUA_LUA_5_4` |
| 5.5.x  | `x64/lua55.dll` | `ZLUA_LUA_5_5` |
| LuaJIT | `x64/luajit.dll` | `ZLUA_USE_LUAJIT` |

Naming: `lua` + major + minor → `lua53` for 5.3.x (no patch digit).
