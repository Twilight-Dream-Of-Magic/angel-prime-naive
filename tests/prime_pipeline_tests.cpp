#include "angel/diagnostics.hpp"
#include "angel/prime.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace angel::prime;

void require(const bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

Download run(const std::uint64_t value, const ObservationPolicy policy = {}) {
    const auto state = candidate(value) | upload_factorial_state();
    const auto summary = state.summary();
    require(summary.candidate == value && summary.factorial_argument == value - 1U,
            "factorial state binding mismatch");
    require(summary.state_seal != 0U && summary.program_seal != 0U,
            "complete state identity is missing");
    const auto view = state | bind_quotient_view();
    require(view.preserves_complete_state_identity(),
            "quotient view did not preserve state identity");
    return view | download_wilson(policy);
}

void verify_decisions() {
    const auto prime = run(17U);
    const auto* prime_observation =
        std::get_if<WilsonObservation>(&prime.observation);
    require(prime_observation != nullptr && prime_observation->prime &&
                prime_observation->factorial_residue == 16U,
            "Wilson prime decision failed");
    require(prime.verification_passed && prime.integrity.preserved(),
            "prime observation integrity failed");
    require(!prime.coordinate.polylogarithmic_claimed &&
                prime.coordinate.rejected_as_polylogarithmic,
            "external coordinate was mislabeled polylogarithmic");

    const auto composite = run(21U);
    const auto* composite_observation =
        std::get_if<WilsonObservation>(&composite.observation);
    require(composite_observation != nullptr && !composite_observation->prime,
            "Wilson composite decision failed");
    require(composite.verification_passed && composite.integrity.preserved(),
            "composite observation integrity failed");
}

void verify_resource_boundary() {
    ObservationPolicy policy{};
    policy.maximum_block_width = 2U;
    const auto result = run(101U, policy);
    const auto* limit = std::get_if<ResourceLimit>(&result.observation);
    require(limit != nullptr && limit->required_block_width == 10U &&
                limit->allowed_block_width == 2U,
            "resource limit was not explicit");
    require(result.verification_passed && result.integrity.preserved(),
            "resource-limit path changed the state");
}

} // namespace

int main() {
    verify_decisions();
    verify_resource_boundary();
    const auto layout = angel::diagnostics::frozen_layout();
    require(layout.exact_arithmetic_state_held &&
                !layout.wrapper_compresses_state,
            "arithmetic state was replaced or compressed");
    std::cout << "PRIME_PIPELINE_TESTS=PASS\n";
    std::cout << "WILSON_OBSERVATION_VERIFIED=YES\n";
    std::cout << "ARITHMETIC_STATE_REWRITTEN=NO\n";
    std::cout << "ARITHMETIC_STATE_COMPRESSED=NO\n";
    return 0;
}
