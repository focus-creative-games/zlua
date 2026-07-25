# Lua 5.3.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).  
Shared ranges keep **only the minimum** version file.

| File | Covers (floor) | FastMT (`ZLUA_FAST_METATABLE`) |
|------|----------------|--------------------------------|
| `5.3.0.patch` | `lua-5.3.0` | **Off** (unsupported; Install forces `0`) |
| `5.3.1.patch` | `lua-5.3.1` | **Off** (unsupported; Install forces `0`) |
| `5.3.2.patch` | `lua-5.3.2` | On via `finishget` / `finishset` |
| `5.3.3.patch` | `lua-5.3.3` … `lua-5.3.6` | On via `finishget` / `finishset` |

There is no `default.patch`. Upstream sources download into `Library/ZLua/LuaSrcCache/` on Install.

See spec `11-MULTI-VERSION` §5.2–§5.4.
