# Architecture

## Design result

The previous public surface exposed a static protocol class plus dozens of
cooperating certificate, boundary, verifier, observer, and view classes. The
new public surface separates values from behavior:

- Plain structs carry data, evidence, policies, and ledgers.
- Ten opaque value handles own complete implementation objects:
  `FactorialState`, `QuotientView`, `SessionAuthority`, `State`,
  `NativeCheckpoint`, `ClosedObservation`, `CyclicActionView`,
  `ClosedCyclicAction`, `NativeFactorialView`, and `ExactFactorialValue`.
- Free functions create operations.
- `operator|` applies an operation to the only legal preceding state type.
- Algorithms live in source modules rather than a god class.

The command structs are zero- or few-field values. They are not service
objects, do not own state, and do not create another object graph.

## Module boundaries

```text
ordinary cyclic encoding
    | upload
opaque cyclic state
    | quotient / continue / presentation change / native checkpoint
opaque cyclic state
    | close an observer
closed observation
    | download
ordinary observation + irreversible cut receipt
```

```text
ordinary candidate
    | upload factorial state
complete native factorial request and state
    | bind quotient view
external descriptor bound to exact state identity
    | download Wilson observation
ordinary residue/decision + before/after state-integrity evidence
```

```text
complete native factorial request and state
    | bind native factorial
immutable view of the certified native result coordinate
    | optimized modular projection
Wilson observation sourced from the native rank
```

```text
immutable native factorial view
    | derive exact factorial
opaque arbitrary-precision exact value, independently replayed
    | exact Wilson remainder OR ordinary integer download
ordinary non-resumable observation
```

```text
complete native factorial request and state
    | bind cyclic action
immutable external cyclic descriptor
    | evaluate and close
closed private response
    | independently verify and download
ordinary period structure + proof ledger
```

The boundary state and factorial-derived pipelines intentionally remain
distinct. The cyclic boundary state in
the R63 experiment is scoped and is not silently identified with the complete
R57 principal-jet state used by the R60 Wilson application.

The arbitrary-precision value is an external operational denotation of the
native factoradic result. It is deliberately not typed as an Angel state.

## Why operator overloading is safe here

Only `operator|` is overloaded, and only for explicit stage pairs. There is no
generic catch-all operator and no arithmetic operator overload on Angel state.
Consequently:

- a `DownloadPacket` cannot be piped into `quotient_to`;
- a prime `Download` cannot be treated as a `FactorialState`;
- an unclosed `State` cannot be passed to `download`;
- a native checkpoint remains a distinct type from an ordinary observation.
- a cyclic action view cannot become a factorial state;
- an exact factorial value cannot bind a native quotient view;
- an exact factorial download cannot become a factorial state.

Those properties are exercised in `negative_compile/`.

## Dependency direction

Public headers know only semantic public types and forward-declared PIMPL
models. Ordinary source modules know only semantic aliases. One private header,
`src/internal/frozen_types.hpp`, maps those aliases to the byte-preserved
historical namespaces. This makes the historical naming dependency explicit,
small, and non-transitive.

## Historical lineage

The immutable implementation is drawn from the complete R63 package and
therefore includes the R56–R62 and R23 headers on which it depends. The R60
external quotient view, R61 cyclic probe, and R62 germ implementation are the
same source revisions embedded in that package. Their exact hashes are listed
in `manifest/frozen_source.sha256`.
