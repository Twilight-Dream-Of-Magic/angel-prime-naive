# Angel Causal Boundary Modular SDK 1.2.0

This additive release introduces a jointly optimized Wilson consumer while
leaving the frozen Angel arithmetic/state implementation and all legacy public
headers byte-for-byte unchanged.

The published `angel/version.hpp` is intentionally preserved as part of the
SDK 1.1.0 compatibility surface.  New code may include `angel/release.hpp` to
read the additive package identity `1.2.0`.

The release preserves the canonical causal boundary:

```text
known ordinary specification
        -> Proxy Upload
        -> complete native factorial state
        -> actual native state-to-state arithmetic
        -> certified native factorial coordinate
        -> explicit Download boundary
        -> ordinary modular observation
```

The new consumer does not reconstruct the factor count from `candidate - 1`.
It reads

```text
rank = n
coefficient = 1
denotation = n!
```

from the independently verified native factorial coordinate, and uses the
candidate only as the modulus and as the separately checked equality
`candidate = n + 1`.

## Joint time-space optimization

Let `m` be the Wilson modulus, `n = m - 1`, and `h = floor(n/2)`.  Pairing the
factors `k` and `m-k` gives the exact integral identities

```text
n even: n! = (-1)^h (h!)^2              modulo m
n odd : n! = (-1)^h (h!)^2 (h+1)        modulo m
```

They hold for every modulus, including composite moduli.  No inverse, field
assumption, rational relaxation, factorization, or prime-specific branch is
used.

The implementation therefore evaluates only `h! mod m`, selects a bounded
near-square block schedule for `h`, omits the zero evaluation point from the
multipoint tree, streams block values directly into the scalar residue, and
releases completed remainder branches immediately.  It also starts Horner
evaluation from the leading coefficient and eliminates degree-one leaf
remainders after their parent products have been formed.  The complete `n!`
is never materialized.

This is a real local Pareto improvement, not an asymptotic-order claim:

```text
old time:  O(M(sqrt(n)) log n + sqrt(n)) ring operations
new time:  O(M(sqrt(n/2)) log n + sqrt(n/2)) + O(1)
old space: O(sqrt(n) log n) live coefficients
new space: O(sqrt(n/2) log n) live coefficients
```

Under standard multiplication models the old and new paths remain in the same
asymptotic class.  The release therefore reports `asymptotic improvement = NO`
and proves a deterministic engineering improvement instead.  On the frozen
operation-count sequence, both the deterministic work count and the
conservative peak-live-coefficient bound are strictly lower for every tested
input.  The instrumented old/new replay also shows strict reductions in ring
additions, ring multiplications, modular reductions, coefficient updates,
allocation count, and peak live limbs throughout the continuous release
interval.  The square-root coordinate is reduced but not eliminated.

## Build and verify

```bash
./build_and_test.sh
```

The script runs:

- Debug and optimized-output equivalence;
- address and undefined-behaviour checks;
- old public pipeline tests;
- native factorial and exact arbitrary-precision tests;
- the jointly optimized Wilson tests;
- a continuous Pareto audit;
- strict old/new ring-addition, ring-multiplication, modular-reduction,
  coefficient-update and peak-limb comparisons;
- negative compilation boundaries;
- frozen-source, legacy-header and supplied-SDK-header hash verification;
- deterministic old/new operation-count generation;
- the public-name audit.

## Existing Wilson API

The published API remains unchanged:

```cpp
#include "angel/prime.hpp"

using namespace angel::prime;

auto result = candidate(1009)
            | upload_factorial_state()
            | bind_quotient_view()
            | download_wilson();
```

## Jointly optimized API

```cpp
#include "angel/joint_wilson.hpp"

using namespace angel::prime;

JointWilsonPolicy policy{};
policy.parallel_ntt_primes = true;

auto result = candidate(1009)
            | upload_factorial_state()
            | bind_native_factorial()
            | project_wilson_jointly(policy);
```

`JointWilsonDownload` contains:

- the ordinary Wilson residue and decision;
- the reduced and legacy coordinate dimensions;
- deterministic ring, polynomial, limb and allocation counters;
- an exact outer live-coefficient peak and a conservative scratch-inclusive
  peak bound;
- state-integrity evidence;
- explicit flags proving no full factorial materialization and no ordinary
  feedback.

## Exact arbitrary-precision path

The full output path remains separate:

```cpp
#include "angel/native_factorial.hpp"

using namespace angel::prime;

auto exact = candidate(101)
           | upload_factorial_state()
           | bind_native_factorial()
           | derive_exact_factorial();

auto integer = exact | download_exact_factorial();
auto exact_wilson = exact | observe_wilson_from_exact();
```

This path materializes every bit of `100!` and is charged accordingly.  Its
`Theta(n log n)` output-size lower bound does not apply to the modular Wilson
consumer, which never expands `n!`.

## Frozen and additive boundaries

- All frozen arithmetic/state headers remain byte-identical.
- All legacy public headers remain byte-identical.
- Existing clients compile without source changes.
- The new public API is additive.
- Native state nodes rewritten: `0`.
- Ordinary observation fed back into native state: `0`.
- Fixed-width truncation used as arbitrary-precision proof: `0`.

Read:

- [Joint time-space optimization](docs/joint_time_space_optimization.md)
- [Time and space complexity](docs/complexity.md)
- [Native factorial semantics](docs/native_factorial.md)
- [Architecture](docs/architecture.md)
- [State integrity](docs/state_integrity.md)
- [Limitations](docs/limitations.md)
- [Compatibility](docs/compatibility.md)
