# Limitations and Open Problems

- The joint Wilson implementation is a **local Pareto improvement**, not a
  proof of a lower asymptotic class. Both the legacy and new external paths
  remain in a square-root-coordinate family.
- The complement-pairing theorem reduces the polynomial problem from `n` to
  `floor(n/2)` and the schedule removes several stored arrays and live
  branches, but it does not eliminate the degree-`Theta(sqrt(n))` block
  polynomial.
- No `O(log n)` or general polylogarithmic external Wilson consumer is claimed.
- The current sufficient summary is not proved closed under every native
  doubling, shift, quotient projection and constant-mode observation. Finding
  such a summary remains the main coordinate-compression problem.
- Deterministic operation counts and peak-live-coefficient bounds are not
  substitutes for a universal bit-complexity theorem. They describe the exact
  implemented chart and its fixed-word polynomial lowering.
- The reported scratch-inclusive peak is a conservative deterministic upper
  bound. It is intentionally not reduced to a wall-clock or resident-set-size
  anecdote.
- The complete integer `n!` is not materialized by the joint modular path. The
  separate exact path does materialize it and therefore pays the unavoidable
  `Theta(n log n)` output size plus its current arbitrary-precision audit work.
- The cyclic boundary pipeline is a scoped typed protocol. Its dense closure
  and period evaluators remain reference audits with polynomial resource use;
  they are not new constant-time native instructions.
- Wilson observation for a supplied integer is a naive prime application. It
  is not a candidate-free next-prime transition or a complete native
  Prime-Birth compiler.
- The public candidate chart remains 64-bit. The SDK retains its independent
  arbitrary-precision exact factorial path, but this release does not widen the
  public Wilson modulus type.
- The public handles are source-stable abstractions, not a guaranteed binary
  ABI across unrelated standard-library implementations.
- LeakSanitizer may be unavailable in ptrace-restricted environments. Address
  and undefined-behaviour checks still run with leak detection disabled, and
  the release log records that scope explicitly.
