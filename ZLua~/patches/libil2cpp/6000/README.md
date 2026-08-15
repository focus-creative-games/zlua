# libil2cpp 6000 (Unity 6) patches

Series dirs: try `6000.{minor}/` first, then this `6000/` fallback.  
Within a dir: floor — greatest `{X.Y.Z}.patch` **<=** Editor.

| File | Covers |
|------|--------|
| `6000.0.0.patch` | `6000.x.y` when resolved under `6000/` (e.g. 6000.0–6000.3 era) |

For **6000.5.6+**, prefer series dir `6000.5/` (`6000.5.6.patch`) — `GenericMethod.h` signatures diverged from 6000.0.

No `default.patch`. Authored against 6000.0.71f1; verified dry-run on 6000.3.16.
