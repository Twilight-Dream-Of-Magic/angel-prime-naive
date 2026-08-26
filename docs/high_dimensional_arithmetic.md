# Angel high-dimensional arithmetic and functors

The public `angel::high` layer is the finite exact SDK model of the paper's
three-class Angel maze object. It is additive: the frozen SDK 1.1 arithmetic and
legacy headers remain unchanged.

## Three arithmetic classes

`TriClassValue` has three jointly authoritative coordinates:

- **Ordinary**: a checked signed integer coefficient.
- **History**: a sparse sum of ordered words. Word order records causal order,
  so multiplication is generally noncommutative.
- **Singular**: a sparse two-sided ideal carrying unresolved or failed-inverse
  information without inventing an ordinary scalar answer.

Equality compares all three coordinates. `ordinary()` is only a projection; it
cannot reconstruct or authorize the full state.

## Four operations

For `X = (r,h,s)` and `Y = (r',h',s')`:

```text
HADD(X,Y) = (r+r', h+h', s+s')
HSUB(X,Y) = (r-r', h-h', s-s')
HMUL(X,Y) = (rr', rh'+r'h+hh', rs'+r's+hs'+sh'+ss')
```

`hadd`, `hsub`, and `hmul` use checked coefficients. Overflow returns an
`ArithmeticContinuation` retaining the operands; machine wraparound is never
reported as an exact result.

`hdiv(X,d)` is total for a central signed scalar `d`. It performs exact
coefficientwise quotient/remainder division and returns a `DivisionPacket`
satisfying:

```text
X = d * quotient + residual
```

For `d == 0`, the quotient is zero and the residual is exactly `X`. A
noncentral high-dimensional denominator is deliberately outside this finite
chart and yields a continuation.

This totalizes the typed division request, not the scalar inverse law. No value
`z` with `0 * z = X` is introduced: the exact reconstruction is
`X = 0 * quotient + residual`. The paper traces this convention to the
SN--OFCS precursor and proves that the ordinary ring embedding remains
conservative.

## Maze state and causal functors

`MazeState` binds the arithmetic value to room, road, frame, causal epoch,
history depth, holonomy, singular generation, higher cell, ordered events, and
a deterministic state seal.

`NativeFunctor` is a finite opcode program:

- the empty program is identity;
- `compose(first, second)` concatenates execution in causal order;
- every instruction maps a complete maze state to another complete state or a
  typed continuation;
- exact division refuses to continue as an exact state when a residual remains.

`UploadFunctor` constructs a new causal ingress from a known specification.
`DownloadFunctor` produces a derived, non-authoritative observation. The API has
no promotion function from an observation back into a state; that prohibited
direction is also checked by a negative-compilation test.

## Class-quantum history algebra

`ClassQuantumFunctor` is a finite integer-weighted family of deterministic
native functors. It models unresolved histories without claiming physical
quantum mechanics.

- **Superposition** retains each weighted branch and its complete path.
- **Interference** groups branches only when their full structural endpoints
  agree.
- **Cancellation** removes a group only when its exact integer weight is zero.
  Equal ordinary shadows alone cannot cancel.
- **Entanglement** for `TensorHistory` means coefficient-matrix rank greater
  than one over the rationals. A nonzero exact `2 x 2` minor is the certificate.
  Checked overflow yields an unresolved analysis, never a false certificate.

## Example

```cpp
#include "angel/high_dimensional.hpp"

using namespace angel::high;

auto state = UploadFunctor{}(OrdinarySpecification{6});
auto program = compose(
    NativeFunctor::add(TriClassValue::ordinary(2)),
    NativeFunctor::multiply(TriClassValue::ordinary(3)));

auto evolved = program(state);
auto observation = DownloadFunctor{}(std::get<MazeState>(evolved));
```

The complete formal definitions, relative model proof, difficult proofs, source
provenance, and finite verification matrix are in the annals manuscript and its
appendices.

## Specialized factorial maze functor

`angel::factorial_maze` is the implemented bridge between the paper's
factorial principal-jet object and the public SDK. Its states carry:

- ordinary coefficient `1`, matching the normalized factorial source;
- ordered history words binding argument, compact program, native state,
  request, result and certificate;
- a zero singular coordinate on successful execution, with failures retained
  by the underlying typed error/continuation paths;
- room, road, frame, holonomy, higher-cell and certificate address fields.

Only `UploadedState` accepts `execute()`, and only `ExecutedState` accepts
`project_jointly()` or `derive_exact()`. The two negative-compilation tests
prove that the public type graph has no skip-execute or terminal-reentry edge.
This is a specialized causal functor, not a claim that generic HMUL alone
computes the factorial principal jet.
