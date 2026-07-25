# Lua 5.5.x patches

Install picks the greatest `{X.Y.Z}.patch` with version **&lt;=** Settings micro-version (floor).

| File | Covers (floor) |
|------|----------------|
| `5.5.0.patch` | `lua-5.5.0` (+ later 5.5.x until a newer floor file is added) |

There is no `default.patch`. Note: 5.5 table lookup APIs differ from 5.4 (`luaH_get` tag+out-param, `luaH_Hgetshortstr`, `luaV_finishget` returns a tag).
