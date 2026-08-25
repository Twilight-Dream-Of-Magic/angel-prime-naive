# Angel Causal Boundary Modular SDK 1.1.0

This additive release fixes the factorial-consumption gap without changing the
published public types or the sealed arithmetic/state implementation.

The runtime now exposes two explicit paths from the same native factorial
coordinate:

```text
FactorialState
  -> NativeFactorialView
     -> optimized modular Wilson projection
     -> exact arbitrary-precision factorial derivation
        -> exact big-integer Wilson remainder
```

The optimized Wilson path loads its factor count from
`AngelFactorialResult.value().rank()`. It no longer assigns the factor count
from `candidate - 1` inside the observer. The candidate is retained as the
modulus and as an independently checked diagonal binding.

The exact path materializes the denotation of that native factoradic
coordinate with an owned arbitrary-precision integer. A sequential derivation
and an independent product-tree derivation must agree bit-for-bit before the
value is certified. Wilson can then consume that exact big integer directly.

Neither external path rewrites, merges, compresses, or feeds an ordinary result
back into the arithmetic state.

## Build and verify

```bash
./build_and_test.sh
```

The script runs Debug/O3 equivalence, AddressSanitizer,
UndefinedBehaviorSanitizer, negative compile tests, frozen-source verification,
legacy-header compatibility, exact factorial fixtures, native-coordinate
Wilson consumption, cyclic-structure theorems, and the public-name audit.

## Existing boundary pipeline

```cpp
#include "angel/boundary.hpp"

using namespace angel::boundary;

SessionAuthority authority{0xCA550001U};
BoundaryLedger ledger{};

auto packet = EncodedOrder::canonical(24)
            | upload(authority, &ledger)
            | quotient_to(6, &ledger)
            | continue_to(4, &ledger)
            | observe_primitive(4, 998244353U, 1, &ledger)
            | download(&ledger);
```

## Published Wilson pipeline, repaired internally

```cpp
#include "angel/prime.hpp"

using namespace angel::prime;

auto result = candidate(1009)
            | upload_factorial_state()
            | bind_quotient_view()
            | download_wilson();
```

The source API is unchanged. `download_wilson()` now delegates to the
native-factorial-coordinate consumer.

## Explicit native factorial and exact big integer

```cpp
#include "angel/native_factorial.hpp"

using namespace angel::prime;

auto native = candidate(101)
            | upload_factorial_state()
            | bind_native_factorial();

auto fast_wilson = native | project_wilson_from_native();

auto exact = native | derive_exact_factorial();
auto integer = exact | download_exact_factorial();
auto exact_wilson = exact | observe_wilson_from_exact();
```

For this example, `integer.decimal` is the complete decimal expansion of
`100!`. `fast_wilson.evidence` proves that the optimized modular algorithm
loaded its factor count from the native result coordinate. `exact_wilson`
computes the residue from the owned arbitrary-precision integer.

## Cyclic structure pipeline

```cpp
#include "angel/cyclic_structure.hpp"

using namespace angel::prime;

CyclicActionPolicy policy{};
policy.include_normalized_action = true;
policy.compute_kernel_dimension = true;

auto structure = candidate(15)
               | upload_factorial_state()
               | bind_cyclic_action()
               | evaluate_cyclic_action(policy)
               | download_cyclic_structure();
```

This exact reference observer verifies the primitive-period response,
Ramanujan coordinates, valuation reattachment, constant-mode factorial, and
proper-period kernel dimension.

## Honest complexity boundary

- Native factorial coordinate construction and storage remain succinct.
- Rebinding the optimized Wilson path to that coordinate adds only binding
  checks; its previous modular T-S asymptotics are unchanged.
- Fully deriving and downloading `n!` is not succinct execution. Its output
  alone has `Theta(n log n)` bits. The included schoolbook arbitrary-precision
  oracle uses polynomial time and `Theta(n log n)` result space.
- The full cyclic response remains `Theta(m^2)` fixed-width modular work and
  `Theta(m)` coefficient space.

No `O(log n)` full-integer output, polylogarithmic Wilson execution,
information-boundary optimality, next-prime transition, or native Prime-Birth
compiler is claimed.

## Compatibility and structure

- All 16 previously published public headers are byte-preserved.
- All 13 sealed source headers are byte-preserved.
- The static-library target remains `angel_causal_boundary`.
- Public code filenames and symbols use semantic names, not release codenames.
- Historical lineage names are confined to sealed source, one private adapter,
  and explanatory document prose.

Read [native factorial](docs/native_factorial.md),
[cyclic structure](docs/cyclic_structure.md),
[complexity](docs/complexity.md),
[state integrity](docs/state_integrity.md), and
[compatibility](docs/compatibility.md) before integration.
