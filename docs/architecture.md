# Architecture

## Design result

The SDK separates mathematical values, stage operations, ordinary observers,
and verification evidence.

- Plain structs carry policies, ledgers, observations and proof summaries.
- Opaque handles own complete implementation objects.
- Free functions create stage operations.
- `operator|` applies an operation only to its legal predecessor type.
- Algorithms live in source modules, not in a generic state wrapper.

The command structs are small operation descriptions. They do not own native
state and do not create a second authority path.

## Canonical causal boundary

```text
known ordinary input/specification
        |
        | Proxy Upload: the only public ascent
        v
complete native state
        |
        | actual native state-to-state transitions
        v
live native factorial state
        |
        | explicit Download boundary
        v
ordinary observation
```

There is no ordinary-observation-to-native feedback edge.

## Native factorial and Wilson paths

```text
ordinary candidate
    | upload factorial state
complete native factorial request and state
    | bind native factorial
immutable verified view of rank=n, coefficient=1
    | project Wilson jointly
ordinary residue/decision + detailed T-S ledger + state-integrity evidence
```

The jointly optimized consumer:

1. reads the factor count from the certified native rank;
2. checks `candidate = rank + 1` independently;
3. applies the integral complement-pairing theorem;
4. computes the reduced modular factorial through the composite-safe
   polynomial engine;
5. streams ordinary block observations into one scalar residue;
6. returns an irreversible ordinary result.

The candidate is not reused as a substitute factor count. The complete integer
`n!` is not materialized.

The legacy Wilson path remains source-compatible:

```text
complete native factorial state
    | bind external quotient view
state-bound external descriptor
    | download Wilson observation
ordinary residue/decision
```

## Exact arbitrary-precision path

```text
verified native factorial view
    | derive exact factorial
opaque arbitrary-precision exact value, independently replayed
    | exact Wilson remainder OR ordinary integer Download
ordinary non-resumable result
```

The arbitrary-precision value is an external denotation. It is deliberately
not typed as a native state and cannot re-enter native execution.

## Cyclic structure path

```text
complete native factorial state
    | bind cyclic action
immutable external cyclic descriptor
    | evaluate and close reference action
closed response
    | independent verification and Download
ordinary period structure + ledger
```

The cyclic reference evaluator and the native factorial state are related by
proved projections, but they remain different typed pipelines.

## Why operator overloading is safe here

Only explicit stage-pair overloads exist. There is no generic catch-all
operator and no ordinary arithmetic overload on native state. Therefore:

- an ordinary Download cannot continue native execution;
- a Wilson observation cannot become a factorial state;
- a factorial state cannot skip the verified native-coordinate binding before
  the joint consumer;
- an ordinary candidate cannot call the joint consumer directly;
- an exact factorial Download cannot become native state;
- a cyclic observation cannot become native state.

These properties are compiled as negative tests.

## Frozen-source boundary

The arithmetic/state implementation under `sealed_core/` is immutable. New
functionality is an additive source module that reads a certified view and
runs after the explicit observation boundary. Release verification checks all
frozen source hashes before accepting the package.

Legacy public headers are also byte-preserved. The new semantic header is
`include/angel/joint_wilson.hpp`; existing source clients need no changes.

## State-integrity rule

The joint consumer records the complete state identity before and after the
observation. Acceptance requires:

```text
request binding unchanged
state seal unchanged
source-program seal unchanged
certificate seal unchanged
payload bytes unchanged
native nodes rewritten = 0
native nodes merged = 0
ordinary feedback = false
```

The optimization therefore changes only Download-side evaluation work and
space. It does not compress, reinterpret or mutate the native arithmetic
object.
