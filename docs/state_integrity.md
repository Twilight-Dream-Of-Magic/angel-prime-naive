# State Integrity

## Frozen-state rule

The SDK does not rebuild a smaller state, serialize selected fields into a new
state, merge nodes, or reinterpret the external quotient descriptor as state.
Each opaque handle holds the exact historical C++ object behind a
`std::shared_ptr<const Model>`.

The pointer is an ownership and ABI boundary. Sharing that pointer when a
public handle is copied does not merge or compress any Angel node; the pointed
object remains unchanged and immutable.

## Enforced checks

`tools/verify_frozen_source.sh` checks all 13 frozen headers against
`manifest/frozen_source.sha256`. Any edit, including comments or whitespace,
fails verification.

The Wilson download returns `StateIntegrity`, which checks:

- request binding before equals after;
- state seal before equals after;
- source-program seal before equals after;
- certificate seal before equals after;
- payload byte count before equals after;
- rewritten node count is zero;
- merged node count is zero;
- state compression is false;
- ordinary feedback is false.

SDK 1.2.0 retains the binding of the Wilson factor count to the frozen
`AngelFactorialResult` rank and coefficient. The binding verifies the native
result certificate, the principal target/valuation diagonal, and the result
seal before modular work begins. `NativeWilsonEvidence` exposes those facts in
the additive API.

The joint Wilson path additionally records an exact before/after state identity, zero rewritten nodes, zero merged nodes, no full-factorial materialization, and no ordinary feedback. Its complement-paired and streamed schedule exists entirely on the Download side.

The exact factorial path derives an owned arbitrary-precision denotation twice
and requires bit-for-bit equality. Its `StateIntegrity` still refers to the
unchanged source state; the large integer is external evidence, not a
replacement state.

The cyclic-structure download uses the same integrity contract and adds
independent Ramanujan, valuation, constant-mode factorial, and kernel checks.

`angel::diagnostics::frozen_layout()` additionally reports the actual frozen
state object sizes and public handle sizes. A smaller handle is not asserted to
be a smaller state; it points to the complete state object.

## Naming exception forced by immutability

Public filenames, headers, classes, functions, examples, tests, binaries, and
logs use semantic names. Historical source contains historical namespaces.
Removing those tokens from the frozen source would change the source and C++
type identities, contradicting the no-core-change rule.

Therefore exactly one private adapter names the historical namespaces. The
automated public-name audit excludes only that adapter and the sealed source
snapshot. This is a containment boundary, not a claim that the old names were
edited out of immutable evidence.

## Causal cut

An ordinary boundary download contains a cut receipt and no native checkpoint.
Re-entry must call `upload` again, which allocates a fresh session and produces
the least-committed cycle multiplicity. Native continuation uses
`NativeCheckpoint`; ordinary observation and native checkpoint are deliberately
different types.

Exact factorial and Wilson observations are also terminal ordinary values.
Their types have no conversion or operator path back into native execution.

## Derived observation-map validation

`validate_download_packet` compares a transported or displayed download with
the observation already bound to the canonical closed state. It checks the
complete public observation, causal-cut identity, and terminal-only flags. The
comparison is one-way: the derived packet is never written back. A test mutates
a copied primitive column, requires rejection, and then verifies that the
canonical state summary is unchanged.
