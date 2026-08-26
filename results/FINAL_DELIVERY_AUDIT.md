# Final Delivery Audit

## Scope

This audit freezes the additive SDK release after the joint Wilson time-space
optimization, paper update, compatibility verification, sanitizer verification,
and deterministic operation-count replay.

## Required final answers

| Required question | Frozen answer |
|---|---|
| Old time complexity | `O(M(sqrt(n)) log n + sqrt(n))` |
| New time complexity | `O(M(sqrt(n/2)) log n + sqrt(n/2)) + O(1)` |
| Old space complexity | `O(sqrt(n) log n)` live coefficients |
| New space complexity | `O(sqrt(n/2) log n)` live coefficients |
| Time asymptotically improved | **No** |
| Space asymptotically improved | **No** |
| Both reduced in the same implementation | **Yes** |
| Square-root coordinate eliminated | **No** |
| Full `n!` materialized in optimized Wilson path | **No** |
| Wilson consumed certified native factorial coordinate | **Yes** |
| Angel state changed | **No** |

This is the permitted local-result branch: deterministic work and peak space
both strictly decrease in one executable implementation, while neither
asymptotic class worsens.

## Exact mathematical transformation

For modulus `m=n+1`, let `h=floor(n/2)` and `epsilon=n mod 2`. Pairing `k`
with `m-k` gives

\[
k(m-k)\equiv-k^2\pmod m,
\]

and therefore

\[
n!\equiv(-1)^h(h!)^2(h+1)^\epsilon\pmod m.
\]

The identity is integral, uses no inverse, and follows the same flow for prime,
composite, even, odd, and zero-divisor moduli.

## Executed optimization

The optimized consumer:

1. reads `n` only from the independently verified native factorial rank;
2. uses the ordinary candidate only as modulus and as the check `m=n+1`;
3. evaluates `h! mod m` through the composite-safe polynomial engine;
4. generates arithmetic-progression points at leaves instead of materializing
   a point array;
5. streams block values into one scalar accumulator instead of materializing a
   block-value array;
6. retires completed remainder branches depth-first;
7. starts Horner evaluation from the leading coefficient;
8. eliminates degree-one leaf remainder construction;
9. applies the exact square, sign, and optional central-factor reconstruction;
10. returns an ordinary observation with no ascent conversion.

## Deterministic Pareto evidence

The continuous audit covers every candidate from `2` through `2048`:

```text
residue equality                         PASS
new deterministic work < old            PASS for 2,047 / 2,047 inputs
new ring additions < old                 PASS for 2,047 / 2,047 inputs
new ring multiplications < old           PASS for 2,047 / 2,047 inputs
new modular reductions < old             PASS for 2,047 / 2,047 inputs
new coefficient updates < old            PASS for 2,047 / 2,047 inputs
new allocation count < old               PASS for 2,047 / 2,047 inputs
new peak coefficient/limb bound < old    PASS for 2,047 / 2,047 inputs
new materialized coordinate < old        PASS for 2,047 / 2,047 inputs
```

The public-path differential test additionally covers candidates `2..1024`,
block boundaries, squares, prime powers, semiprimes, and multi-factor
composites. The fixed increasing sequence in
`joint_wilson_operation_counts.csv` has:

```text
T_new/T_old range = 0.44116366 .. 0.64170827
S_new/S_old range = 0.19169256 .. 0.52803056
C_new/C_old range = 0.18367347 .. 0.23759791
```

These finite ratios support the scoped Pareto claim only. They are not fitted
into an unsupported asymptotic theorem.

## Correctness and toolchain verification

```text
CMake frontend in current environment          SKIPPED: executable unavailable
Portable canonical build script                PASS
Debug/Release observable-output comparison     IDENTICAL
AddressSanitizer                               PASS
UndefinedBehaviorSanitizer                     PASS
LeakSanitizer                                  SKIPPED: ptrace-restricted environment
Negative compile cases                         PASS (14 / 14 expected failures)
Frozen arithmetic source hashes                PASS
Legacy public header hashes                    PASS
Supplied SDK 1.1 public header hashes          PASS
Old source-client compatibility                PASS
Public-name audit                              PASS
Transform/CRT and monic-remainder differential PASS
Parallel/serial NTT equivalence                PASS
Derived observation tamper rejection           PASS
Three-class HADD/HSUB/HMUL and HDIV             PASS
Native functor identity/composition             PASS
Structural interference/cancellation           PASS
Exact entanglement-minor certificate            PASS
Three-class factorial maze chart                PASS
Specialized factorial functor bridge            PASS
Same shadow/different factorial history         PASS
Wilson structural rows                          PASS (4,095 / 4,095)
Cyclic pure-Laplacian equivalence                PASS (255 / 255)
Prime cyclic coordinate-fold collision           PASS (54 / 54)
Period-rank filtration                          PASS (547 / 547)
Higher factorial-jet identities                 PASS (64 / 64)
```

A CMake propagation defect was repaired additively: sanitizer runtime link
flags now propagate through the static library so sanitizer-enabled examples
and probes link correctly. No frozen source or prior public header was changed.

Independent exact-factorial fixtures cover `0!`, `1!`, `20!`, `100!`, and
`1000!`. The exact arbitrary-precision path is accounted separately from the
non-materializing Wilson path.

## State and boundary invariants

```text
native state nodes rewritten   0
native state nodes merged      0
ordinary feedback              0
full factorial materialized    0
silent fixed-width truncation  0
```

The public modular chart remains the supplied `uint64_t` chart with typed
resource rejection. The release does not claim an arbitrary-precision Wilson
modulus API. The exact factorial output path remains arbitrary precision.

## High-dimensional Angel layer

The additive `angel::high` surface realizes the paper's finite exact
`Ordinary + History + Singular` model. Ordered-word HMUL retains causal order;
the singular sector is a two-sided ideal; central-scalar HDIV returns a quotient
and typed residual that reconstruct the numerator, including at denominator
zero. Native functors preserve identity and composition on complete maze states.
Class-quantum superposition is an integer-weighted deterministic history module;
interference and cancellation require complete structural endpoint equality,
and entanglement requires a nonzero exact coefficient minor.

The release does not claim generalized inversion for arbitrary noncentral
denominators, unbounded coefficients in this sparse chart, a Born rule, unitary
physics, or execution on quantum hardware.

The additive `angel::factorial_maze` surface realizes the specialized
principal-jet bridge as opaque `Specification -> UploadedState ->
ExecutedState` stages. Only the executed carrier admits exact or joint terminal
consumers. The bridge is additive and leaves all frozen/supplied headers and
sealed arithmetic source byte-identical.

## Paper verification

The updated manuscript is a 162-page color A4 PDF. The body includes the formal
Angel naming boundary, Angel primality/state/transition definitions, three
families of high-dimensional axioms, all four operations, causal and
class-quantum functors, the SN--OFCS precursor and conservative zero division,
ordered fold multiplication, factorial-maze/Wilson/cyclic charts, the scoped
fold--shadow collision, the Volume V residual-horizon line, procyclic
completion, higher factorial jets, Ramanujan shadows, Laplacian prime
equivalences, tensor observers and exact path module.
Complete proofs of the difficult model, arithmetic,
functor, spectral, closure-valuation, least-factor, barrier, source-child,
stability, zipper and ramified-transport results are in technical appendices;
finite computational/engineering evidence is confined to evidence appendices.

## Final classification

```text
result                              COMPLETE_SCOPED_LOCAL_JOINT_PARETO
joint asymptotic improvement        NO
joint executable local improvement  YES
square-root coordinate eliminated   NO
native factorial semantics changed  NO
ordinary feedback introduced        NO
```
