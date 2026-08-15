# libil2cpp 6000.5 patches

Series dirs: try `6000.5/` first, then `6000/` fallback.  
Within a dir: floor — greatest `{X.Y.Z}.patch` **<=** Editor.

| File | Covers |
|------|--------|
| `6000.5.0.patch` | `6000.5.0+` |

Notes vs `6000/6000.0.0.patch`:
- `InflateRGCTX` gained `Il2CppException** exc`
- Extra statics / APIs around `GenericMethod` (locks, entry-point stubs)
