#include "internal/frozen_types.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using Polynomial = angel::detail::frozen::wilson::Polynomial;
using PolynomialLedger = angel::detail::frozen::wilson::PolynomialLedger;
using PolynomialPolicy = angel::detail::frozen::wilson::PolynomialPolicy;
using PolynomialRing =
    angel::detail::frozen::wilson::CompositeSafePolynomialRing;
__extension__ using WideUnsigned = unsigned __int128;

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

[[nodiscard]] std::uint64_t multiply_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return static_cast<std::uint64_t>(
        (static_cast<WideUnsigned>(left) * right) % modulus);
}

[[nodiscard]] std::uint64_t add_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= modulus - right ? left - (modulus - right) : left + right;
}

[[nodiscard]] std::uint64_t subtract_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= right ? left - right : modulus - (right - left);
}

void trim(Polynomial& value) {
    while (!value.empty() && value.back() == 0U) value.pop_back();
}

[[nodiscard]] Polynomial reference_multiply(
    const Polynomial& left,
    const Polynomial& right,
    const std::uint64_t modulus) {
    if (left.empty() || right.empty()) return {};
    Polynomial result(left.size() + right.size() - 1U, 0U);
    for (std::size_t i = 0U; i < left.size(); ++i) {
        for (std::size_t j = 0U; j < right.size(); ++j) {
            result[i + j] = add_mod(
                result[i + j],
                multiply_mod(left[i], right[j], modulus),
                modulus);
        }
    }
    trim(result);
    return result;
}

[[nodiscard]] Polynomial reference_monic_remainder(
    const Polynomial& dividend,
    const Polynomial& divisor,
    const std::uint64_t modulus) {
    if (divisor.empty() || divisor.back() != 1U)
        throw std::invalid_argument("reference divisor is not monic");
    if (dividend.size() < divisor.size()) return dividend;
    Polynomial remainder = dividend;
    const auto divisor_degree = divisor.size() - 1U;
    for (std::size_t cursor = remainder.size(); cursor-- > divisor_degree;) {
        const auto coefficient = remainder[cursor];
        if (coefficient != 0U) {
            const auto shift = cursor - divisor_degree;
            for (std::size_t index = 0U; index < divisor_degree; ++index) {
                remainder[shift + index] = subtract_mod(
                    remainder[shift + index],
                    multiply_mod(coefficient, divisor[index], modulus),
                    modulus);
            }
        }
        remainder[cursor] = 0U;
    }
    remainder.resize(divisor_degree);
    trim(remainder);
    return remainder;
}

class DeterministicGenerator final {
public:
    explicit DeterministicGenerator(const std::uint64_t seed) : state_(seed) {}

    [[nodiscard]] std::uint64_t next() noexcept {
        state_ ^= state_ << 13U;
        state_ ^= state_ >> 7U;
        state_ ^= state_ << 17U;
        return state_;
    }

private:
    std::uint64_t state_;
};

[[nodiscard]] Polynomial generated_polynomial(
    const std::size_t size,
    const std::uint64_t modulus,
    DeterministicGenerator& generator) {
    Polynomial value(size, 0U);
    for (auto& coefficient : value) coefficient = generator.next() % modulus;
    if (!value.empty() && value.back() == 0U) value.back() = 1U;
    return value;
}

void compare_multiplication(
    const std::uint64_t modulus,
    const bool parallel) {
    DeterministicGenerator generator{modulus ^ 0x9e3779b97f4a7c15ULL};
    const auto left = generated_polynomial(257U, modulus, generator);
    const auto right = generated_polynomial(263U, modulus, generator);
    const auto reference = reference_multiply(left, right, modulus);

    PolynomialLedger ledger{};
    PolynomialPolicy policy{};
    policy.parallel_ntt_primes = parallel;
    PolynomialRing ring{modulus, ledger, policy};
    const auto actual = ring.multiply(left, right);

    require(actual == reference, "NTT/CRT multiplication mismatch");
    require(ledger.ntt_convolutions != 0U,
            "NTT/CRT multiplication path was not exercised");
    if (parallel) {
        require(ledger.ntt_parallel_batches != 0U &&
                    ledger.maximum_ntt_workers > 1U,
                "parallel NTT-prime path was not exercised");
    }
}

void compare_remainder(
    const std::uint64_t modulus,
    const bool parallel) {
    DeterministicGenerator generator{modulus ^ 0xd1b54a32d192ed03ULL};
    const auto dividend = generated_polynomial(520U, modulus, generator);
    auto divisor = generated_polynomial(257U, modulus, generator);
    divisor.back() = 1U;
    const auto reference =
        reference_monic_remainder(dividend, divisor, modulus);

    PolynomialLedger ledger{};
    PolynomialPolicy policy{};
    policy.parallel_ntt_primes = parallel;
    PolynomialRing ring{modulus, ledger, policy};
    const auto actual = ring.monic_remainder(dividend, divisor);

    require(actual == reference, "fast monic remainder mismatch");
    require(ledger.newton_inverse_rounds != 0U &&
                ledger.ntt_convolutions != 0U,
            "fast monic remainder path was not exercised");
}

} // namespace

int main() {
    constexpr std::uint64_t composite_modulus = 4'294'967'296ULL;
    constexpr std::uint64_t large_modulus = 18'446'744'073'709'551'557ULL;

    for (const auto modulus : {composite_modulus, large_modulus}) {
        compare_multiplication(modulus, false);
        compare_multiplication(modulus, true);
        compare_remainder(modulus, false);
        compare_remainder(modulus, true);
    }

    std::cout << "POLYNOMIAL_RING_TESTS=PASS\n";
    std::cout << "COMPOSITE_MODULUS_DIFFERENTIAL=PASS\n";
    std::cout << "LARGE_UINT64_MODULUS_DIFFERENTIAL=PASS\n";
    std::cout << "PARALLEL_SERIAL_NTT_EQUIVALENCE=PASS\n";
    std::cout << "FAST_MONIC_REMAINDER_DIFFERENTIAL=PASS\n";
    return 0;
}
