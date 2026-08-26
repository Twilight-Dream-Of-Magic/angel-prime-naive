# Angel Causal Boundary Modular SDK 1.2.0

This additive release introduces a jointly optimized Wilson consumer while
leaving the frozen Angel arithmetic/state implementation and all legacy public
headers byte-for-byte unchanged.

The annals revision adds a finite exact three-class Angel arithmetic layer,
high-dimensional causal functors, deterministic class-quantum history algebra,
a direct polynomial-ring differential suite, explicit derived-observation
validation, corrected legacy coordinate accounting, a typed factorial-maze
functor bridge, a conservative SN--OFCS zero-division reconstruction, ordered
fold-multiplication and fold--shadow theory, and a 162-page color Angel Prime
publication. Difficult proofs and all finite engineering evidence are collected
in appendices.

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

## Three-class Angel arithmetic

The additive `angel::high` API makes the paper's missing high-dimensional layer
executable without changing the frozen legacy arithmetic:

```cpp
#include "angel/high_dimensional.hpp"

using namespace angel::high;

auto x = TriClassValue::ordinary(6);
auto y = TriClassValue::ordinary(3);
auto product = hmul(x, y);          // HMUL
auto packet = hdiv(x, y);           // exact quotient + typed residual
```

`TriClassValue` retains ordinary, ordered-history, and singular-ideal
coordinates. `hadd`, `hsub`, and `hmul` are checked exact operations; `hdiv` is
total for central scalar denominators and reconstructs the numerator from its
quotient and residual, including division by zero. Unsupported generalized
division and coefficient overflow return typed continuations retaining their
operands.

`NativeFunctor` preserves identity and composition on complete `MazeState`
objects. `ClassQuantumFunctor` carries finite integer-weighted deterministic
histories; structural interference and cancellation require complete endpoint
equality. `TensorHistory` certifies class-quantum entanglement by a nonzero exact
`2 x 2` minor. These are exact history semantics, not a physical quantum claim.

## Factorial-maze functor bridge

The additive `angel::factorial_maze` API turns the native factorial coordinate
into an explicit three-class Angel maze chart without rewriting the sealed
principal-jet implementation:

```cpp
#include "angel/factorial_maze.hpp"

using namespace angel::factorial_maze;

auto executed = specification(1009)
              | upload()
              | execute();

auto modular = executed | project_jointly();
auto exact   = executed | derive_exact();
```

`Specification -> UploadedState -> ExecutedState` is enforced by opaque types.
An uploaded state cannot skip execution, and neither terminal consumer can
re-enter native execution. The chart binds factorial argument, compact program,
native state, request and execution result in independent history/certificate
fields while leaving the successful singular coordinate zero. Theory beyond
this implemented chart remains in the paper with an explicit status label.

## Higher-dimensional structural atlas

The post-algorithm research part treats the factorial and cyclic constructions
as exact charts of one q-Pochhammer source. It proves a fixed-prefix procyclic
completion, the room-dependent self-action boundary, all higher factorial-jet
coefficients, Ramanujan-sum cyclic rows, Laplacian/rank/spectrum prime
equivalences, arbitrary tensor-power shadows, semisimple spectral HDIV and an
exact path module for class-quantum interference.

`experiments/angel_structure_probe.cpp` cross-checks the optimized Wilson path
against independent finite shadows. The archived campaign contains 4,095
candidate rows, 255 cyclic rows, 547 period-rank cases and 64 higher-jet rows.
Full-space gluing, universal integral HDIV and native cold Angel Prime birth
remain explicit conjectures/open interfaces.

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

The corrected materialized baseline coordinate is

```text
C0 = block_coefficients + 2 * full_blocks
```

because the baseline owns both an evaluation-point array and a block-value
array. Tail factors are streamed scalar events, not persistent coordinate
slots. `allocation_count` refers only to tracked outer-schedule objects.

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
- independent transform/CRT multiplication and monic-remainder differential
  tests at composite and near-maximum word moduli;
- serial/parallel per-prime transform equivalence;
- derived observation-map tamper rejection with canonical-state immutability;
- three-class HADD/HSUB/HMUL laws and HDIV reconstruction;
- native-functor identity/composition and terminal observation non-authority;
- exact superposition, structural interference/cancellation, and tensor
  entanglement-minor certification;
- the three-class factorial chart, specialized factorial functor and bridge
  certificate;
- negative compilation of both skip-execute and observation-reentry attempts;
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

## Derived observation validation

```cpp
#include "angel/observation_integrity.hpp"

auto validation = angel::boundary::validate_download_packet(closed, packet);
if (!validation.accepted()) {
    // The displayed/transported packet does not match canonical closure.
}
```

Validation compares the complete public observation and causal cut with the
already-bound closed state. It never writes a derived packet back into that
state.

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
- [Publication architecture](docs/publication_architecture.md)
- [High-dimensional arithmetic and functors](docs/high_dimensional_arithmetic.md)
- [Code and manuscript audit](results/CODE_AND_MANUSCRIPT_AUDIT.md)
