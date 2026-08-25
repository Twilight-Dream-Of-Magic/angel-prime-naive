# Compatibility Contract

## Published API preservation

SDK 1.1.0 is additive. Every public header shipped in the preceding modular
SDK is listed in `manifest/legacy_public_headers.sha256` and remains
byte-for-byte identical. `tools/verify_api_compatibility.sh` enforces this on
every full build.

The original static-library target remains `angel_causal_boundary`. Existing
boundary and Wilson source clients are rebuilt and executed unchanged. Their
Wilson implementation is internally repaired to consume the native factorial
coordinate, without changing the public signature or result layout.

New applications opt in with:

```cpp
#include "angel/native_factorial.hpp"
#include "angel/cyclic_structure.hpp"
```

No existing umbrella header was edited to inject the new APIs transitively.

## Frozen implementation preservation

All sealed arithmetic and state headers keep their historical byte identity.
`manifest/frozen_source.sha256` covers every sealed header. The new external
runtime obtains the existing result coordinate through the private adapter; it
does not patch the sealed implementation.

## Compatibility level

This release guarantees source compatibility for the published headers and
preserves all prior public type definitions. As before, it does not promise a
stable binary ABI across arbitrary C++ standard-library implementations,
compilers, or build flags.
