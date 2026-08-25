#include "angel/boundary/behavior.hpp"

#include <iostream>
#include <stdexcept>

int main() {
    using namespace angel::boundary::behavior;
    MooreMachine<4U, 2U> machine{};
    machine.transition = {{{{2U, 0U}}, {{1U, 1U}}, {{2U, 2U}}, {{1U, 1U}}}};
    machine.observation = {{0U, 0U, 1U, 0U}};
    machine.semantic_version = 7U;
    machine.observer_contract = 11U;

    const auto partition = minimal_partition(machine);
    if (partition.class_count != 3U || future_equivalent(machine, 0U, 1U) ||
        !future_equivalent(machine, 1U, 3U) ||
        observation_is_resumable(machine) ||
        resume_complement_lower_bound_bits(machine) != 1U ||
        certify_stability(machine, 0U).has_value() ||
        !certify_stability(machine, 1U).has_value())
        throw std::runtime_error("behavioral boundary theorem failed");

    std::cout << "BEHAVIOR_TESTS=PASS\n";
    std::cout << "MINIMAL_BEHAVIOR_CLASSES=3\n";
    std::cout << "OBSERVATION_RESUMABLE_WITHOUT_COMPLEMENT=NO\n";
    return 0;
}
