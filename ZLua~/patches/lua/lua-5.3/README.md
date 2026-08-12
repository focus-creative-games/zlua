# Lua 5.3.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).  
Shared ranges keep **only the minimum** version file.

| File | Covers (floor) | FastMT (`ZLUA_FAST_METATABLE`) |
|------|----------------|--------------------------------|
| `5.3.0.patch` | `lua-5.3.0` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.3.1.patch` | `lua-5.3.1` | On via `luaV_gettable` / `luaV_settable` (**raw get first**) |
| `5.3.2.patch` | `lua-5.3.2` | On via `finishget` / `finishset` |
| `5.3.3.patch` | `lua-5.3.3` … `lua-5.3.6` | On via `finishget` / `finishset` |

Notes:

- 5.3.0 / 5.3.1 have no `finishget` / `finishset`; FastMT uses the same gettable/settable ordering as 5.1/5.2 (raw table fields before sealed memberTable).
- Do **not** early-exit FastMT before raw `luaH_get` — that breaks type-table fields such as `FullName` / `__tostring`.
- **5.3.2**: getter FastMT must live in `luaV_finishget` (not inside `finishset`); otherwise property/`typeof` reads return raw getter functions.

See spec `11-MULTI-VERSION` §5.2–§5.4.
