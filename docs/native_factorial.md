# Native Factorial Semantics and Consumption

## Certified coordinate

The native arithmetic result is a self-contained factorial coordinate:

```text
rank = n
coefficient = 1
denotation = n!
```

The public native view is issued only after independent replay verifies the
request binding, principal-jet state, source program, result coordinate and
certificate. The view does not materialize the ordinary integer.

## Consumption contract

Every modular consumer in this SDK must obtain the factor count from the
verified native rank. The candidate is used only as:

1. the modulus of the ordinary observation ring;
2. an independent consistency constraint `candidate = rank + 1`.

Reconstructing the factor count directly from `candidate - 1` inside a
consumer is forbidden even though the diagonal request contract makes the two
values equal.

The legacy public Wilson API and both explicit native consumers obey this
contract.

## Joint modular consumer

The additive joint consumer applies the exact identity, for
`m = n + 1` and `h = floor(n/2)`,

\[
n!\equiv
\begin{cases}
(-1)^h(h!)^2 & n\text{ even},\\
(-1)^h(h!)^2(h+1) & n\text{ odd}
\end{cases}
\pmod m.
\]

It therefore asks the composite-safe polynomial engine to evaluate only
`h! mod m`. Evaluation points are generated at leaves, block values are
streamed into a scalar accumulator, and completed remainder branches are
released immediately. The full denotation `n!` is never constructed.

The result evidence records:

- native-coordinate verification;
- factor-count source;
- candidate-use restriction;
- complement-pairing theorem use;
- streamed scalar projection;
- no full factorial materialization;
- no ordinary feedback;
- no native state rewrite.

## Runtime exact derivation

`derive_exact_factorial()` is a separate bounded path. It interprets the
verified coordinate in an owned arbitrary-precision integer carrier using a
dynamic vector of 32-bit limbs.

Two implementations run independently:

1. sequential small-factor multiplication;
2. a balanced product tree with general limb multiplication.

They must agree bit-for-bit before an exact value is certified. The value
remains an external object until decimal or hexadecimal Download.

The regression suite verifies `0!`, `1!`, `20!`, `100!` and `1000!`. It also
compares the old public Wilson path, the original native modular path, the new
joint modular path, and an independent exact factorial remainder.

## Causal boundary

`NativeFactorialView`, the joint Wilson Download, `ExactFactorialValue`, and
ordinary integer output have no conversion back to `FactorialState`.
Negative compilation tests enforce:

- ordinary candidate cannot call the joint projection directly;
- factorial state cannot skip native-coordinate binding;
- joint Wilson Download cannot become native state;
- exact factorial Download cannot become native state.

The exact big integer and modular residues are external denotations, not new
Angel states. No frozen arithmetic source or state type is modified.
