# Joint Time-Space Optimization of Wilson Consumption

## 1. Problem statement

The native arithmetic side produces a certified factorial coordinate

```text
rank = n
coefficient = 1
denotation = n!
```

without materializing the ordinary integer `n!`.  The Download-side Wilson
consumer must compute

\[
n! \bmod m,
\qquad m=n+1,
\]

while preserving the native state byte-for-byte.  The previous external
consumer used a block width

\[
s=\lceil\sqrt n\rceil
\]

and a product/remainder tree.  Its charged bounds were

\[
T_0(n)=O(M(s)\log s+s),
\qquad
S_0(n)=O(s\log s),
\]

with a materialized external coordinate of order \(\Theta(s)\).

The goal of this release is not to rename that representation.  The goal is a
single executable implementation in which deterministic work and peak live
space both decrease while neither asymptotic bound becomes worse.

## 2. Integral complement-pairing theorem

Let \(m\ge2\), \(n=m-1\), and \(h=\lfloor n/2\rfloor\).

### Theorem 2.1

For every integer modulus \(m\), prime or composite,

\[
n!\equiv
\begin{cases}
(-1)^h(h!)^2 & n\text{ even},\\[2mm]
(-1)^h(h!)^2(h+1) & n\text{ odd}
\end{cases}
\pmod m.
\]

### Proof

For every \(1\le k\le h\),

\[
k(m-k)\equiv-k^2\pmod m.
\]

If \(n\) is even, then \(m\) is odd and the factors
\(1,2,\ldots,m-1\) split into the disjoint pairs
\((k,m-k)\), \(1\le k\le h\).  Multiplying the pair congruences gives

\[
(m-1)!\equiv(-1)^h(h!)^2\pmod m.
\]

If \(n\) is odd, then \(m\) is even.  The same pairs cover every factor except
\(m/2=h+1\), which yields the second formula.  No cancellation or inverse is
used, so the proof is valid in the ring \(\mathbb Z/m\mathbb Z\) even when it
contains zero divisors. \(\square\)

This theorem is the source of the simultaneous reduction.  The polynomial
consumer is applied to \(h\), not to \(n\), and the original residue is
reconstructed with at most two scalar modular multiplications and one modular
negation.

## 3. Wilson consumption contract

The implementation accepts only a `NativeFactorialView`.  Before any ordinary
projection begins, it independently verifies:

1. the complete native request binding;
2. the factorial program and principal-jet certificate;
3. `coefficient = 1`;
4. the equality of native rank, jet valuation, and jet target;
5. `native rank = candidate - 1` as an independent boundary consistency check.

The factor count used by the algorithm is then loaded from the verified native
rank.  The candidate is retained only as the modulus and the consistency
constraint `candidate = rank + 1`.

The consumer does not expose an ascent operation.  Its result is an ordinary
observation and cannot be converted to a native state.

## 4. Reduced polynomial coordinate

For the reduced factor count \(h\), choose a block width \(w\) near
\(\sqrt h\) and define

\[
P_w(X)=\prod_{i=1}^{w}(X+i)\in(\mathbb Z/m\mathbb Z)[X].
\]

Writing

\[
h=qw+r,
\qquad 0\le r<w,
\]

gives

\[
h!\equiv
\left(\prod_{j=0}^{q-1}P_w(jw)\right)
\left(\prod_{k=qw+1}^{h}k\right)
\pmod m.
\]

The polynomial engine is composite-safe.  It uses monic remainders and exact
multi-prime convolution reconstruction; it never assumes that
\(\mathbb Z/m\mathbb Z\) is a field.

## 5. Bounded schedule selection

The width selector examines a fixed-radius neighbourhood of three integer
anchors:

\[
\lfloor\sqrt h\rfloor,
\qquad
\lceil\sqrt h\rceil,
\qquad
\left\lceil\frac{h}{\lfloor\sqrt h\rfloor}\right\rceil.
\]

For each candidate width it computes

\[
q=\lfloor h/w\rfloor,
\qquad
r=h\bmod w,
\qquad
C(w)=w+q+r.
\]

It minimizes lexicographically

\[
(C(w),\max(w,q),r,w).
\]

The search radius is policy-bounded and independent of the input magnitude.
It is not a hidden table, factorization, or candidate-specific precomputation.
The selector changes only the external evaluation schedule; it never mutates
the native factorial state.

## 6. Joint execution schedule

The new schedule contains six work-reducing and space-reducing changes in the
same implementation.

### 6.1 Half-size factorial problem

The complement theorem replaces the polynomial problem of size \(n\) by one
of size \(h=\lfloor n/2\rfloor\).

### 6.2 Direct zero-point projection

The first block value is

\[
P_w(0),
\]

which is exactly the constant coefficient of the already materialized block
polynomial.  The implementation reads that coefficient directly.  It does not
construct a zero evaluation leaf and does not run a Horner evaluation for it.

### 6.3 Streamed scalar accumulation

Evaluation points are generated from their block indices when a tree leaf is
created.  No point array is stored.  Each block value is immediately
multiplied into the scalar residue.  No block-value array is stored.

### 6.4 Destructive branch retirement

The remaining evaluation points are split into two top-level ranges.  Each
range is built, reduced, evaluated, and destroyed before the next range is
processed.  During remainder descent, the left remainder branch is completed
and released before the right remainder is created.  Completed product-tree
nodes are released immediately.

No polynomial is recomputed.  The schedule therefore removes product nodes
and simultaneous live remainders rather than exchanging additional time for
less space.

### 6.5 Leading-coefficient Horner evaluation

The legacy leaf evaluator initializes Horner's recurrence at zero and performs
one multiply-add step for every stored coefficient.  The optimized evaluator
initializes the recurrence with the leading coefficient and processes only the
remaining coefficients.  For every nonempty leaf polynomial this removes one
ring multiplication, one ring addition, one modular reduction, and one
coefficient update without changing the value.

### 6.6 Degree-one leaf elimination

Once the two degree-one leaf factors have been multiplied into their parent
product, those leaf polynomials are no longer retained.  During descent, an
already reduced parent polynomial is evaluated directly at each leaf point.
The optimized path therefore avoids the two monic remainders by degree-one
polynomials that the legacy materialized tree performs at every bottom-level
parent.  This simultaneously removes remainder work and shortens the live
coefficient frontier.  No factor, product, or remainder is reconstructed later.

## 7. Correctness theorem

### Theorem 7.1

Assume the native factorial coordinate verifies with rank \(n\), coefficient
one, and modulus consistency \(m=n+1\).  The jointly optimized consumer returns
\(n!\bmod m\).

### Proof

The product/remainder evaluation computes \(h!\bmod m\) exactly by the block
identity in Section 4.  The polynomial engine performs exact ring operations
for arbitrary composite modulus.  The reconstruction in Theorem 2.1 then
returns \(n!\bmod m\).  Every evaluation value is multiplied into the same
scalar accumulator exactly once, and every tail factor from \(qw+1\) through
\(h\) is multiplied exactly once.  Streaming changes neither the set nor the
order-independent product of ring elements. \(\square\)

## 8. Time complexity

Let

\[
s_0=\lceil\sqrt n\rceil,
\qquad
s_1=\Theta(\sqrt{n/2}).
\]

The old and new bounds are

\[
T_0(n)=O(M(s_0)\log s_0+s_0),
\]

\[
T_1(n)=O(M(s_1)\log s_1+s_1)+O(1).
\]

The bounded width search costs \(O(1)\) schedule evaluations.  Direct zero
projection removes one multipoint leaf.  Top-level branch splitting removes a
product-node multiplication.  Streaming adds no polynomial operation.
Leading-coefficient Horner evaluation removes one multiply-add-reduction step
per nonempty leaf evaluation, while degree-one leaf elimination removes the
bottom-level degree-one remainder calls and their temporary coefficients.

This is not an \(o(T_0)\) theorem under a generic multiplication model: both
paths remain in the same asymptotic class.  The release therefore records a
joint local optimization.  Deterministic operation counts, generated by the
same executable polynomial engine, are strictly lower on the frozen test
sequence and on the continuous audit interval.

## 9. Space complexity

The old tree retains a block polynomial, point array, value array, and all
product-tree node polynomials while the remainder tree runs.  Its charged
bound is

\[
S_0(n)=O(s_0\log s_0).
\]

The new path uses a half-size problem, stores neither point nor value arrays,
processes only one top-level range at a time, and retires completed branches.
Its bound is

\[
S_1(n)=O(s_1\log s_1).
\]

The implementation reports two memory counters:

- `peak_live_coefficients`: exact simultaneously live coefficient slots owned
  by the outer product/remainder schedule;
- `peak_live_coefficients_upper_bound`: that peak plus a conservative bound for
  exact polynomial-engine scratch, including transform and reconstruction
  storage.

Both are ordinary Download-side costs.  Native state payload is unchanged.

## 10. Bit complexity

Each modular coefficient occupies one 64-bit machine word in the current
public candidate chart.  The ledger separately records polynomial events,
modular operations, 64-bit limb products/additions, allocation events, copied
bytes, peak coefficient slots, and peak word limbs.

The full exact integer path is separate.  If

\[
B=\operatorname{bits}(n!)=\Theta(n\log n),
\]

then downloading all of \(n!\) requires at least \(\Omega(B)\) output time and
space.  The jointly optimized Wilson path does not materialize that integer,
so the output-size lower bound does not apply to it.

## 11. Pareto criterion

The release classifies success as a local joint improvement:

\[
T_1(n)\le T_0(n),
\qquad
S_1(n)\le S_0(n),
\]

with strict deterministic reductions over the frozen test domain.  It does
not classify the result as a joint asymptotic improvement, because the
square-root coordinate remains.

The generated CSV reports

\[
T_1/T_0,
\qquad
S_1/S_0,
\qquad
C_1/C_0
\]

for a growing input sequence.  A release test fails if any selected row does
not satisfy all three strict inequalities.

The old path is not represented by a formula-only estimate.  A separate
instrumented replay executes the supplied materialized-point/materialized-value
schedule through the same polynomial engine and counter conventions as the new
path.  Consequently the CSV contains old/new values for ring operations,
coefficient updates, limb work, temporary objects, allocations, copied-byte
upper bounds, peak space, and coordinate materialization.  On the continuous
audit interval, ring additions, ring multiplications, modular reductions,
coefficient updates, allocation count, and peak live limbs all decrease
strictly in the same implementation that lowers total work and peak space.

## 12. Rejected time-space exchanges

The implementation deliberately rejects:

- checkpoint/recompute schedules that save memory by repeating polynomial
  products or remainders;
- larger precomputed tables or cached point sets;
- storing all cyclic modes;
- replacing exact integral sources by rational or field relaxations;
- omitting verification or state-integrity checks;
- parallel branches that reduce wall time while increasing simultaneous live
  polynomials;
- short descriptors counted as completed execution;
- full factorial materialization followed by an ordinary remainder.

## 13. Remaining coordinate barrier

### 13.1 Location of summary growth

The compact source program composes by the shifted law

\[
F_{a+b}(Z,u)=F_a(Z,u)F_b(Zu^a,u).
\]

A scalar constant-mode value is not sufficient for this composition: after a
future shift, the consumer needs the section at new block origins, not only at
zero.  Retaining a fixed number of low coefficients is also not closed.  The
product step forms cross-coefficients whose supported degree is the sum of the
two input degrees, and exact quotient/remainder projection needs those
coefficients until the evaluation branch has been selected.  In the current
lowering, this is precisely the step

```text
shifted source composition
    -> block-polynomial convolution
    -> monic quotient/remainder projection
```

at which a proposed fixed-size summary grows.  Keeping all required
coefficients gives the degree-`Theta(sqrt(n))` block coordinate; keeping all
shifted scalar values gives the same number of evaluation coordinates.  The
new complement quotient halves the factorial range and the scheduler removes
redundant storage/work, but neither supplies a future-sufficient summary whose
length is closed under this shifted composition.

This is a statement about the present implementations and attempted summary
families, not a lower bound for every possible Angel-native consumer.

The block polynomial still has degree \(\Theta(\sqrt n)\), and the exact
multipoint machinery still has a square-root-scale sufficient coordinate.
Thus

```text
square-root coordinate eliminated = NO
joint asymptotic improvement       = NO
joint executable local Pareto gain = YES
```

The unresolved problem is a composition summary for the succinct native
factorial program that remains compact under doubling, shift, quotient
projection, and constant-mode observation without materializing a
square-root-scale polynomial or mode family.

## 14. State-integrity result

For every joint observation:

```text
native state nodes rewritten = 0
native state nodes merged    = 0
native state compressed      = false
ordinary feedback            = false
full n! materialized         = false
fixed-width truncation       = false
```

The Download result cannot be converted into a native state.  The frozen
arithmetic/state hashes and all legacy public-header hashes are verified by
the release script.
