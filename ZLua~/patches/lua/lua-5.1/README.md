# Lua 5.1.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).  
Shared ranges keep **only the minimum** version file.

| File | Covers (floor) | FastMT (`ZLUA_FAST_METATABLE`) |
|------|----------------|--------------------------------|
| `5.1.0.patch` | `lua-5.1.0` … `lua-5.1.1` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.1.2.patch` | `lua-5.1.2` … `lua-5.1.5` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |

Notes:

- Lua 5.1 has no `finishget` / `finishset`; FastMT hooks the whole-table get/set paths **after** a raw table miss (so type-table fields like `FullName` still work).
- Patches also force `GetModuleFileNameA` / `LoadLibraryA` / `FormatMessageA` so Unity `UNICODE` builds do not treat UTF-16 module paths as narrow C strings.
- Install also renames Lua 5.1 `getline` → `getfuncline` (Android NDK `stdio.h` POSIX `getline` clash).
- There is no `default.patch`. Upstream sources download into `Library/ZLua/LuaSrcCache/` on Install (archive name for 5.1.0 is `lua-5.1.tar.gz`).

See spec `11-MULTI-VERSION` §5.2–§5.4.
