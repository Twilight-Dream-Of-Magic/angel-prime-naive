# Release Manifest

## Release identity

```text
package                  Angel Causal Boundary Modular SDK 1.2.0
compatibility header     preserved SDK 1.1.0 public header
result classification    strict local joint Pareto improvement
language                  C++20
network dependency        none
```

## Required final answers

| Question | Answer |
|---|---|
| Old time complexity | `O(M(sqrt(n)) log n + sqrt(n))` |
| New time complexity | `O(M(sqrt(n/2)) log n + sqrt(n/2)) + O(1)` |
| Old space complexity | `O(sqrt(n) log n)` live coefficients |
| New space complexity | `O(sqrt(n/2) log n)` live coefficients |
| Time asymptotically improved | **No** |
| Space asymptotically improved | **No** |
| Both improved in the same implementation | **Yes** |
| Square-root coordinate eliminated | **No** |
| Full `n!` materialized in optimized Wilson path | **No** |
| Wilson consumed certified native factorial coordinate | **Yes** |
| Angel state changed | **No** |

The release is therefore Result B: both deterministic time work and peak space
strictly fall in the same executable implementation without worsening either
asymptotic class.

## New algorithm and implementation

- `include/angel/high_dimensional.hpp`
- `include/angel/high_dimensional/arithmetic.hpp`
- `include/angel/high_dimensional/functor.hpp`
- `include/angel/high_dimensional/class_quantum.hpp`
- `src/high_dimensional.cpp`
- `tests/high_dimensional_tests.cpp`
- `examples/high_dimensional_pipeline.cpp`
- `negative_compile/high_dimensional_observation_cannot_reenter.cpp`
- `include/angel/joint_wilson.hpp`
- `include/angel/release.hpp`
- `src/joint_wilson.cpp`
- `src/internal/joint_wilson_runtime.hpp`
- `include/angel/observation_integrity.hpp`
- `src/observation_integrity.cpp`
- `examples/joint_wilson_pipeline.cpp`
- `include/angel/factorial_maze.hpp`
- `src/factorial_maze.cpp`
- `tests/factorial_maze_tests.cpp`
- `examples/factorial_maze_pipeline.cpp`
- `negative_compile/factorial_maze_download_cannot_reenter.cpp`
- `negative_compile/factorial_maze_upload_cannot_skip_execute.cpp`
- `experiments/angel_structure_probe.cpp`

The exact complement-pairing identity reduces the consumed factorial range
from `n` to `floor(n/2)`.  The same implementation also performs bounded
near-square schedule selection, direct constant-mode projection, streamed
scalar accumulation, destructive branch retirement, leading-coefficient
Horner evaluation, and degree-one leaf remainder elimination.

The additive high-dimensional layer supplies the finite exact three-class value
`Ordinary + History + Singular`, checked HADD/HSUB/HMUL, central-scalar HDIV
with a reconstruction certificate, complete maze-state metadata, native functor
identity/composition, terminal non-authoritative observation, exact
superposition/interference/cancellation, and a nonzero-minor entanglement
certificate. Noncentral generalized division, coefficient overflow, and
unsupported exact execution remain typed continuations.

The factorial-maze bridge supplies an executable special functor from the
certified principal-jet factorial coordinate to a complete Ord/Hist/Sing maze
state. Opaque stage types enforce specification--upload--execute--observation
order, and the bridge certificate binds argument, compact program, native
state, request and result without changing the sealed core.

## Correctness and comparison

- `tests/joint_wilson_tests.cpp`
- `tests/joint_wilson_operation_probe.cpp`
- `tests/polynomial_ring_tests.cpp`
- `tests/high_dimensional_tests.cpp`
- `tests/factorial_maze_tests.cpp`
- `results/angel_structure_wilson_probe.csv`
- `results/angel_structure_cyclic_shadow_probe.csv`
- `results/angel_structure_period_rank_probe.csv`
- `results/angel_structure_higher_jet_probe.csv`
- `results/ANGEL_STRUCTURE_PROBE_SUMMARY.txt`
- `results/VOLUME_V_V5_9_VERIFIER_COMPLETE.log`
- `results/joint_wilson_operation_counts_annals_revision.csv`
- `results/JOINT_TIME_SPACE_LEDGER.md`
- `results/joint_time_space_summary.json`
- `results/joint_wilson_tests_debug.txt`
- `results/joint_wilson_tests_optimized.txt`
- `results/joint_wilson_tests_sanitized.txt`

Continuous strict Pareto audit: candidates `2..2048` (2,047 inputs).
Public-path differential audit: candidates `2..1024`, structured squares,
prime powers, semiprimes, multi-factor composites and block boundaries.
Independent exact factorial fixtures: `0!`, `1!`, `20!`, `100!`, `1000!`.

## Peak-memory evidence

The CSV and Markdown ledger report both the exact outer live-coefficient peak
and a conservative scratch-inclusive peak.  On the fixed increasing sequence:

```text
S_new/S_old min = 0.19169256
S_new/S_old max = 0.52803056
```

No point array or block-value array is materialized by the new path.

## Compatibility and integrity

- `manifest/frozen_source.sha256`
- `manifest/legacy_public_headers.sha256`
- `manifest/sdk_1_1_public_headers.sha256`
- `results/frozen_source_verification.txt`
- `results/api_compatibility.txt`
- `results/FINAL_AXIOMATIC_NEGATIVE_COMPILE_SUMMARY.txt`
- `results/public_name_audit.txt`

```text
native state nodes rewritten = 0
ordinary feedback            = 0
silent fixed-width truncation= 0
full factorial materialized  = 0
```

The public Wilson modulus chart remains the supplied `uint64_t` chart and
rejects unsupported resource ranges rather than truncating them.  The separate
exact-factorial path retains arbitrary-precision integer output.  This release
does not add an arbitrary-precision Wilson modulus API.

## Sanitizers and reproducibility

- `build_and_test.sh`
- `CMakeLists.txt`
- `results/BUILD_AND_TEST_COMPLETE.log`
- `results/AXIOMATIC_HIGH_DIMENSIONAL_BUILD_AND_TEST_COMPLETE.log`
- `results/ANNALS_REVISION_BUILD_AND_TEST_COMPLETE.log`
- `results/cmake_configure.txt`
- `results/cmake_build.txt`
- `results/cmake_ctest.txt`

AddressSanitizer and UndefinedBehaviorSanitizer pass.  LeakSanitizer is skipped
only because the execution environment is ptrace-restricted; that scope is
recorded in the build log.  The CMake frontend is unavailable in the current
verification environment; the portable canonical build script is the passing
release gate.

## Updated paper

- `paper/Angel_Arithmetic_Executable_Naive_Prime_Program_Joint_Time_Space_2026-08-26.tex`
- `paper/Angel_Arithmetic_Executable_Naive_Prime_Program_Joint_Time_Space_2026-08-26.pdf`
- `paper/Angel_SDK_1.2.0_Engineering_Audit_2026-08-26.tex`
- `paper/Angel_SDK_1.2.0_Engineering_Audit_2026-08-26.pdf`
- `paper/appendix_deferred_proofs.tex`
- `paper/appendix_evidence_and_audit.tex`
- `paper/angel_axioms_and_arithmetic.tex`
- `paper/appendix_axiom_provenance.tex`
- `paper/foundational_motivation_and_dimension.tex`
- `paper/factorial_maze_bridge.tex`
- `paper/reader_examples_and_related_work.tex`
- `paper/angel_prime_architecture_and_barriers.tex`
- `paper/volume_v_transmission_extension.tex`
- `paper/angel_full_space_and_shadow_theory.tex`
- `paper/wilson_structural_experiments.tex`
- `paper/source_material/Angel_Prime_Mathematical_Annals_Integrated_Volumes_I_IV.tex`
- `paper/source_material/Angel_Prime_Mathematical_Annals_Volume_V_v5.9_Strict_Research_Package.zip`
- `paper/expository_expansion.tex`
- `paper/wilson_worked_examples.tex`
- `paper/cyclic_worked_examples.tex`
- `paper/build_paper.sh`
- `paper/PAPER_BUILD_AND_VISUAL_AUDIT.md`
- `paper/MANUSCRIPT_MAINTENANCE_MAP.md`
- `docs/high_dimensional_arithmetic.md`
- `output/pdf/Causal_State_Factorial_Arithmetic_Annals_2026-08-26.pdf`
- `output/pdf/Angel_SDK_1.2.0_Engineering_Audit_2026-08-26.pdf`
- `manifest/annals_revision_files.sha256`

The final manuscript is 162 A4 pages. The body defines Angel primality
certificates, Angel Prime states and native Angel Prime transitions; explains
why the axioms are needed and why Ord/Hist/Sing are semantic sectors rather
than Euclidean dimensions; records the SN--OFCS precursor and proves typed
zero-division conservativity; gives HADD/HSUB/HMUL/HDIV, ordered interval-fold
and contraction functors, the factorial-maze bridge, running examples, Wilson
and cyclic charts, the prime fold--shadow collision, the Volume V
residual-horizon line, and a post-main-line procyclic/full-atlas theory with
4,961 structural probe records. Full proofs of the difficult model,
arithmetic, functor, spectral, valuation, barrier, source-child, stability,
zipper and transport theorems are collected in technical appendices. All finite
operation counts, differential campaigns, state-integrity evidence, toolchain
status and audit findings appear only in evidence appendices.

The separate color engineering-audit memorandum records the algorithmic
assessment, the twelve material repairs, the claim boundary and the corrected
deterministic sequence.  Its executable evidence is likewise confined to its
appendices.

## Final delivery audit

- `results/FINAL_DELIVERY_AUDIT.md`

This file gives the compact, item-by-item final answers, deterministic Pareto
ranges, toolchain status, state-integrity invariants, and the exact scope of the
release claim.
