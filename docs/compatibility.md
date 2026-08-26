# Compatibility Contract

## Published API preservation

SDK 1.2.0 is additive. Every public header shipped in the supplied SDK 1.1.0
is listed in `manifest/sdk_1_1_public_headers.sha256` and remains byte-for-byte
identical. The earlier compatibility set remains independently covered by
`manifest/legacy_public_headers.sha256`. `tools/verify_api_compatibility.sh`
enforces both manifests on every full build.

The static-library target remains `angel_causal_boundary`. Existing boundary,
Wilson, native-factorial and cyclic-structure clients are rebuilt and executed
without source changes.

New applications opt in with:

```cpp
#include "angel/joint_wilson.hpp"
```

No legacy umbrella header is edited to inject the additive API transitively.
In particular, the published `angel/version.hpp` remains byte-identical and
continues to report its original compatibility-line value.  Applications that
need the additive package identity may include `angel/release.hpp`, which
reports `1.2.0` without rewriting the old header.

## Frozen implementation preservation

Every sealed arithmetic and state header keeps its historical byte identity.
`manifest/frozen_source.sha256` covers the complete sealed set. The new
consumer reads the existing certified coordinate through the private semantic
adapter and performs only ordinary Download-side work.

## Source and binary scope

The release guarantees source compatibility for the published headers and
preserves all prior public type definitions. It does not promise a stable
binary ABI across unrelated C++ standard libraries, compilers or build flags.
