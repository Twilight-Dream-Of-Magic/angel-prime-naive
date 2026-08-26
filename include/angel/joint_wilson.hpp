#pragma once

#include "angel/native_factorial.hpp"

#include <cstdint>

namespace angel::prime {

// Additive policy for the complement-paired external Wilson consumer.
// The native factorial state is not modified; this policy applies only after
// the explicit observation boundary has been crossed.
struct JointWilsonPolicy final {
    std::uint64_t maximum_block_width{100'000U};
    std::uint64_t width_search_radius{8U};
    std::uint64_t schoolbook_product_limit{65'536U};
    std::uint64_t maximum_ntt_length{1U << 23U};
    std::uint32_t minimum_crt_primes{};
    bool force_schoolbook{};
    bool parallel_ntt_primes{true};
};

struct JointWilsonCoordinate final {
    std::uint64_t original_factor_count{};
    std::uint64_t reduced_factor_count{};
    std::uint64_t legacy_block_width{};
    std::uint64_t optimized_block_width{};
    std::uint64_t full_blocks{};
    std::uint64_t tail_factors{};
    std::uint64_t logical_evaluation_points{};
    std::uint64_t materialized_block_coefficients{};
    std::uint64_t materialized_evaluation_point_array{};
    std::uint64_t materialized_block_value_array{};
    std::uint64_t materialized_coordinate_count{};
    std::uint64_t legacy_materialized_coordinate_count{};
};

// All counters are deterministic algorithmic counters except elapsed time.
// peak_live_coefficients counts simultaneously live coefficient slots owned by
// the outer algorithm. peak_live_coefficients_upper_bound additionally charges
// a conservative bound for multiplication/remainder scratch inside the exact
// polynomial engine.
struct JointWilsonLedger final {
    std::uint64_t input_bits{};
    std::uint64_t native_steps{};
    std::uint64_t state_payload_bytes{};

    std::uint64_t ring_additions{};
    std::uint64_t ring_multiplications{};
    std::uint64_t modular_reductions{};
    std::uint64_t coefficient_updates{};
    std::uint64_t deterministic_work_units{};
    std::uint64_t limb_products{};
    std::uint64_t limb_additions{};

    std::uint64_t polynomial_multiplications{};
    std::uint64_t schoolbook_coefficient_products{};
    std::uint64_t ntt_butterflies{};
    std::uint64_t crt_mixed_radix_digits{};
    std::uint64_t monic_remainders{};
    std::uint64_t horner_coefficient_steps{};
    std::uint64_t maximum_polynomial_coefficients{};
    std::uint64_t maximum_ntt_length{};

    std::uint64_t temporary_polynomial_count{};
    std::uint64_t temporary_big_integer_count{};
    std::uint64_t allocation_count{};
    std::uint64_t copied_bytes_upper_bound{};
    std::uint64_t peak_live_coefficients{};
    std::uint64_t peak_live_coefficients_upper_bound{};
    std::uint64_t peak_live_limbs{};
    std::uint64_t materialized_coordinate_count{};
    std::uint64_t native_state_nodes_rewritten{};
    std::uint64_t ordinary_elapsed_nanoseconds{};

    bool ordinary_projection_started{};
    bool ordinary_projection_completed{};
    bool ordinary_feedback{};
    bool full_factorial_materialized{};
    bool exact_counters_checked{};
};

struct JointWilsonEvidence final {
    std::uint64_t factorial_argument{};
    std::uint64_t native_coefficient{};
    std::uint64_t native_result_seal{};
    std::uint64_t native_certificate_seal{};
    bool native_coordinate_verified{};
    bool factor_count_loaded_from_native_coordinate{};
    bool candidate_used_only_as_modulus_and_consistency_check{};
    bool complement_pairing_identity_verified{};
    bool streamed_scalar_projection{};
    bool square_root_coordinate_eliminated{};
    bool full_factorial_materialized{};
    bool ordinary_feedback{};
};

struct JointWilsonDownload final {
    OrdinaryObservation observation{};
    JointWilsonCoordinate coordinate{};
    JointWilsonLedger ledger{};
    StateIntegrity integrity{};
    JointWilsonEvidence evidence{};
    bool verification_passed{};

    [[nodiscard]] bool verified() const noexcept {
        return verification_passed && integrity.preserved() &&
               evidence.native_coordinate_verified &&
               evidence.factor_count_loaded_from_native_coordinate &&
               evidence.complement_pairing_identity_verified &&
               evidence.streamed_scalar_projection &&
               !evidence.full_factorial_materialized &&
               !evidence.ordinary_feedback &&
               ledger.native_state_nodes_rewritten == 0U &&
               !ledger.ordinary_feedback &&
               !ledger.full_factorial_materialized &&
               ledger.exact_counters_checked;
    }
};

struct ProjectWilsonJointly final {
    JointWilsonPolicy policy{};
};

[[nodiscard]] constexpr ProjectWilsonJointly project_wilson_jointly(
    const JointWilsonPolicy policy = {}) noexcept {
    return ProjectWilsonJointly{policy};
}

[[nodiscard]] JointWilsonDownload operator|(
    const NativeFactorialView& view, ProjectWilsonJointly operation);

} // namespace angel::prime
