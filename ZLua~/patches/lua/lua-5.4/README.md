# Lua 5.4.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).  
Shared ranges keep **only the minimum** version file.

| File | Covers (floor) | Notes |
|------|----------------|-------|
| `5.4.0.patch` | `lua-5.4.0` … `lua-5.4.2` | `unlikely`, `L->top` |
| `5.4.3.patch` | `lua-5.4.3` | `l_unlikely`, `L->top` |
| `5.4.4.patch` | `lua-5.4.4` | `l_unlikely`, `L->top` |
| `5.4.5.patch` | `lua-5.4.5` … `lua-5.4.6` | `l_unlikely`, `L->top.p` (StkIdRel) |
| `5.4.7.patch` | `lua-5.4.7` … `lua-5.4.8` | `l_unlikely`, `L->top.p` |

There is no `default.patch`. Upstream sources download into `Library/ZLua/LuaSrcCache/` on Install.
