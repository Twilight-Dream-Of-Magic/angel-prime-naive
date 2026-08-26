#include "angel/joint_wilson.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace {

constexpr std::uint64_t kProbeModulus = 1'000'000'007U;

[[nodiscard]] bool is_prime_reference(const std::uint64_t value) noexcept {
    if (value < 2U) return false;
    if (value % 2U == 0U) return value == 2U;
    for (std::uint64_t divisor = 3U;
         divisor <= value / divisor;
         divisor += 2U) {
        if (value % divisor == 0U) return false;
    }
    return true;
}

[[nodiscard]] std::uint64_t euler_phi(std::uint64_t value) noexcept {
    std::uint64_t result = value;
    std::uint64_t remaining = value;
    for (std::uint64_t prime = 2U;
         prime <= remaining / prime;
         ++prime) {
        if (remaining % prime != 0U) continue;
        while (remaining % prime == 0U) remaining /= prime;
        result -= result / prime;
    }
    if (remaining > 1U) result -= result / remaining;
    return result;
}

[[nodiscard]] std::uint64_t divisor_count(std::uint64_t value) noexcept {
    std::uint64_t result = 1U;
    std::uint64_t remaining = value;
    for (std::uint64_t prime = 2U;
         prime <= remaining / prime;
         ++prime) {
        if (remaining % prime != 0U) continue;
        std::uint64_t exponent = 0U;
        while (remaining % prime == 0U) {
            remaining /= prime;
            ++exponent;
        }
        result *= exponent + 1U;
    }
    if (remaining > 1U) result *= 2U;
    return result;
}

[[nodiscard]] std::vector<std::uint64_t> cyclic_prefix_row(
    const std::uint64_t room,
    const std::uint64_t prefix,
    const std::uint64_t modulus) {
    std::vector<std::uint64_t> row(
        static_cast<std::size_t>(room), 0U);
    row[0] = 1U;
    for (std::uint64_t step = 1U; step <= prefix; ++step) {
        const auto shift = step % room;
        auto next = row;
        for (std::uint64_t index = 0U; index < room; ++index) {
            const auto source = (index + room - shift) % room;
            next[static_cast<std::size_t>(index)] =
                (row[static_cast<std::size_t>(index)] + modulus -
                 row[static_cast<std::size_t>(source)]) % modulus;
        }
        row = std::move(next);
    }
    return row;
}

[[nodiscard]] std::uint64_t modular_power(
    std::uint64_t base,
    std::uint64_t exponent,
    const std::uint64_t modulus) noexcept {
    std::uint64_t result = 1U;
    while (exponent != 0U) {
        if ((exponent & 1U) != 0U) result = (result * base) % modulus;
        base = (base * base) % modulus;
        exponent >>= 1U;
    }
    return result;
}

[[nodiscard]] std::uint64_t coordinate_fold_mod_room(
    const std::vector<std::uint64_t>& row,
    const std::uint64_t room,
    const std::uint64_t encoding_modulus) noexcept {
    std::uint64_t product = 1U % room;
    for (const auto encoded : row) {
        const bool negative = encoded > encoding_modulus / 2U;
        const auto magnitude = negative ? encoding_modulus - encoded : encoded;
        const auto reduced_magnitude = magnitude % room;
        const auto residue = negative && reduced_magnitude != 0U
            ? room - reduced_magnitude
            : reduced_magnitude;
        product = (product * residue) % room;
    }
    return product;
}

[[nodiscard]] std::uint64_t circulant_rank(
    const std::vector<std::uint64_t>& row,
    const std::uint64_t modulus) {
    const auto dimension = static_cast<std::uint64_t>(row.size());
    std::vector<std::vector<std::uint64_t>> matrix(
        static_cast<std::size_t>(dimension),
        std::vector<std::uint64_t>(static_cast<std::size_t>(dimension), 0U));
    for (std::uint64_t r = 0U; r < dimension; ++r) {
        for (std::uint64_t c = 0U; c < dimension; ++c) {
            const auto index = (c + dimension - r) % dimension;
            matrix[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] =
                row[static_cast<std::size_t>(index)];
        }
    }

    std::uint64_t rank = 0U;
    for (std::uint64_t column = 0U;
         column < dimension && rank < dimension;
         ++column) {
        std::uint64_t pivot = rank;
        while (pivot < dimension &&
               matrix[static_cast<std::size_t>(pivot)]
                     [static_cast<std::size_t>(column)] == 0U) {
            ++pivot;
        }
        if (pivot == dimension) continue;
        std::swap(matrix[static_cast<std::size_t>(pivot)],
                  matrix[static_cast<std::size_t>(rank)]);
        const auto pivot_value =
            matrix[static_cast<std::size_t>(rank)]
                  [static_cast<std::size_t>(column)];
        const auto inverse = modular_power(pivot_value, modulus - 2U, modulus);
        for (std::uint64_t c = column; c < dimension; ++c) {
            auto& value = matrix[static_cast<std::size_t>(rank)]
                                [static_cast<std::size_t>(c)];
            value = (value * inverse) % modulus;
        }
        for (std::uint64_t r = 0U; r < dimension; ++r) {
            if (r == rank) continue;
            const auto factor = matrix[static_cast<std::size_t>(r)]
                                      [static_cast<std::size_t>(column)];
            if (factor == 0U) continue;
            for (std::uint64_t c = column; c < dimension; ++c) {
                const auto reduction =
                    (factor * matrix[static_cast<std::size_t>(rank)]
                                    [static_cast<std::size_t>(c)]) % modulus;
                auto& value = matrix[static_cast<std::size_t>(r)]
                                    [static_cast<std::size_t>(c)];
                value = (value + modulus - reduction) % modulus;
            }
        }
        ++rank;
    }
    return rank;
}

[[nodiscard]] std::uint64_t expected_prefix_rank(
    const std::uint64_t room,
    const std::uint64_t prefix) noexcept {
    std::uint64_t rank = 0U;
    for (std::uint64_t divisor = 1U; divisor <= room; ++divisor) {
        if (room % divisor == 0U && divisor > prefix) {
            rank += euler_phi(divisor);
        }
    }
    return rank;
}

[[nodiscard]] std::uint64_t factorial_mod(const std::uint64_t value) noexcept {
    std::uint64_t result = 1U;
    for (std::uint64_t factor = 2U; factor <= value; ++factor) {
        result = (result * factor) % kProbeModulus;
    }
    return result;
}

[[nodiscard]] std::vector<std::uint64_t> higher_jet_coefficients(
    const std::uint64_t order,
    const std::uint64_t extra) {
    const auto limit = order + extra;
    std::vector<std::uint64_t> product(
        static_cast<std::size_t>(limit + 1U), 0U);
    product[0] = 1U;
    for (std::uint64_t k = 1U; k <= order; ++k) {
        std::vector<std::uint64_t> factor(
            static_cast<std::size_t>(limit + 1U), 0U);
        std::uint64_t binomial = 1U;
        for (std::uint64_t degree = 1U;
             degree <= k && degree <= limit;
             ++degree) {
            binomial = (binomial * (k - degree + 1U)) % kProbeModulus;
            binomial = (binomial * modular_power(
                degree, kProbeModulus - 2U, kProbeModulus)) % kProbeModulus;
            factor[static_cast<std::size_t>(degree)] =
                binomial == 0U ? 0U : kProbeModulus - binomial;
        }
        std::vector<std::uint64_t> next(
            static_cast<std::size_t>(limit + 1U), 0U);
        for (std::uint64_t left = 0U; left <= limit; ++left) {
            if (product[static_cast<std::size_t>(left)] == 0U) continue;
            for (std::uint64_t right = 1U;
                 right + left <= limit && right <= k;
                 ++right) {
                const auto term =
                    (product[static_cast<std::size_t>(left)] *
                     factor[static_cast<std::size_t>(right)]) % kProbeModulus;
                auto& coefficient =
                    next[static_cast<std::size_t>(left + right)];
                coefficient = (coefficient + term) % kProbeModulus;
            }
        }
        product = std::move(next);
    }
    return product;
}

int run_wilson_probe() {
    using namespace angel::prime;
    std::cout
        << "candidate,reference_prime,wilson_prime,wilson_residue,phi_m,"
           "primitive_rank,cyclic_nullity,divisor_strata,interior_strata,"
           "pure_laplacian,two_boundary_spectrum,verified,work_units,"
           "peak_live_coefficients,materialized_coordinate\n";
    JointWilsonPolicy policy{};
    policy.parallel_ntt_primes = false;
    for (std::uint64_t value = 2U; value <= 4'096U; ++value) {
        const auto result = candidate(value)
            | upload_factorial_state()
            | bind_native_factorial()
            | project_wilson_jointly(policy);
        const auto* observation = std::get_if<WilsonObservation>(
            &result.observation);
        if (observation == nullptr || !result.verified()) return 2;
        const auto phi = euler_phi(value);
        const auto strata = divisor_count(value);
        const bool reference_prime = is_prime_reference(value);
        const bool pure_laplacian = phi == value - 1U;
        const bool two_boundary = strata == 2U;
        if (observation->prime != reference_prime ||
            pure_laplacian != reference_prime ||
            two_boundary != reference_prime) {
            return 3;
        }
        std::cout
            << value << ','
            << static_cast<unsigned>(reference_prime) << ','
            << static_cast<unsigned>(observation->prime) << ','
            << observation->factorial_residue << ','
            << phi << ',' << phi << ',' << value - phi << ','
            << strata << ',' << strata - 2U << ','
            << static_cast<unsigned>(pure_laplacian) << ','
            << static_cast<unsigned>(two_boundary) << ','
            << static_cast<unsigned>(result.verified()) << ','
            << result.ledger.deterministic_work_units << ','
            << result.ledger.peak_live_coefficients_upper_bound << ','
            << result.coordinate.materialized_coordinate_count << '\n';
    }
    return 0;
}

int run_cyclic_probe() {
    std::cout
        << "room,reference_prime,primitive_rank,cyclic_nullity,"
           "row_differs_from_pure_laplacian,coordinate_fold_mod_room,"
           "prime_fold_collision,pure_laplacian_equivalence\n";
    for (std::uint64_t room = 2U; room <= 256U; ++room) {
        const auto row = cyclic_prefix_row(room, room - 1U, kProbeModulus);
        std::uint64_t differences = 0U;
        for (std::uint64_t index = 0U; index < room; ++index) {
            const auto signed_expected = index == 0U ? room - 1U :
                kProbeModulus - 1U;
            if (row[static_cast<std::size_t>(index)] != signed_expected) {
                ++differences;
            }
        }
        const bool prime = is_prime_reference(room);
        const auto coordinate_fold =
            coordinate_fold_mod_room(row, room, kProbeModulus);
        const bool fold_collision =
            !prime || coordinate_fold == room - 1U;
        if ((differences == 0U) != prime || !fold_collision) return 4;
        const auto phi = euler_phi(room);
        std::cout
            << room << ',' << static_cast<unsigned>(prime) << ','
            << phi << ',' << room - phi << ',' << differences << ','
            << coordinate_fold << ','
            << static_cast<unsigned>(fold_collision) << ','
            << static_cast<unsigned>((differences == 0U) == prime) << '\n';
    }
    return 0;
}

int run_rank_probe() {
    std::cout << "room,prefix,observed_rank,expected_rank,agreement\n";
    for (std::uint64_t room = 2U; room <= 96U; ++room) {
        std::array<std::uint64_t, 6U> samples{{
            1U, 2U, 3U,
            std::max<std::uint64_t>(1U, room / 4U),
            std::max<std::uint64_t>(1U, room / 2U), room - 1U}};
        std::sort(samples.begin(), samples.end());
        const auto unique_end = std::unique(samples.begin(), samples.end());
        for (auto it = samples.begin(); it != unique_end; ++it) {
            const auto prefix = std::min(*it, room - 1U);
            const auto row = cyclic_prefix_row(room, prefix, kProbeModulus);
            const auto observed = circulant_rank(row, kProbeModulus);
            const auto expected = expected_prefix_rank(room, prefix);
            if (observed != expected) return 5;
            std::cout << room << ',' << prefix << ',' << observed << ','
                      << expected << ",1\n";
        }
    }
    return 0;
}

int run_jet_probe() {
    std::cout
        << "order,leading_mod_1000000007,next_mod_1000000007,"
           "leading_formula,next_formula_identity,higher_2_mod,"
           "higher_3_mod,higher_4_mod\n";
    for (std::uint64_t order = 1U; order <= 64U; ++order) {
        const auto coefficients = higher_jet_coefficients(order, 4U);
        auto leading = factorial_mod(order);
        if ((order & 1U) != 0U && leading != 0U) {
            leading = kProbeModulus - leading;
        }
        const auto next = coefficients[static_cast<std::size_t>(order + 1U)];
        const bool leading_ok =
            coefficients[static_cast<std::size_t>(order)] == leading;
        const bool next_ok =
            (4U * next) % kProbeModulus ==
            (((leading * order) % kProbeModulus) * (order - 1U)) %
                kProbeModulus;
        if (!leading_ok || !next_ok) return 6;
        std::cout
            << order << ',' << coefficients[static_cast<std::size_t>(order)]
            << ',' << next << ',' << static_cast<unsigned>(leading_ok) << ','
            << static_cast<unsigned>(next_ok) << ','
            << coefficients[static_cast<std::size_t>(order + 2U)] << ','
            << coefficients[static_cast<std::size_t>(order + 3U)] << ','
            << coefficients[static_cast<std::size_t>(order + 4U)] << '\n';
    }
    return 0;
}

} // namespace

int main(const int argc, const char* const* argv) {
    if (argc != 2) return 64;
    const std::string mode{argv[1]};
    if (mode == "wilson") return run_wilson_probe();
    if (mode == "cyclic") return run_cyclic_probe();
    if (mode == "rank") return run_rank_probe();
    if (mode == "jet") return run_jet_probe();
    return 64;
}
