# Exact Factorial Runtime Proof

## Coordinate contract

The accepted native result has canonical factoradic form `(r,c)` with

```text
r = principal target = principal valuation = candidate - 1
c = 1.
```

The factorial-place denotation of a canonical digit is

```text
D(r,c) = c * r!.
```

Thus the accepted coordinate denotes `r!`. SDK 1.1.0 does not stop at this
declarative equality; it evaluates the denotation into an arbitrary-precision
carrier.

## Sequential evaluator

Initialize `P_1=1`. For each `k=2..r`, compute `P_k=P_(k-1)*k` with exact limb
arithmetic. By induction, `P_k=k!`; therefore the final value is `r!`.

Each limb update uses an unsigned 128-bit accumulator for a 32-bit limb times a
64-bit factor plus carry. The accumulator range is sufficient, and every lower
32-bit digit and carry is retained. No fixed-width truncation is permitted.

## Independent product tree

For an interval `[a,b]`, the second evaluator returns:

```text
1                              if a>b
a                              if a=b
Product(a,middle)*Product(middle+1,b) otherwise.
```

Structural induction on interval length proves that it equals the exact
integer product of all elements of `[a,b]`. Invoked on `[2,r]`, it therefore
equals `r!`.

General big-integer multiplication uses schoolbook limb convolution with a
128-bit accumulator and carry propagation. The implementation retains every
result limb.

## Runtime acceptance

An exact value is returned only if:

```text
native request/result binding valid
AND sequential value == product-tree value
AND exact value seal binds the native view, argument, value hash, and ledgers.
```

The decimal/hexadecimal download replays the value seal. The exact Wilson
observer computes the modulus directly from the retained arbitrary-precision
value, while the optimized observer uses the same certified rank as its factor
count. Tests compare both residues.

## Scope

This proves correctness of the implemented runtime denotation for every input
admitted by the explicit exact-factorial resource policy, subject to ordinary
C++ implementation correctness. It is not a formal proof inside a theorem
prover and does not reduce the output-size lower bound of a fully materialized
factorial.
