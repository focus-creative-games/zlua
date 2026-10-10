# libil2cpp patches for HybridCLR trees

Used instead of `patches/libil2cpp/` when **HybridCLR support** is enabled
(`ZLua/Settings` → *Enable HybridCLR Support*). Same floor resolution rule:
greatest `{X.Y.Z}.patch` with version **<=** Editor, no `default.patch`.

HybridCLR replaces `libil2cpp` wholesale from the `il2cpp_plus` repo, tagged per
Unity LTS series (e.g. `v2022.3.x-9.0.0`), so **one patch per series is exact** —
micro versions within a series share the same libil2cpp. That is why the floor
patch here is always `X.Y.0.patch`.

| Series | File | Covers |
|--------|------|--------|
| `2022.3` | `2022.3.0.patch` | `2022.3.x`, including Tuanjie `…tN` builds |
| `6000.0` | `6000.0.0.patch` | `6000.0.x` |
| `6000.3` | `6000.3.0.patch` | `6000.3.x` |

Only these series exist because HybridCLR itself supports only these (see
`Data~/hybridclr_version.json` in the HybridCLR package). Unity `2021.3` and
`6000.5` have zlua patches under `patches/libil2cpp/` but no HybridCLR support,
so enabling the toggle on them fails with an explicit error.

## Content

Same two edits as the non-HybridCLR patches, re-anchored for the HybridCLR tree:

1. `metadata/GenericMethod.{cpp,h}` — add `IsAnUnresolvedCallStubWasNotFound`.
2. `vm/Runtime.cpp` — include `zlua/lvm/LuaAppDomain.h` and call
   `zlua::LuaAppDomain::Initialize()` at the end of `Runtime::Init`.

The Lua init call lands **after** HybridCLR's own `hybridclr::Runtime::Initialize()`
in the same function, so the interpreter is live before any Lua runs. The
generator asserts this ordering.

## Regenerating

Do not hand-edit. Use the generator, which derives each patch from a real
HybridCLR-installed `libil2cpp` and verifies it applies with **zero fuzz**
(and that the 2022.3 patch also applies to the Tuanjie tree):

```
python tools/gen_hybridclr_patches.py \
    --out-root <zlua>/ZLua~/patches/libil2cpp-hybridclr
```

Point `SERIES` in that script at your own reference trees. After a HybridCLR
upgrade that changes these files, re-run it and re-verify.
