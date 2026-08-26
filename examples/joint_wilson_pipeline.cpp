#include "angel/joint_wilson.hpp"

#include <iostream>
#include <variant>

int main() {
    using namespace angel::prime;

    JointWilsonPolicy policy{};
    policy.parallel_ntt_primes = true;

    const auto result = candidate(1009U)
        | upload_factorial_state()
        | bind_native_factorial()
        | project_wilson_jointly(policy);

    if (const auto* observation =
            std::get_if<WilsonObservation>(&result.observation)) {
        std::cout << "candidate=" << observation->candidate << '\n';
        std::cout << "factorial_residue="
                  << observation->factorial_residue << '\n';
        std::cout << "prime=" << (observation->prime ? "yes" : "no") << '\n';
        std::cout << "work_units="
                  << result.ledger.deterministic_work_units << '\n';
        std::cout << "peak_live_coefficients_upper_bound="
                  << result.ledger.peak_live_coefficients_upper_bound << '\n';
        std::cout << "verified=" << (result.verified() ? "yes" : "no")
                  << '\n';
        return result.verified() ? 0 : 2;
    }

    const auto& limit = std::get<ResourceLimit>(result.observation);
    std::cout << "resource_limit_required_width="
              << limit.required_block_width << '\n';
    return 3;
}
