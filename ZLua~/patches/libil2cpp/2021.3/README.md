# libil2cpp 2021.3 patches

Install: floor — greatest `{X.Y.Z}.patch` with version **<=** Editor (letter suffixes ignored).

| File | Covers | Contents |
|------|--------|----------|
| `2021.3.0.patch` | `2021.3.0`–`2021.3.13` | backport `GenericMetadata::ContainsGenericParameters`; LuaAppDomain init |
| `2021.3.14.patch` | `2021.3.14`–`2021.3.30` | LuaAppDomain init |
| `2021.3.31.patch` | `2021.3.31`+ | LuaAppDomain init |

No `default.patch`. Unresolved-call-stub detection is handled in zlua `Il2CppCompatible.h` (`ZLuaIsAnUnresolvedCallStubWasNotFound`), not by patching `GenericMethod`.
