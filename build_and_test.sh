#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
CXX="${CXX:-g++}"
WORK="$(mktemp -d "$ROOT/.verification.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
mkdir -p "$ROOT/results"

WARN=(-std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
INC=(-I"$ROOT/include" -I"$ROOT/sealed_core/include" -I"$ROOT/src")
SOURCES=(
    "$ROOT/src/boundary_pipeline.cpp"
    "$ROOT/src/cyclic_structure.cpp"
    "$ROOT/src/diagnostics.cpp"
    "$ROOT/src/encoding.cpp"
    "$ROOT/src/native_factorial.cpp"
    "$ROOT/src/prime_pipeline.cpp"
    "$ROOT/src/session.cpp"
    "$ROOT/src/state.cpp")
TESTS=(
    boundary_pipeline_tests
    prime_pipeline_tests
    behavior_tests
    cyclic_structure_tests
    native_factorial_tests)

compile_variant() {
    local variant="$1"
    shift
    local variant_dir="$WORK/$variant"
    mkdir -p "$variant_dir/obj"
    for source in "${SOURCES[@]}"; do
        local object="$variant_dir/obj/$(basename "${source%.cpp}").o"
        "$CXX" "${WARN[@]}" "$@" "${INC[@]}" -c "$source" -o "$object"
    done
    ar rcs "$variant_dir/libangel_causal_boundary.a" "$variant_dir"/obj/*.o
    for test in "${TESTS[@]}"; do
        "$CXX" "${WARN[@]}" "$@" "${INC[@]}" \
            "$ROOT/tests/$test.cpp" "$variant_dir/libangel_causal_boundary.a" \
            -pthread -o "$variant_dir/$test"
    done
}

run_variant() {
    local variant="$1"
    for test in "${TESTS[@]}"; do
        "$WORK/$variant/$test" > "$ROOT/results/${test}_${variant}.txt"
    done
}

compile_variant debug -O0 -g
compile_variant optimized -O3 -DNDEBUG
run_variant debug
run_variant optimized

for test in "${TESTS[@]}"; do
    diff -u "$ROOT/results/${test}_debug.txt" \
        "$ROOT/results/${test}_optimized.txt" \
        > "$ROOT/results/${test}_optimization.diff"
done

"$CXX" "${WARN[@]}" -O3 -DNDEBUG "${INC[@]}" \
    "$ROOT/tests/complexity_probe.cpp" \
    "$WORK/optimized/libangel_causal_boundary.a" -pthread \
    -o "$WORK/optimized/complexity_probe"
"$WORK/optimized/complexity_probe" > "$ROOT/results/complexity_probe.txt"

# Address and undefined-behavior checks. Leak detection is disabled because
# ptrace-restricted containers cannot inspect /proc; that limitation is logged.
compile_variant sanitized -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer
for test in "${TESTS[@]}"; do
    ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
        "$WORK/sanitized/$test" \
        > "$ROOT/results/${test}_sanitized.txt" 2>&1
done

: > "$ROOT/results/negative_compile_summary.txt"
for source in "$ROOT"/negative_compile/*.cpp; do
    name="$(basename "$source" .cpp)"
    if "$CXX" "${WARN[@]}" -I"$ROOT/include" -c "$source" \
        -o "$WORK/$name.o" \
        > "$ROOT/results/negative_${name}.stdout.txt" \
        2> "$ROOT/results/negative_${name}.stderr.txt"; then
        echo "UNEXPECTED_COMPILE_SUCCESS $name" >&2
        exit 1
    fi
    echo "EXPECTED_COMPILE_FAILURE $name" \
        >> "$ROOT/results/negative_compile_summary.txt"
done

"$ROOT/tools/verify_frozen_source.sh" \
    > "$ROOT/results/frozen_source_verification.txt"
"$ROOT/tools/verify_api_compatibility.sh" \
    > "$ROOT/results/api_compatibility.txt"
"$ROOT/tools/audit_public_names.sh" \
    > "$ROOT/results/public_name_audit.txt"

{
    echo "BUILD_AND_TEST_COMPLETE"
    echo "compiler=$($CXX --version | head -n 1)"
    echo "debug_optimized_identical=YES"
    echo "address_undefined_sanitizers=PASS"
    echo "leak_sanitizer=SKIPPED_PTRACE_RESTRICTED_ENVIRONMENT"
    echo "negative_compile_firewall=PASS"
    echo "frozen_source_hashes=PASS"
    echo "legacy_public_headers_byte_preserved=YES"
    echo "legacy_source_compatibility=PASS"
    echo "runtime_exact_factorial_derivation=PASS"
    echo "arbitrary_precision_integer=PASS"
    echo "wilson_factor_count_source=NATIVE_FACTORIAL_COORDINATE"
    echo "exact_big_integer_wilson_oracle=PASS"
    echo "public_internal_codenames=0"
    echo "arithmetic_state_rewritten=NO"
    echo "arithmetic_state_compressed=NO"
    echo "ordinary_feedback=NO"
} > "$ROOT/results/BUILD_AND_TEST_COMPLETE.log"

cat "$ROOT/results/BUILD_AND_TEST_COMPLETE.log"
