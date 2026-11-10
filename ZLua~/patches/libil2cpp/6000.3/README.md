# libil2cpp 6000.3 LTS patches

| File | Covers |
|------|--------|
| `6000.3.0.patch` | Unity 6.3 LTS (`6000.3.x`) |

The GenericMethod header has AcquireMetadataLocks / ReleaseMetadataLocks,
so this series needs its own context instead of the 6000.0 fallback patch.
Verified with `git apply --check` on 6000.3.0f1 and 6000.3.22f1.
