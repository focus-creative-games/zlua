# libil2cpp 2022.3 patches

Install: floor — greatest `{X.Y.Z}.patch` with version **<=** Editor (letter suffixes ignored).

| File | Covers |
|------|--------|
| `2022.3.0.patch` | `2022.3.x` (including Tuanjie `…tN` builds on the 2022.3 line) |

No `default.patch`. Authored against 2022.3.62f3 (unresolved-stub helper + LuaAppDomain init).

`Array::IndexFromIndices` is **not** patched here: early 2022.3 (e.g. 0 / 11) lacks the API; zlua-runtime `Il2CppCompatible.h` uses a manual formula when `ZLUA_UNITY_VERSION < 20220324`.
