#include "angel/joint_wilson.hpp"
#include "internal/big_unsigned.hpp"
#include "internal/joint_wilson_runtime.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace {

#define REQUIRE(condition) require((condition), #condition)

void require(const bool condition, const char* expression) {
    if (!condition)
        throw std::runtime_error(std::string{"requirement failed: "} + expression);
}


[[nodiscard]] std::uint64_t direct_factorial_mod(
    const std::uint64_t argument,
    const std::uint64_t modulus) noexcept {
    std::uint64_t residue = 1U % modulus;
    for (std::uint64_t factor = 1U; factor <= argument; ++factor) {
        residue = angel::detail::frozen::wilson::multiply_mod(
            residue, factor % modulus, modulus);
    }
    return residue;
}

[[nodiscard]] const angel::prime::WilsonObservation& wilson_observation(
    const angel::prime::OrdinaryObservation& observation) {
    if (!std::holds_alternative<angel::prime::WilsonObservation>(observation))
        throw std::runtime_error("unexpected resource limit");
    return std::get<angel::prime::WilsonObservation>(observation);
}

void verify_exact_factorial_fixture(const std::uint64_t argument) {
    angel::detail::BigUnsignedLedger exact_ledger{};
    const auto exact = angel::detail::factorial_sequential(
        argument, exact_ledger);
    const auto modulus = argument + 1U;
    REQUIRE(exact.modulo(modulus) == direct_factorial_mod(argument, modulus));

    if (argument == 0U) {
        REQUIRE(exact.decimal() == "1");
        return;
    }

    using namespace angel::prime;
    const auto state = candidate(modulus) | upload_factorial_state();
    const auto native = state | bind_native_factorial();
    const auto materialized = native
        | derive_exact_factorial(ExactFactorialPolicy{argument})
        | download_exact_factorial();
    REQUIRE(materialized.verification_passed);
    REQUIRE(materialized.factorial_argument == argument);
    REQUIRE(materialized.decimal == exact.decimal());
}

void compare_all_paths(const std::uint64_t candidate_value) {
    using namespace angel::prime;
    const auto state = candidate(candidate_value) | upload_factorial_state();
    const auto before = state.summary();
    const auto native = state | bind_native_factorial();

    ObservationPolicy old_policy{};
    old_policy.maximum_block_width = 100'000U;
    old_policy.parallel_tree_branches = false;
    const auto old_public = state
        | bind_quotient_view()
        | download_wilson(old_policy);
    const auto old_native = native
        | project_wilson_from_native(old_policy);

    JointWilsonPolicy joint_policy{};
    joint_policy.maximum_block_width = 100'000U;
    joint_policy.parallel_ntt_primes = false;
    const auto optimized = native | project_wilson_jointly(joint_policy);

    REQUIRE(old_public.verification_passed);
    REQUIRE(old_native.verified());
    REQUIRE(optimized.verified());
    const auto& public_value = wilson_observation(old_public.observation);
    const auto& native_value = wilson_observation(old_native.observation);
    const auto& optimized_value = wilson_observation(optimized.observation);
    const auto exact = direct_factorial_mod(
        candidate_value - 1U, candidate_value);

    REQUIRE(public_value.factorial_residue == exact);
    REQUIRE(native_value.factorial_residue == exact);
    REQUIRE(optimized_value.factorial_residue == exact);
    REQUIRE(public_value.prime == native_value.prime);
    REQUIRE(native_value.prime == optimized_value.prime);
    REQUIRE(optimized.evidence.factor_count_loaded_from_native_coordinate);
    REQUIRE(optimized.evidence.candidate_used_only_as_modulus_and_consistency_check);
    REQUIRE(optimized.evidence.complement_pairing_identity_verified);
    REQUIRE(optimized.evidence.streamed_scalar_projection);
    REQUIRE(!optimized.evidence.square_root_coordinate_eliminated);
    REQUIRE(!optimized.evidence.full_factorial_materialized);
    REQUIRE(!optimized.evidence.ordinary_feedback);
    REQUIRE(optimized.ledger.native_state_nodes_rewritten == 0U);
    REQUIRE(!optimized.ledger.ordinary_feedback);
    REQUIRE(!optimized.ledger.full_factorial_materialized);
    REQUIRE(optimized.ledger.exact_counters_checked);
    REQUIRE(optimized.coordinate.materialized_evaluation_point_array == 0U);
    REQUIRE(optimized.coordinate.materialized_block_value_array == 0U);
    REQUIRE(optimized.coordinate.materialized_coordinate_count <
           optimized.coordinate.legacy_materialized_coordinate_count);

    const auto after = state.summary();
    REQUIRE(before.request_binding == after.request_binding);
    REQUIRE(before.state_seal == after.state_seal);
    REQUIRE(before.program_seal == after.program_seal);
    REQUIRE(before.payload_bytes == after.payload_bytes);
    REQUIRE(optimized.integrity.preserved());
}

} // namespace

int main() {
    verify_exact_factorial_fixture(0U);
    for (const auto argument : {1U, 20U, 100U, 1'000U})
        verify_exact_factorial_fixture(argument);

    for (std::uint64_t candidate_value = 2U;
         candidate_value <= 1'024U; ++candidate_value) {
        compare_all_paths(candidate_value);
    }

    const std::array<std::uint64_t, 32U> structured_candidates{{
        49U, 64U, 81U, 121U, 125U, 128U, 169U, 243U,
        256U, 289U, 343U, 361U, 512U, 625U, 729U, 841U,
        899U, 961U, 1'024U, 1'025U, 2'047U, 2'048U, 2'049U,
        3'599U, 3'600U, 3'601U, 4'095U, 4'096U, 4'097U,
        4'899U, 4'900U, 4'901U}};
    for (const auto candidate_value : structured_candidates)
        compare_all_paths(candidate_value);

    angel::prime::JointWilsonPolicy audit_policy{};
    audit_policy.maximum_block_width = 100'000U;
    audit_policy.parallel_ntt_primes = false;
    std::uint64_t audit_count = 0U;
    for (std::uint64_t candidate_value = 2U;
         candidate_value <= 2'048U; ++candidate_value) {
        const auto audit = angel::detail::audit_joint_wilson_projection(
            candidate_value - 1U, candidate_value, audit_policy);
        REQUIRE(audit.residues_equal);
        REQUIRE(audit.time_work_strictly_lower);
        REQUIRE(audit.peak_space_strictly_lower);
        REQUIRE(audit.coordinate_strictly_lower);
        REQUIRE(audit.ring_additions_strictly_lower);
        REQUIRE(audit.ring_multiplications_strictly_lower);
        REQUIRE(audit.modular_reductions_strictly_lower);
        REQUIRE(audit.coefficient_updates_strictly_lower);
        REQUIRE(audit.peak_limbs_strictly_lower);
        REQUIRE(audit.optimized_ledger.allocation_count <
                audit.legacy_ledger.allocation_count);
        ++audit_count;
    }

    using namespace angel::prime;
    JointWilsonPolicy limited_policy{};
    limited_policy.maximum_block_width = 2U;
    const auto limited = candidate(1'009U)
        | upload_factorial_state()
        | bind_native_factorial()
        | project_wilson_jointly(limited_policy);
    REQUIRE(std::holds_alternative<ResourceLimit>(limited.observation));
    REQUIRE(!limited.ledger.ordinary_feedback);
    REQUIRE(!limited.ledger.full_factorial_materialized);
    REQUIRE(limited.ledger.native_state_nodes_rewritten == 0U);

    std::cout << "joint_wilson_tests=PASS\n";
    std::cout << "continuous_public_candidates=1023\n";
    std::cout << "continuous_pareto_audits=" << audit_count << '\n';
    std::cout << "exact_factorial_fixtures=0,1,20,100,1000\n";
    std::cout << "native_state_nodes_rewritten=0\n";
    std::cout << "ordinary_feedback=0\n";
    std::cout << "fixed_width_truncation=0\n";
    return 0;
}
