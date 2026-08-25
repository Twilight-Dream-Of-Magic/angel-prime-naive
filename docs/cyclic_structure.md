# Cyclic Structure Pipeline

## Exact mathematical contract

For cyclic shift `S` of order `m`, define

```text
[k]_S = I + S + ... + S^(k-1)
J_m   = (-1)^(m-1) product(k=1..m-1) [k]_S
K_m   = (S-I)^(m-1) J_m
      = product(k=1..m-1) (I-S^k).
```

The retained factorial program evaluates to `J_m` in the self-cyclic frame.
If `V_d` is the exact-period-`d` character sector, then

```text
ker(J_m) = direct_sum(V_d : d divides m and 1 < d < m)
dim ker(J_m) = m - 1 - phi(m).
```

Therefore, for `m >= 2`,

```text
m is prime  <=>  ker(J_m) is zero.
```

Reattaching the valuation gives

```text
K_m = m * projector_onto_exact_period_m.
```

The first cyclic column of `K_m` is the Ramanujan response. At prime order it
is `(m-1,-1,...,-1)`, the complete-graph Laplacian column.

## Typed execution boundary

```text
FactorialState
  | bind_cyclic_action
CyclicActionView
  | evaluate_cyclic_action
ClosedCyclicAction
  | download_cyclic_structure
CyclicStructureDownload
```

The view contains seven binding words and no arithmetic state. It is not
convertible to `FactorialState`. The closed action exposes only its closure
certificate. Download performs an independent replay and produces an ordinary,
non-resumable result.

## Verification layers

The response is accepted only when both layers pass:

1. Request, state, program, valuation, view, response seal, and no-feedback
   ledger agree.
2. A separate implementation derives all Ramanujan coefficients. When the
   normalized action is requested, it also replays valuation reattachment,
   the constant-mode factorial identity, and the kernel-dimension theorem.

The evaluator does not call the independent divisor/Ramanujan implementation
to construct its output.

## Scope

This is an exact external reference observer. It supplies a reliable
structural primality property for a given order. It does not implement a
sublinear cyclic compiler, next-prime generator, or native prime transition.
