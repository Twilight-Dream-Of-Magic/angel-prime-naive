#include "angel/cyclic_structure.hpp"
#include "angel/version.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

namespace {

using namespace angel::prime;

void require(const bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

[[nodiscard]] bool ordinary_prime(const std::uint64_t value) noexcept {
    if (value < 2U) return false;
    for (std::uint64_t divisor = 2U; divisor <= value / divisor; ++divisor)
        if (value % divisor == 0U) return false;
    return true;
}

[[nodiscard]] CyclicStructureDownload run(
    const std::uint64_t order,
    const CyclicActionPolicy policy = {}) {
    const auto factorial = candidate(order) | upload_factorial_state();
    const auto before = factorial.summary();
    const auto view = factorial | bind_cyclic_action();
    const auto view_summary = view.summary();
    require(view_summary.order == order &&
                view_summary.represented_factor_count == order - 1U,
            "cyclic action diagonal binding mismatch");
    require(view_summary.descriptor_words == 7U &&
                !view_summary.contains_arithmetic_state &&
                !view_summary.compresses_arithmetic_state &&
                !view_summary.can_feed_back_to_arithmetic,
            "cyclic action view crossed the state boundary");
    require(view.preserves_complete_state_identity(),
            "cyclic action view lost source identity");
    const auto closed = view | evaluate_cyclic_action(policy);
    require(closed.certificate().observation_closed,
            "cyclic action observation did not close");
    const auto result = closed | download_cyclic_structure();
    const auto after = factorial.summary();
    require(before.request_binding == after.request_binding &&
                before.state_seal == after.state_seal &&
                before.program_seal == after.program_seal &&
                before.payload_bytes == after.payload_bytes,
            "cyclic observation changed the arithmetic state");
    return result;
}

void verify_exhaustive_period_structure() {
    CyclicActionPolicy policy{};
    policy.maximum_order = 128U;
    std::uint64_t primitive_only = 0U;
    std::uint64_t proper_period = 0U;
    for (std::uint64_t order = 2U; order <= policy.maximum_order; ++order) {
        const auto result = run(order, policy);
        require(result.verified(), "cyclic structure verification failed");
        const auto* observation =
            std::get_if<CyclicStructureObservation>(&result.observation);
        require(observation != nullptr,
                "unexpected resource limit in exhaustive range");
        require(observation->primitive_period_response.size() == order &&
                    observation->normalized_action_response.empty() &&
                    !observation->proper_period_kernel_dimension.has_value(),
                "default response dimensions are wrong");
        const bool expected = ordinary_prime(order);
        require((observation->structure == PeriodStructure::PrimitiveOnly) ==
                    expected,
                "period structure disagreed with divisor audit");
        require(result.verification.response_materialized &&
                    result.verification.ramanujan_replay_checked &&
                    result.verification.ramanujan_replay_valid &&
                    !result.verification.normalized_action_checked &&
                    result.verification.period_classification_valid,
                "independent structural replay was incomplete");
        require(result.ledger.factors_applied == order - 1U &&
                    result.ledger.cyclic_coefficient_updates ==
                        order * (order - 1U) &&
                    result.ledger.maximum_live_coefficients == 2U * order &&
                    result.ledger.independent_number_theory_steps != 0U &&
                    result.ledger.nodes_rewritten == 0U &&
                    result.ledger.nodes_merged == 0U &&
                    !result.ledger.ordinary_feedback,
                "cyclic action ledger is inconsistent");
        if (expected) ++primitive_only;
        else ++proper_period;
    }
    require(primitive_only == 31U && proper_period == 96U,
            "unexpected structure counts through order 128");
}

void verify_normalized_kernel_and_constant_mode() {
    struct Fixture final {
        std::uint64_t order;
        std::uint64_t kernel;
    };
    constexpr Fixture fixtures[]{{7U, 0U}, {8U, 3U}, {9U, 2U},
                                 {15U, 6U}, {25U, 4U}, {49U, 6U},
                                 {97U, 0U}};
    for (const auto modulus : {998'244'353U, 1'000'000'007U}) {
        CyclicActionPolicy policy{};
        policy.maximum_order = 128U;
        policy.audit_modulus = modulus;
        policy.include_normalized_action = true;
        policy.compute_kernel_dimension = true;
        for (const auto fixture : fixtures) {
            const auto result = run(fixture.order, policy);
            require(result.verified(),
                    "normalized cyclic structure verification failed");
            const auto& observation =
                std::get<CyclicStructureObservation>(result.observation);
            require(observation.normalized_action_response.size() ==
                        fixture.order &&
                    observation.proper_period_kernel_dimension ==
                        fixture.kernel,
                    "proper-period kernel dimension mismatch");
            require(result.verification.normalized_action_checked &&
                        result.verification.valuation_reattachment_valid &&
                        result.verification.constant_mode_factorial_valid &&
                        result.verification.kernel_dimension_checked &&
                        result.verification.kernel_dimension_valid,
                    "normalized action proof obligations failed");
            require(result.ledger.independent_relation_updates != 0U,
                    "normalized relation replay was not charged");
        }
    }
}

void verify_resource_limit_closes_before_dense_domain_checks() {
    CyclicActionPolicy policy{};
    policy.maximum_order = 1024U;
    const std::uint64_t large_order = (std::uint64_t{1U} << 40U) + 39U;
    const auto result = run(large_order, policy);
    const auto* limit =
        std::get_if<CyclicStructureResourceLimit>(&result.observation);
    require(limit != nullptr && limit->requested_order == large_order &&
                limit->maximum_order == policy.maximum_order,
            "large input did not produce a certified resource limit");
    require(result.verified() && result.certificate.resource_limited &&
                !result.verification.response_materialized &&
                !result.verification.ramanujan_replay_checked,
            "resource-limit closure or integrity failed");
}

void verify_invalid_evaluation_domain_is_rejected() {
    CyclicActionPolicy policy{};
    policy.maximum_order = 128U;
    policy.audit_modulus = 17U;
    bool rejected = false;
    try {
        static_cast<void>(run(17U, policy));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "invalid modular evaluation domain was accepted");

    policy.audit_modulus = 101U;
    policy.compute_kernel_dimension = true;
    rejected = false;
    try {
        static_cast<void>(run(17U, policy));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "unsupported kernel audit field was accepted");
}

} // namespace

int main() {
    require(angel::sdk_version_major == 1 && angel::sdk_version_minor == 1 &&
                angel::sdk_version_patch == 0,
            "semantic SDK version mismatch");
    verify_exhaustive_period_structure();
    verify_normalized_kernel_and_constant_mode();
    verify_resource_limit_closes_before_dense_domain_checks();
    verify_invalid_evaluation_domain_is_rejected();
    std::cout << "CYCLIC_STRUCTURE_TESTS=PASS\n";
    std::cout << "INDEPENDENT_RAMANUJAN_REPLAY=PASS\n";
    std::cout << "VALUATION_REATTACHMENT=PASS\n";
    std::cout << "CONSTANT_MODE_FACTORIAL=PASS\n";
    std::cout << "PROPER_PERIOD_KERNEL=PASS\n";
    std::cout << "ARITHMETIC_STATE_REWRITTEN=NO\n";
    std::cout << "ORDINARY_FEEDBACK=NO\n";
    return 0;
}
