#include "angel/native_factorial.hpp"

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

[[nodiscard]] NativeFactorialView native_view(const std::uint64_t candidate_value) {
    const auto state = candidate(candidate_value) | upload_factorial_state();
    const auto before = state.summary();
    const auto view = state | bind_native_factorial();
    const auto summary = view.summary();
    require(summary.candidate == candidate_value &&
                summary.factorial_argument == candidate_value - 1U &&
                summary.coefficient == 1U &&
                summary.native_result_seal != 0U &&
                summary.native_certificate_seal != 0U &&
                summary.native_coordinate_verified &&
                !summary.denotation_materialized,
            "native factorial coordinate binding failed");
    require(view.preserves_complete_state_identity(),
            "native factorial view lost complete state identity");
    const auto after = state.summary();
    require(before.state_seal == after.state_seal &&
                before.program_seal == after.program_seal &&
                before.payload_bytes == after.payload_bytes,
            "native factorial view changed source state");
    return view;
}

[[nodiscard]] ExactFactorialValue exact_value(
    const std::uint64_t candidate_value,
    const std::uint64_t maximum_argument = 4096U) {
    ExactFactorialPolicy policy{};
    policy.maximum_factorial_argument = maximum_argument;
    const auto exact = native_view(candidate_value) | derive_exact_factorial(policy);
    const auto summary = exact.summary();
    require(summary.factorial_argument == candidate_value - 1U &&
                summary.native_coordinate_consumed &&
                summary.sequential_product_tree_equal &&
                summary.exact_value_seal != 0U,
            "exact factorial runtime derivation failed");
    return exact;
}

void verify_known_big_integer_values() {
    const auto factorial_20 = exact_value(21U) | download_exact_factorial();
    require(factorial_20.verification_passed &&
                factorial_20.integrity.preserved() &&
                factorial_20.decimal == "2432902008176640000" &&
                factorial_20.hexadecimal == "21c3677c82b40000",
            "20 factorial exact materialization mismatch");

    const auto factorial_100 = exact_value(101U) | download_exact_factorial();
    require(factorial_100.decimal ==
                "933262154439441526816992388562667004907159682643816214685929"
                "638952175999932299156089414639761565182862536979208272237582"
                "51185210916864000000000000000000000000" &&
                factorial_100.verification_passed,
            "100 factorial exact materialization mismatch");

    const auto factorial_1000 = exact_value(1001U) | download_exact_factorial();
    require(factorial_1000.verification_passed &&
                factorial_1000.ledger.result_bits == 8530U &&
                factorial_1000.decimal.size() == 2568U &&
                factorial_1000.decimal.starts_with(
                    "402387260077093773543702433923003985719374864210714632543799") &&
                factorial_1000.decimal.ends_with(
                    "000000000000000000000000000000000000000000000000000000000000"),
            "1000 factorial arbitrary-precision materialization mismatch");
    require(factorial_1000.ledger.result_limbs > 1U &&
                factorial_1000.ledger.sequential_small_multiplications == 999U &&
                factorial_1000.ledger.product_tree_multiplications == 998U &&
                factorial_1000.ledger.product_tree_limb_products != 0U &&
                factorial_1000.ledger.decimal_divisions != 0U,
            "arbitrary-precision work ledger is incomplete");
}

void verify_native_coordinate_is_consumed_by_wilson() {
    for (std::uint64_t candidate_value = 2U; candidate_value <= 64U;
         ++candidate_value) {
        const auto view = native_view(candidate_value);
        const auto direct = view | project_wilson_from_native();
        require(direct.verified() &&
                    direct.evidence.mode ==
                        WilsonConsumptionMode::NativeCoordinateModularProjection &&
                    direct.evidence.factorial_argument == candidate_value - 1U &&
                    direct.evidence.native_coefficient == 1U &&
                    direct.evidence.factor_count_loaded_from_native_coordinate &&
                    direct.evidence.candidate_used_only_as_modulus &&
                    !direct.evidence.exact_big_integer_consumed,
                "direct Wilson path did not consume native coordinate");
        const auto* direct_observation =
            std::get_if<WilsonObservation>(&direct.observation);
        require(direct_observation != nullptr &&
                    direct_observation->ledger.target_factor_count ==
                        direct.evidence.factorial_argument &&
                    direct_observation->prime == ordinary_prime(candidate_value),
                "native-coordinate Wilson result mismatch");

        const auto exact = view | derive_exact_factorial();
        const auto exact_wilson = exact | observe_wilson_from_exact();
        const auto* exact_observation =
            std::get_if<WilsonObservation>(&exact_wilson.observation);
        require(exact_wilson.verified() && exact_observation != nullptr &&
                    exact_wilson.evidence.mode ==
                        WilsonConsumptionMode::ExactBigIntegerRemainder &&
                    exact_wilson.evidence.exact_big_integer_consumed &&
                    exact_wilson.evidence.modular_result_matches_exact_big_integer &&
                    exact_observation->factorial_residue ==
                        direct_observation->factorial_residue &&
                    exact_observation->prime == direct_observation->prime,
                "exact big-integer Wilson consumption mismatch");

        const auto legacy = candidate(candidate_value)
                          | upload_factorial_state()
                          | bind_quotient_view()
                          | download_wilson();
        const auto* legacy_observation =
            std::get_if<WilsonObservation>(&legacy.observation);
        require(legacy.verification_passed && legacy_observation != nullptr &&
                    legacy_observation->factorial_residue ==
                        direct_observation->factorial_residue,
                "published Wilson API did not delegate to native coordinate");
    }
}

void verify_exact_resource_policy() {
    ExactFactorialPolicy policy{};
    policy.maximum_factorial_argument = 100U;
    bool rejected = false;
    try {
        static_cast<void>(
            native_view(102U) | derive_exact_factorial(policy));
    } catch (const std::length_error&) {
        rejected = true;
    }
    require(rejected, "exact factorial policy limit was ignored");
}

} // namespace

int main() {
    verify_known_big_integer_values();
    verify_native_coordinate_is_consumed_by_wilson();
    verify_exact_resource_policy();
    std::cout << "NATIVE_FACTORIAL_TESTS=PASS\n";
    std::cout << "RUNTIME_EXACT_FACTORIAL_DERIVATION=PASS\n";
    std::cout << "ARBITRARY_PRECISION_INTEGER=PASS\n";
    std::cout << "SEQUENTIAL_PRODUCT_TREE_INDEPENDENCE=PASS\n";
    std::cout << "WILSON_CONSUMES_NATIVE_COORDINATE=PASS\n";
    std::cout << "EXACT_BIG_INTEGER_WILSON_ORACLE=PASS\n";
    std::cout << "PUBLISHED_WILSON_API_NATIVE_DELEGATION=PASS\n";
    return 0;
}
