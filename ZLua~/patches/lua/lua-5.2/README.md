# Lua 5.2.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).  
Shared ranges keep **only the minimum** version file.

| File | Covers (floor) | FastMT (`ZLUA_FAST_METATABLE`) |
|------|----------------|--------------------------------|
| `5.2.0.patch` | `lua-5.2.0` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.2.1.patch` | `lua-5.2.1` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.2.2.patch` | `lua-5.2.2` … `lua-5.2.3` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.2.4.patch` | `lua-5.2.4` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |

Notes:

- Same gettable ordering as Lua 5.1 FastMT: never intercept raw fields on sealed type tables (e.g. `__tostring` / `FullName`).
- Includes Il2Cpp lump-safe `luai_num*` / `luai_hashnum` ungates.
- `5.2.1` also casts `-g->GCmemtrav` to `l_mem` (upstream 5.2.2+) to avoid MSVC **C4146**.
- `5.2.4` has a different `Table` field order than earlier 5.2.x; it needs its own floor file.

See spec `11-MULTI-VERSION` §5.2–§5.4.
