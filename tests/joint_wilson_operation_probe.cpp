#include "internal/joint_wilson_runtime.hpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace {

[[nodiscard]] long double ratio(
    const std::uint64_t numerator,
    const std::uint64_t denominator) noexcept {
    return denominator == 0U
        ? 0.0L
        : static_cast<long double>(numerator) /
              static_cast<long double>(denominator);
}

} // namespace

int main() {
    using namespace angel;
    prime::JointWilsonPolicy policy{};
    policy.maximum_block_width = 100'000U;
    policy.parallel_ntt_primes = false;

    std::cout << std::fixed << std::setprecision(8);
    std::cout
        << "candidate,factor_count,old_work,new_work,T_ratio,"
           "old_peak_coeff,new_peak_coeff,S_ratio,"
           "old_coordinate,new_coordinate,C_ratio,"
           "old_ring_additions,new_ring_additions,"
           "old_ring_multiplications,new_ring_multiplications,"
           "old_modular_reductions,new_modular_reductions,"
           "old_coefficient_updates,new_coefficient_updates,"
           "old_limb_products,new_limb_products,"
           "old_limb_additions,new_limb_additions,"
           "old_poly_mult,new_poly_mult,"
           "old_schoolbook_products,new_schoolbook_products,"
           "old_ntt_butterflies,new_ntt_butterflies,"
           "old_crt_digits,new_crt_digits,"
           "old_monic_remainders,new_monic_remainders,"
           "old_horner_steps,new_horner_steps,"
           "old_temp_polynomials,new_temp_polynomials,"
           "old_temp_bigints,new_temp_bigints,"
           "old_allocations,new_allocations,"
           "old_copied_bytes_upper,new_copied_bytes_upper,"
           "old_peak_limbs,new_peak_limbs,"
           "old_materialized_points,new_materialized_points,"
           "old_materialized_values,new_materialized_values,"
           "old_native_nodes_rewritten,new_native_nodes_rewritten,"
           "old_full_factorial_materialized,new_full_factorial_materialized,"
           "old_ordinary_feedback,new_ordinary_feedback,verified\n";

    const std::array<std::uint64_t, 9U> candidates{{
        257U, 1'021U, 4'093U, 16'381U, 65'521U,
        100'003U, 250'003U, 500'009U, 1'000'003U}};

    for (const auto candidate : candidates) {
        const auto factor_count = candidate - 1U;
        const auto audit = detail::audit_joint_wilson_projection(
            factor_count, candidate, policy);
        const auto& old_coordinate = audit.legacy_coordinate;
        const auto& new_coordinate = audit.optimized_coordinate;
        const auto& old = audit.legacy_ledger;
        const auto& optimized = audit.optimized_ledger;

        const bool verified =
            audit.residues_equal && audit.time_work_strictly_lower &&
            audit.peak_space_strictly_lower &&
            audit.coordinate_strictly_lower &&
            audit.ring_additions_strictly_lower &&
            audit.ring_multiplications_strictly_lower &&
            audit.modular_reductions_strictly_lower &&
            audit.coefficient_updates_strictly_lower &&
            audit.peak_limbs_strictly_lower &&
            old.native_state_nodes_rewritten == 0U &&
            optimized.native_state_nodes_rewritten == 0U &&
            !old.full_factorial_materialized &&
            !optimized.full_factorial_materialized &&
            !old.ordinary_feedback && !optimized.ordinary_feedback &&
            old.exact_counters_checked && optimized.exact_counters_checked;
        if (!verified) return 2;

        std::cout
            << candidate << ','
            << factor_count << ','
            << old.deterministic_work_units << ','
            << optimized.deterministic_work_units << ','
            << ratio(optimized.deterministic_work_units,
                     old.deterministic_work_units) << ','
            << old.peak_live_coefficients_upper_bound << ','
            << optimized.peak_live_coefficients_upper_bound << ','
            << ratio(optimized.peak_live_coefficients_upper_bound,
                     old.peak_live_coefficients_upper_bound) << ','
            << old_coordinate.materialized_coordinate_count << ','
            << new_coordinate.materialized_coordinate_count << ','
            << ratio(new_coordinate.materialized_coordinate_count,
                     old_coordinate.materialized_coordinate_count) << ','
            << old.ring_additions << ',' << optimized.ring_additions << ','
            << old.ring_multiplications << ','
            << optimized.ring_multiplications << ','
            << old.modular_reductions << ','
            << optimized.modular_reductions << ','
            << old.coefficient_updates << ','
            << optimized.coefficient_updates << ','
            << old.limb_products << ',' << optimized.limb_products << ','
            << old.limb_additions << ',' << optimized.limb_additions << ','
            << old.polynomial_multiplications << ','
            << optimized.polynomial_multiplications << ','
            << old.schoolbook_coefficient_products << ','
            << optimized.schoolbook_coefficient_products << ','
            << old.ntt_butterflies << ',' << optimized.ntt_butterflies << ','
            << old.crt_mixed_radix_digits << ','
            << optimized.crt_mixed_radix_digits << ','
            << old.monic_remainders << ',' << optimized.monic_remainders << ','
            << old.horner_coefficient_steps << ','
            << optimized.horner_coefficient_steps << ','
            << old.temporary_polynomial_count << ','
            << optimized.temporary_polynomial_count << ','
            << old.temporary_big_integer_count << ','
            << optimized.temporary_big_integer_count << ','
            << old.allocation_count << ',' << optimized.allocation_count << ','
            << old.copied_bytes_upper_bound << ','
            << optimized.copied_bytes_upper_bound << ','
            << old.peak_live_limbs << ',' << optimized.peak_live_limbs << ','
            << old_coordinate.materialized_evaluation_point_array << ','
            << new_coordinate.materialized_evaluation_point_array << ','
            << old_coordinate.materialized_block_value_array << ','
            << new_coordinate.materialized_block_value_array << ','
            << old.native_state_nodes_rewritten << ','
            << optimized.native_state_nodes_rewritten << ','
            << static_cast<unsigned>(old.full_factorial_materialized) << ','
            << static_cast<unsigned>(optimized.full_factorial_materialized)
            << ','
            << static_cast<unsigned>(old.ordinary_feedback) << ','
            << static_cast<unsigned>(optimized.ordinary_feedback) << ','
            << (verified ? "yes" : "no") << '\n';
    }
    return 0;
}
