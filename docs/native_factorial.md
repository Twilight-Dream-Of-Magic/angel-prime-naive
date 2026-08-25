# Native Factorial Consumption

## The repaired binding

The frozen arithmetic result is a self-contained canonical factoradic
coordinate:

```text
rank = n
coefficient = 1
denotation = 1 * n!.
```

Earlier Wilson code verified the surrounding state but then assigned its
working factor count from `candidate - 1`. That produced the correct integer
under the diagonal request contract, but it did not operationally consume the
native result coordinate.

SDK 1.1.0 changes the observer-side dataflow to:

```text
Certified request
  -> verify native result certificate
  -> load factor count from native result rank
  -> verify rank equals principal target and valuation
  -> use candidate only as modular ring order
  -> run the optimized modular factorial algorithm.
```

The published `download_wilson()` API delegates to this repaired path. The
additive `project_wilson_from_native()` API exposes a
`NativeWilsonEvidence` record containing the consumed result and certificate
seals, rank, coefficient, and source flags.

## Runtime exact derivation

`derive_exact_factorial()` interprets the verified factoradic coordinate in an
owned arbitrary-precision integer carrier. The carrier uses a dynamic vector
of 32-bit limbs and does not overflow at 64 bits.

Two implementations run:

1. Sequential small-factor multiplication computes the retained exact value.
2. A balanced product tree computes an independent exact value with general
   limb-by-limb multiplication.

The values must compare equal bit-for-bit. Only then is an
`ExactFactorialValue` returned. The value remains an opaque external object
until `download_exact_factorial()` emits decimal and hexadecimal strings.

The evaluator invariants and induction proof are recorded in
[exact factorial runtime proof](exact_factorial_proof.md).

The regression suite checks complete values for `20!` and `100!`, and checks
the 2,568 decimal digits, 8,530 bits, prefix, suffix, and arbitrary-precision
work ledger of `1000!`.

## Wilson consumption modes

`NativeWilsonEvidence::mode` distinguishes two honest paths:

- `NativeCoordinateModularProjection`: the optimized modular algorithm loads
  its factor count from the native coordinate. It does not materialize `n!`.
- `ExactBigIntegerRemainder`: Wilson directly computes the remainder of the
  independently derived arbitrary-precision integer.

The test matrix compares both modes and the previously published Wilson API
for every candidate from 2 through 64.

## Causal boundary

`NativeFactorialView`, `ExactFactorialValue`, decimal output, and Wilson output
have no conversion back to `FactorialState`. Negative compile tests enforce
that ordinary or externally derived values cannot resume native execution.

The exact big integer is an external operational denotation of the native
coordinate. It is not mislabeled as a new Angel state, and no sealed core or
state type is changed.
