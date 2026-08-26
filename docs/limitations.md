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
- The public factorial-maze bridge realizes Angel primality certificates for
  supplied candidates. It does not claim to generate the next candidate
  natively.
- Volume V v5.9 residual stability, no-recomputation and zipper properties are
  research-verifier results on declared finite rooms, not public SDK APIs.
- The residual-horizon theorem supports an amortized warm-transition shadow
  only after source packets have been paid for. A descriptor-native cold
  `p^(o(1))` source morphism remains open.
- The procyclic completion is rigorous for fixed prefixes. The diagonal
  self-room family `K_m=P_(m,m-1)` is a dependent section, not one compatible
  inverse-limit element.
- All-chart reconstruction and an integral generalized HDIV that glues across
  rooms/jets are conjectures. Finite structural probes do not prove them.
- Fold multiplication supplies an ordered interval tree and compositional jet
  signatures, but a product tree is not by itself a new dimension. The cyclic
  row has `p` coordinates while `(p-1)!` has `p-1` factors; their proved meeting
  is a terminal congruence, not a leaf bijection or an inverse reconstruction
  of the maze.
- Higher jet coefficients do not determine a fold tree without separate
  History/Program data, and a tree does not determine higher jets without the
  local factor series.
- The public candidate chart remains 64-bit. The SDK retains its independent
  arbitrary-precision exact factorial path, but this release does not widen the
  public Wilson modulus type.
- The public handles are source-stable abstractions, not a guaranteed binary
  ABI across unrelated standard-library implementations.
- LeakSanitizer may be unavailable in ptrace-restricted environments. Address
  and undefined-behaviour checks still run with leak detection disabled, and
  the release log records that scope explicitly.
