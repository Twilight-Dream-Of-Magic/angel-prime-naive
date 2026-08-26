#include "angel/boundary.hpp"
#include "angel/cyclic_structure.hpp"
#include "angel/diagnostics.hpp"
#include "angel/joint_wilson.hpp"
#include "angel/native_factorial.hpp"
#include "angel/prime.hpp"

#include <chrono>
#include <iostream>

int main() {
    const auto layout = angel::diagnostics::frozen_layout();
    std::cout << "arithmetic_state_object_bytes="
              << layout.arithmetic_state_bytes << '\n';
    std::cout << "cyclic_boundary_state_object_bytes="
              << layout.cyclic_boundary_state_bytes << '\n';
    std::cout << "public_factorial_handle_bytes="
              << layout.public_factorial_handle_bytes << '\n';
    std::cout << "public_boundary_handle_bytes="
              << layout.public_boundary_handle_bytes << '\n';

    using namespace angel::prime;
    const auto factorial = candidate(1009U) | upload_factorial_state();
    const auto state = factorial.summary();
    const auto view = factorial | bind_quotient_view();
    const auto coordinate = view.coordinate();
    const auto before = std::chrono::steady_clock::now();
    const auto result = view | download_wilson();
    const auto after = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        after - before).count();
    std::cout << "candidate=1009\n";
    std::cout << "native_steps=" << state.native_steps << '\n';
    std::cout << "native_payload_bytes=" << state.payload_bytes << '\n';
    std::cout << "external_block_width=" << coordinate.block_width << '\n';
    std::cout << "external_scalar_slots=" << coordinate.total_scalar_slots << '\n';
    std::cout << "external_download_wall_nanoseconds=" << elapsed << '\n';
    std::cout << "state_integrity="
              << (result.integrity.preserved() ? "preserved" : "failed") << '\n';
    const auto native_factorial = factorial | bind_native_factorial();
    const auto native_wilson =
        native_factorial | project_wilson_from_native();
    std::cout << "wilson_factor_count_source="
              << (native_wilson.evidence.factor_count_loaded_from_native_coordinate
                      ? "native_factorial_coordinate"
                      : "invalid")
              << '\n';
    std::cout << "wilson_native_result_seal="
              << native_wilson.evidence.native_result_seal << '\n';
    std::cout << "wilson_native_coordinate_verified="
              << (native_wilson.evidence.native_coordinate_verified
                      ? "yes" : "no")
              << '\n';

    JointWilsonPolicy joint_policy{};
    joint_policy.parallel_ntt_primes = false;
    const auto joint_wilson =
        native_factorial | project_wilson_jointly(joint_policy);
    std::cout << "joint_wilson_work_units="
              << joint_wilson.ledger.deterministic_work_units << '\n';
    std::cout << "joint_wilson_peak_live_coefficients_upper_bound="
              << joint_wilson.ledger.peak_live_coefficients_upper_bound
              << '\n';
    std::cout << "joint_wilson_materialized_coordinate_count="
              << joint_wilson.coordinate.materialized_coordinate_count
              << '\n';
    std::cout << "joint_wilson_state_nodes_rewritten="
              << joint_wilson.ledger.native_state_nodes_rewritten << '\n';
    std::cout << "joint_wilson_verified="
              << (joint_wilson.verified() ? "yes" : "no") << '\n';

    const auto exact_started = std::chrono::steady_clock::now();
    const auto exact_1000 = candidate(1001U)
                          | upload_factorial_state()
                          | bind_native_factorial()
                          | derive_exact_factorial()
                          | download_exact_factorial();
    const auto exact_stopped = std::chrono::steady_clock::now();
    const auto exact_elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            exact_stopped - exact_started).count();
    std::cout << "exact_factorial_argument=1000\n";
    std::cout << "exact_factorial_bits="
              << exact_1000.ledger.result_bits << '\n';
    std::cout << "exact_factorial_limbs="
              << exact_1000.ledger.result_limbs << '\n';
    std::cout << "exact_factorial_decimal_digits="
              << exact_1000.decimal.size() << '\n';
    std::cout << "exact_sequential_limb_updates="
              << exact_1000.ledger.sequential_limb_updates << '\n';
    std::cout << "exact_product_tree_limb_products="
              << exact_1000.ledger.product_tree_limb_products << '\n';
    std::cout << "exact_decimal_limb_updates="
              << exact_1000.ledger.decimal_limb_updates << '\n';
    std::cout << "exact_derivation_wall_nanoseconds="
              << exact_elapsed << '\n';
    std::cout << "exact_factorial_verified="
              << (exact_1000.verification_passed ? "yes" : "no") << '\n';

    using namespace angel::boundary;
    SessionAuthority authority{0xCA550002U};
    BoundaryLedger ledger{};
    const auto packet =
        EncodedOrder::canonical(24U)
        | upload(authority, &ledger)
        | quotient_to(6U, &ledger)
        | continue_to(4U, &ledger)
        | observe_primitive(4U, 998244353U, 1U, &ledger)
        | download(&ledger);
    static_cast<void>(packet);
    std::cout << "reference_dense_coefficients="
              << ledger.dense_reference_coefficients << '\n';
    std::cout << "reference_dense_updates="
              << ledger.dense_reference_updates << '\n';
    std::cout << "boundary_nodes_rewritten=" << ledger.nodes_rewritten << '\n';

    CyclicActionPolicy cyclic_policy{};
    cyclic_policy.maximum_order = 512U;
    std::cout << "cyclic_scaling_columns="
              << "order,response_coefficients,factor_updates,peak_coefficients,"
                 "independent_number_theory_steps,verified\n";
    for (const std::uint64_t order : {64U, 128U, 256U, 512U}) {
        const auto cyclic_state =
            candidate(order) | upload_factorial_state();
        const auto cyclic_view = cyclic_state | bind_cyclic_action();
        const auto view_summary = cyclic_view.summary();
        const auto cyclic_result = cyclic_view
                                 | evaluate_cyclic_action(cyclic_policy)
                                 | download_cyclic_structure();
        const auto& cyclic_observation =
            std::get<CyclicStructureObservation>(cyclic_result.observation);
        std::cout << "cyclic_scaling="
                  << order << ','
                  << cyclic_observation.primitive_period_response.size() << ','
                  << cyclic_result.ledger.cyclic_coefficient_updates << ','
                  << cyclic_result.ledger.maximum_live_coefficients << ','
                  << cyclic_result.ledger.independent_number_theory_steps << ','
                  << (cyclic_result.verified() ? "yes" : "no") << '\n';
        if (order == 512U)
            std::cout << "cyclic_view_descriptor_words="
                      << view_summary.descriptor_words << '\n';
    }
    return 0;
}
