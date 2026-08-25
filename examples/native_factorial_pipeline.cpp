#include "angel/native_factorial.hpp"

#include <iostream>
#include <variant>

int main() {
    using namespace angel::prime;

    const auto native = candidate(101U)
                      | upload_factorial_state()
                      | bind_native_factorial();
    const auto exact = native | derive_exact_factorial();
    const auto ordinary = exact | download_exact_factorial();
    const auto wilson = exact | observe_wilson_from_exact();
    const auto& observation = std::get<WilsonObservation>(wilson.observation);

    std::cout << "factorial_argument=" << ordinary.factorial_argument << '\n';
    std::cout << "decimal_digits=" << ordinary.decimal.size() << '\n';
    std::cout << "factorial_residue=" << observation.factorial_residue << '\n';
    std::cout << "wilson_prime=" << (observation.prime ? "yes" : "no") << '\n';
    std::cout << "native_coordinate_consumed="
              << (wilson.evidence.factor_count_loaded_from_native_coordinate
                      ? "yes" : "no")
              << '\n';
    return ordinary.verification_passed && wilson.verified() ? 0 : 1;
}
