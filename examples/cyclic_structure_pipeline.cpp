#include "angel/cyclic_structure.hpp"

#include <iostream>
#include <variant>

int main() {
    using namespace angel::prime;

    CyclicActionPolicy policy{};
    policy.maximum_order = 256U;
    policy.include_normalized_action = true;
    policy.compute_kernel_dimension = true;

    const auto result = candidate(15U)
                      | upload_factorial_state()
                      | bind_cyclic_action()
                      | evaluate_cyclic_action(policy)
                      | download_cyclic_structure();

    if (const auto* observation =
            std::get_if<CyclicStructureObservation>(&result.observation)) {
        std::cout << "order=" << observation->order << '\n';
        std::cout << "period_structure="
                  << (observation->structure == PeriodStructure::PrimitiveOnly
                          ? "primitive-only"
                          : "proper-period-kernel")
                  << '\n';
        if (observation->proper_period_kernel_dimension)
            std::cout << "proper_period_kernel_dimension="
                      << *observation->proper_period_kernel_dimension << '\n';
    }
    std::cout << "verified=" << (result.verified() ? "yes" : "no") << '\n';
    std::cout << "state_preserved="
              << (result.integrity.preserved() ? "yes" : "no") << '\n';
    return result.verified() ? 0 : 1;
}
