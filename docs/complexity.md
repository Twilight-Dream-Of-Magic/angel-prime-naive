# Time and Space Complexity

This ledger distinguishes native Angel execution, ordinary modular
observation, and full ordinary materialization. A succinct program is not
counted as completed bit-level execution.

Let:

- `m` be the Wilson candidate;
- `n = m - 1` be the factorial argument read from the certified native
  coordinate;
- `b = ceil(log2(n + 1))` be the input bit length;
- `s0 = ceil(sqrt(n))` be the legacy external block width;
- `h = floor(n/2)` and `s1 = Theta(sqrt(h))` be the reduced problem and its
  selected near-square width;
- `M(d)` be the cost of multiplying degree-`d` polynomials in the exact
  composite-safe modular engine;
- `r` be a cyclic state order, `c` its cycle multiplicity, and `J` a requested
  jet horizon.

## Native factorial stage

| Stage | Time | Additional live space | Result |
|---|---:|---:|---|
| `upload_factorial_state` | `Theta(b)` native ledger steps | `Theta(b)` | Succinct program, complete principal-jet state, certificate |
| `bind_native_factorial` | replay linear in the succinct program | `O(1)` handle overhead | Independently verified rank/coefficient view |

The native state remains live and unchanged after binding. These costs are
reported separately from Download-side modular work.

## Previous Wilson baseline

The legacy external consumer uses

\[
P_s(X)=\prod_{i=1}^{s}(X+i),
\qquad s=s_0,
\]

and multipoint evaluation at all block origins. Its charged bounds are

\[
T_0(n)=O(M(s_0)\log s_0+s_0),
\qquad
S_0(n)=O(s_0\log s_0).
\]

Its materialized external coordinate has exactly

\[
C_0(n)=s_0+1+\lfloor n/s_0\rfloor+(n\bmod s_0)
\]

scalar slots. This is \(\Theta(\sqrt n)\), not polylogarithmic.

## Jointly optimized Wilson consumer

The exact complement identity reduces the polynomial problem from `n! mod m`
to `h! mod m`:

\[
n!\equiv
\begin{cases}
(-1)^h(h!)^2 & n\text{ even},\\
(-1)^h(h!)^2(h+1) & n\text{ odd}
\end{cases}
\pmod m.
\]

The new path also:

- selects a bounded near-square width that minimizes `w + floor(h/w) + h%w`;
- reads the zero block from the block polynomial's constant coefficient;
- generates evaluation points at leaves rather than storing a point array;
- multiplies each block value directly into the scalar residue rather than
  storing a value array;
- processes two top-level evaluation ranges sequentially;
- releases completed remainder branches before constructing their siblings;
- initializes Horner evaluation from the leading coefficient rather than a
  synthetic zero accumulator;
- releases degree-one leaf products after their parent is formed and evaluates
  the reduced parent polynomial directly at leaf points, avoiding degree-one
  remainder calls.

No product or remainder is recomputed. The new bounds are

\[
T_1(n)=O(M(s_1)\log s_1+s_1)+O(1),
\]

\[
S_1(n)=O(s_1\log s_1).
\]

Because \(s_1=\Theta(\sqrt{n/2})\), the new implementation remains in the same
asymptotic class as the baseline under a generic multiplication model. The
release therefore claims a strict local Pareto improvement, not
`T1=o(T0)` or `S1=o(S0)`.

## Deterministic Pareto ledger

`results/joint_wilson_operation_counts.csv` records, for a fixed increasing
input sequence:

- deterministic old/new work units;
- `T_new/T_old`;
- conservative old/new peak-live-coefficient bounds;
- `S_new/S_old`;
- old/new materialized coordinate counts;
- polynomial multiplications;
- schoolbook coefficient products;
- transform butterflies;
- exact reconstruction digits;
- monic remainders;
- Horner coefficient steps;
- old/new ring additions, multiplications, reductions and coefficient updates;
- old/new fixed-word limb products/additions;
- old/new temporary polynomial and big-integer counts;
- old/new allocation events and copied-byte upper bounds;
- old/new peak live limbs;
- state rewrite, full-factorial-materialization and feedback flags.

The release probe exits with failure unless all selected rows satisfy

\[
T_{new}/T_{old}<1,
\qquad
S_{new}/S_{old}<1,
\qquad
C_{new}/C_{old}<1.
\]

The continuous test interval independently checks the same strict
inequalities for every candidate in its frozen range.  It also requires strict
reductions in ring additions, ring multiplications, modular reductions,
coefficient updates, allocation count, and peak live limbs.

### Meaning of the peak counters

`peak_live_coefficients` is the exact peak number of coefficient slots owned by
the outer product/remainder schedule. `peak_live_coefficients_upper_bound`
adds a conservative, deterministic scratch allowance for the polynomial
engine, including exact transform and reconstruction storage. Both paths are
compared using the scratch-inclusive bound.

Each modular coefficient is one 64-bit word in the current public candidate
chart, so the scratch-inclusive coefficient bound is also reported as peak
live word limbs. No arbitrary-precision integer is created by the joint
Wilson path.

## Ring and bit complexity

The polynomial engine has two exact backends:

- schoolbook convolution for bounded coefficient products;
- multi-prime transform convolution followed by exact mixed-radix
  reconstruction for larger products.

The second backend does not reduce modulo the possibly composite target until
integer convolution coefficients have been reconstructed exactly. Monic
remainder evaluation therefore remains valid without field inverses.

The ledger distinguishes high-level deterministic events from fixed-word
arithmetic. It never promotes the native-operation count to a general bit
complexity theorem. Modulus-word multiplication uses a double-width exact
intermediate in the public 64-bit candidate chart.

## Exact arbitrary-precision replay

Let

\[
B=\operatorname{bit\_length}(n!)=\Theta(n\log n)
\]

and let the included owned integer use 32-bit limbs.

| Exact stage | Time | Live/result space |
|---|---:|---:|
| sequential derivation | `Theta(n^2 log n / 32)` limb updates in the current implementation | `Theta(B/32)` limbs |
| independent product tree with schoolbook multiplication | `O((B/32)^2)` limb-product accumulations | `O(B/32)` live limbs |
| decimal Download | repeated exact small division | `Theta(B)` output bits |
| exact Wilson remainder after materialization | `Theta(B/32)` limb scans | `O(1)` additional words |

The strict exact path runs two independent factorial derivations and compares
them before certification. This path is not part of the optimized modular
headline. Any complete binary or decimal Download of `n!` has unconditional
`Omega(B)` output time and space.

## Cyclic boundary pipeline

For fixed-width values, normalization, upload, presentation transport, native
quotient, same-frame continuation, order closure, checkpoint, and order
Download each use constant state-sized work. In a variable-width model,
copying and validating an order requires at least its input bit length.

The primitive closure evaluator is a dense reference audit. It applies `c*r`
factors and updates two `(J+1) x r` tensors:

```text
time  = O(c * r^2 * J^2) modular updates
space = O(r * J) modular coefficients
```

At `J=c`, this is `O(c^3 r^2)` time and `O(cr)` coefficient space.

## Factorial-derived cyclic structure

For order `r`, the primitive-period reference evaluator performs exactly
`r(r-1)` modular coefficient updates and keeps two `r`-coefficient vectors.
Optional normalized action, valuation reattachment and reference kernel
calculation remain quadratic fixed-word audits. A compact descriptor does not
change those execution costs.

## Public wrapper overhead

Each opaque handle uses one shared model allocation. Copying a handle copies a
`shared_ptr`; it does not copy, merge or compress the native state. Exceptions
are used for invalid stage transitions and resource-policy violations.

## Current boundary

The release has not eliminated the square-root external coordinate and does
not claim a polylogarithmic Wilson consumer. It has established an executable
joint local Pareto improvement while preserving:

```text
native state nodes rewritten = 0
ordinary feedback            = 0
full n! materialized         = 0
```

`results/complexity_probe.txt` contains object sizes, native steps, state
payload, legacy external coordinate size, new deterministic work and peak
space, exact factorial ledgers, and non-normative local wall-clock samples.
