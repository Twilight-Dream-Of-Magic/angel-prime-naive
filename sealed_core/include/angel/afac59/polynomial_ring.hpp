#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace angel::afac59 {

__extension__ using wide_uint = unsigned __int128;
using Polynomial = std::vector<std::uint64_t>;

struct PolynomialLedger final {
    std::uint64_t polynomial_multiplications{};
    std::uint64_t schoolbook_convolutions{};
    std::uint64_t schoolbook_coefficient_products{};
    std::uint64_t ntt_convolutions{};
    std::uint64_t ntt_prime_transforms{};
    std::uint64_t ntt_butterflies{};
    std::uint64_t ntt_parallel_batches{};
    std::uint64_t maximum_ntt_workers{};
    std::uint64_t crt_output_coefficients{};
    std::uint64_t crt_mixed_radix_digits{};
    std::uint64_t crt_exact_bound_checks{};
    std::uint64_t maximum_crt_primes{};
    std::uint64_t monic_remainders{};
    std::uint64_t newton_inverse_rounds{};
    std::uint64_t horner_evaluations{};
    std::uint64_t horner_coefficient_steps{};
    std::uint64_t product_tree_internal_nodes{};
    std::uint64_t remainder_tree_internal_nodes{};
    std::uint64_t maximum_polynomial_coefficients{};
    std::uint64_t maximum_ntt_length{};

    friend bool operator==(const PolynomialLedger&,
                           const PolynomialLedger&) = default;
};

inline void merge_polynomial_ledger(
    PolynomialLedger& destination, const PolynomialLedger& source) noexcept {
    destination.polynomial_multiplications += source.polynomial_multiplications;
    destination.schoolbook_convolutions += source.schoolbook_convolutions;
    destination.schoolbook_coefficient_products +=
        source.schoolbook_coefficient_products;
    destination.ntt_convolutions += source.ntt_convolutions;
    destination.ntt_prime_transforms += source.ntt_prime_transforms;
    destination.ntt_butterflies += source.ntt_butterflies;
    destination.ntt_parallel_batches += source.ntt_parallel_batches;
    destination.maximum_ntt_workers = std::max(
        destination.maximum_ntt_workers, source.maximum_ntt_workers);
    destination.crt_output_coefficients += source.crt_output_coefficients;
    destination.crt_mixed_radix_digits += source.crt_mixed_radix_digits;
    destination.crt_exact_bound_checks += source.crt_exact_bound_checks;
    destination.maximum_crt_primes = std::max(
        destination.maximum_crt_primes, source.maximum_crt_primes);
    destination.monic_remainders += source.monic_remainders;
    destination.newton_inverse_rounds += source.newton_inverse_rounds;
    destination.horner_evaluations += source.horner_evaluations;
    destination.horner_coefficient_steps += source.horner_coefficient_steps;
    destination.product_tree_internal_nodes += source.product_tree_internal_nodes;
    destination.remainder_tree_internal_nodes +=
        source.remainder_tree_internal_nodes;
    destination.maximum_polynomial_coefficients = std::max(
        destination.maximum_polynomial_coefficients,
        source.maximum_polynomial_coefficients);
    destination.maximum_ntt_length = std::max(
        destination.maximum_ntt_length, source.maximum_ntt_length);
}

struct PolynomialPolicy final {
    std::uint64_t schoolbook_product_limit{65'536U};
    std::uint64_t maximum_ntt_length{1U << 23U};
    std::uint32_t minimum_crt_primes{};
    bool force_schoolbook{};
    bool parallel_ntt_primes{true};
};

class PolynomialResourceError final : public std::runtime_error {
public:
    explicit PolynomialResourceError(const char* message)
        : std::runtime_error(message) {}
};

[[nodiscard]] inline std::uint64_t add_mod(
    const std::uint64_t left, const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= modulus - right ? left - (modulus - right) : left + right;
}

[[nodiscard]] inline std::uint64_t subtract_mod(
    const std::uint64_t left, const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= right ? left - right : modulus - (right - left);
}

[[nodiscard]] inline std::uint64_t multiply_mod(
    const std::uint64_t left, const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return static_cast<std::uint64_t>(
        (static_cast<wide_uint>(left) * right) % modulus);
}

inline void trim(Polynomial& value) {
    while (!value.empty() && value.back() == 0U) value.pop_back();
}

[[nodiscard]] inline std::uint64_t prime_power_mod(
    std::uint64_t base, std::uint64_t exponent,
    const std::uint32_t modulus) noexcept {
    std::uint64_t result = 1U;
    base %= modulus;
    while (exponent != 0U) {
        if ((exponent & 1U) != 0U)
            result = (result * base) % modulus;
        base = (base * base) % modulus;
        exponent >>= 1U;
    }
    return result;
}

struct NTTPrime final {
    std::uint32_t modulus;
    std::uint32_t primitive_root;
    std::uint32_t maximum_power_of_two;
    std::uint32_t conservative_bits;
};

inline constexpr std::array<NTTPrime, 6U> ntt_primes{{
    {2'013'265'921U, 31U, 27U, 30U},
    {1'224'736'769U, 3U, 24U, 30U},
    {998'244'353U, 3U, 23U, 29U},
    {754'974'721U, 11U, 24U, 29U},
    {469'762'049U, 3U, 26U, 28U},
    {167'772'161U, 3U, 25U, 27U},
}};

class NTTBatchExecutor final {
public:
    explicit NTTBatchExecutor(const std::size_t worker_count) {
        workers_.reserve(worker_count);
        for (std::size_t index = 0U; index < worker_count; ++index)
            workers_.emplace_back([this] { worker_loop(); });
    }

    NTTBatchExecutor(const NTTBatchExecutor&) = delete;
    NTTBatchExecutor& operator=(const NTTBatchExecutor&) = delete;

    ~NTTBatchExecutor() {
        {
            const std::lock_guard lock{mutex_};
            stopping_ = true;
            ++generation_;
        }
        start_.notify_all();
        for (auto& worker : workers_)
            if (worker.joinable()) worker.join();
    }

    void run(
        const std::size_t task_count,
        std::function<void(std::size_t)> task) {
        if (task_count == 0U) return;
        std::unique_lock lock{mutex_};
        task_ = std::move(task);
        task_count_ = task_count;
        next_task_ = 0U;
        completed_tasks_ = 0U;
        error_ = nullptr;
        ++generation_;
        start_.notify_all();
        done_.wait(lock, [&] { return completed_tasks_ == task_count_; });
        task_ = {};
        const auto error = error_;
        lock.unlock();
        if (error) std::rethrow_exception(error);
    }

private:
    std::mutex mutex_{};
    std::condition_variable start_{};
    std::condition_variable done_{};
    std::vector<std::thread> workers_{};
    std::function<void(std::size_t)> task_{};
    std::exception_ptr error_{};
    std::size_t task_count_{};
    std::size_t next_task_{};
    std::size_t completed_tasks_{};
    std::uint64_t generation_{};
    bool stopping_{};

    void worker_loop() noexcept {
        std::uint64_t observed_generation = 0U;
        for (;;) {
            std::unique_lock lock{mutex_};
            start_.wait(lock, [&] {
                return stopping_ || generation_ != observed_generation;
            });
            if (stopping_) return;
            observed_generation = generation_;
            while (next_task_ < task_count_) {
                const auto index = next_task_++;
                const auto task = task_;
                lock.unlock();
                try {
                    task(index);
                } catch (...) {
                    const std::lock_guard error_lock{mutex_};
                    if (!error_) error_ = std::current_exception();
                }
                lock.lock();
                ++completed_tasks_;
                if (completed_tasks_ == task_count_) done_.notify_one();
            }
        }
    }
};

class CompositeSafePolynomialRing final {
public:
    CompositeSafePolynomialRing(
        const std::uint64_t modulus, PolynomialLedger& ledger,
        const PolynomialPolicy policy = {})
        : modulus_(modulus), ledger_(ledger), policy_(policy) {
        if (modulus_ < 2U)
            throw std::invalid_argument("polynomial modulus must be at least two");
        if (policy_.maximum_ntt_length == 0U ||
            !std::has_single_bit(policy_.maximum_ntt_length))
            throw std::invalid_argument("maximum NTT length must be a power of two");
        if (policy_.minimum_crt_primes > ntt_primes.size())
            throw std::invalid_argument("minimum CRT prime count exceeds basis");
    }

    [[nodiscard]] std::uint64_t modulus() const noexcept { return modulus_; }

    [[nodiscard]] Polynomial multiply(
        const Polynomial& left, const Polynomial& right) {
        ++ledger_.polynomial_multiplications;
        if (left.empty() || right.empty()) return {};
        if (left.size() > std::numeric_limits<std::size_t>::max() - right.size() + 1U)
            throw PolynomialResourceError("polynomial result size overflow");
        const auto result_size = left.size() + right.size() - 1U;
        ledger_.maximum_polynomial_coefficients = std::max<std::uint64_t>(
            ledger_.maximum_polynomial_coefficients, result_size);

        const auto scalar_products = static_cast<wide_uint>(left.size()) * right.size();
        if (policy_.force_schoolbook ||
            scalar_products <= policy_.schoolbook_product_limit)
            return schoolbook_multiply(left, right);
        return ntt_crt_multiply(left, right);
    }

    [[nodiscard]] Polynomial monic_remainder(
        const Polynomial& dividend, const Polynomial& divisor) {
        ++ledger_.monic_remainders;
        if (divisor.empty() || divisor.back() != 1U)
            throw std::invalid_argument("remainder divisor must be nonzero and monic");
        if (dividend.size() < divisor.size()) return dividend;

        const auto quotient_size = dividend.size() - divisor.size() + 1U;
        const auto divisor_degree = divisor.size() - 1U;
        const auto classical_cost = static_cast<wide_uint>(quotient_size) *
                                    divisor.size();
        if (policy_.force_schoolbook ||
            classical_cost <= policy_.schoolbook_product_limit)
            return classical_monic_remainder(dividend, divisor);

        Polynomial reversed_dividend(quotient_size, 0U);
        for (std::size_t index = 0U; index < quotient_size; ++index)
            reversed_dividend[index] = dividend[dividend.size() - 1U - index];

        Polynomial reversed_divisor(
            std::min(divisor.size(), quotient_size), 0U);
        for (std::size_t index = 0U; index < reversed_divisor.size(); ++index)
            reversed_divisor[index] = divisor[divisor.size() - 1U - index];

        auto inverse = series_inverse(reversed_divisor, quotient_size);
        auto reversed_quotient = multiply(reversed_dividend, inverse);
        reversed_quotient.resize(quotient_size);
        Polynomial quotient(quotient_size, 0U);
        for (std::size_t index = 0U; index < quotient_size; ++index)
            quotient[index] = reversed_quotient[quotient_size - 1U - index];

        const auto quotient_times_divisor = multiply(quotient, divisor);
        Polynomial remainder(divisor_degree, 0U);
        for (std::size_t index = 0U; index < divisor_degree; ++index) {
            const auto product_coefficient = index < quotient_times_divisor.size()
                ? quotient_times_divisor[index]
                : 0U;
            remainder[index] = subtract_mod(
                dividend[index], product_coefficient, modulus_);
        }
        trim(remainder);
        return remainder;
    }

    [[nodiscard]] std::uint64_t evaluate(
        const Polynomial& polynomial, const std::uint64_t point) noexcept {
        ++ledger_.horner_evaluations;
        ledger_.horner_coefficient_steps += polynomial.size();
        std::uint64_t value = 0U;
        for (auto cursor = polynomial.rbegin(); cursor != polynomial.rend(); ++cursor)
            value = add_mod(multiply_mod(value, point, modulus_), *cursor, modulus_);
        return value;
    }

private:
    std::uint64_t modulus_;
    PolynomialLedger& ledger_;
    PolynomialPolicy policy_;
    std::unique_ptr<NTTBatchExecutor> ntt_executor_{};

    [[nodiscard]] Polynomial schoolbook_multiply(
        const Polynomial& left, const Polynomial& right) {
        ++ledger_.schoolbook_convolutions;
        const auto products = static_cast<wide_uint>(left.size()) * right.size();
        if (products > std::numeric_limits<std::uint64_t>::max())
            ledger_.schoolbook_coefficient_products =
                std::numeric_limits<std::uint64_t>::max();
        else if (ledger_.schoolbook_coefficient_products <=
                 std::numeric_limits<std::uint64_t>::max() -
                     static_cast<std::uint64_t>(products))
            ledger_.schoolbook_coefficient_products +=
                static_cast<std::uint64_t>(products);
        else
            ledger_.schoolbook_coefficient_products =
                std::numeric_limits<std::uint64_t>::max();

        Polynomial result(left.size() + right.size() - 1U, 0U);
        for (std::size_t i = 0U; i < left.size(); ++i) {
            for (std::size_t j = 0U; j < right.size(); ++j) {
                result[i + j] = add_mod(
                    result[i + j],
                    multiply_mod(left[i], right[j], modulus_), modulus_);
            }
        }
        trim(result);
        return result;
    }

    [[nodiscard]] Polynomial classical_monic_remainder(
        const Polynomial& dividend, const Polynomial& divisor) const {
        Polynomial remainder = dividend;
        const auto divisor_degree = divisor.size() - 1U;
        for (std::size_t cursor = remainder.size(); cursor-- > divisor_degree;) {
            const auto coefficient = remainder[cursor];
            if (coefficient == 0U) continue;
            const auto shift = cursor - divisor_degree;
            for (std::size_t j = 0U; j < divisor_degree; ++j) {
                remainder[shift + j] = subtract_mod(
                    remainder[shift + j],
                    multiply_mod(coefficient, divisor[j], modulus_), modulus_);
            }
            remainder[cursor] = 0U;
        }
        remainder.resize(divisor_degree);
        trim(remainder);
        return remainder;
    }

    [[nodiscard]] Polynomial series_inverse(
        const Polynomial& polynomial, const std::size_t target_size) {
        if (target_size == 0U) return {};
        if (polynomial.empty() || polynomial[0] != 1U)
            throw std::invalid_argument("series inverse requires unit constant one");

        Polynomial inverse{1U};
        while (inverse.size() < target_size) {
            ++ledger_.newton_inverse_rounds;
            const auto next_size = std::min(target_size, inverse.size() * 2U);
            Polynomial prefix(std::min(polynomial.size(), next_size), 0U);
            std::copy_n(polynomial.begin(), prefix.size(), prefix.begin());
            auto product = multiply(prefix, inverse);
            product.resize(next_size, 0U);
            Polynomial correction(next_size, 0U);
            correction[0] = subtract_mod(2U % modulus_, product[0], modulus_);
            for (std::size_t index = 1U; index < next_size; ++index)
                correction[index] = product[index] == 0U
                    ? 0U
                    : modulus_ - product[index];
            inverse = multiply(inverse, correction);
            inverse.resize(next_size);
        }
        inverse.resize(target_size);
        return inverse;
    }

    static void ntt(
        std::vector<std::uint32_t>& values, const bool invert,
        const NTTPrime prime, PolynomialLedger& ledger) {
        const auto size = values.size();
        for (std::size_t i = 1U, j = 0U; i < size; ++i) {
            std::size_t bit = size >> 1U;
            for (; (j & bit) != 0U; bit >>= 1U) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(values[i], values[j]);
        }

        for (std::size_t length = 2U; length <= size; length <<= 1U) {
            std::uint64_t root = prime_power_mod(
                prime.primitive_root,
                (prime.modulus - 1U) / static_cast<std::uint64_t>(length),
                prime.modulus);
            if (invert)
                root = prime_power_mod(root, prime.modulus - 2U, prime.modulus);
            for (std::size_t offset = 0U; offset < size; offset += length) {
                std::uint64_t power = 1U;
                const auto half = length >> 1U;
                for (std::size_t index = 0U; index < half; ++index) {
                    const auto even = values[offset + index];
                    const auto odd = static_cast<std::uint32_t>(
                        (static_cast<std::uint64_t>(values[offset + index + half]) *
                         power) % prime.modulus);
                    const auto sum = static_cast<std::uint64_t>(even) + odd;
                    values[offset + index] = static_cast<std::uint32_t>(
                        sum >= prime.modulus ? sum - prime.modulus : sum);
                    values[offset + index + half] = static_cast<std::uint32_t>(
                        even >= odd ? even - odd : prime.modulus - (odd - even));
                    power = (power * root) % prime.modulus;
                }
            }
            ledger.ntt_butterflies += size >> 1U;
        }

        if (invert) {
            const auto inverse_size = prime_power_mod(
                size % prime.modulus, prime.modulus - 2U, prime.modulus);
            for (auto& value : values)
                value = static_cast<std::uint32_t>(
                    (static_cast<std::uint64_t>(value) * inverse_size) %
                    prime.modulus);
        }
        ++ledger.ntt_prime_transforms;
    }

    [[nodiscard]] std::size_t required_prime_count(
        const std::size_t convolution_terms) {
        ++ledger_.crt_exact_bound_checks;
        const auto coefficient_bits = static_cast<std::uint64_t>(
            std::bit_width(modulus_ - 1U));
        const auto term_bits = static_cast<std::uint64_t>(
            std::bit_width(convolution_terms));
        const auto required_bits = coefficient_bits * 2U + term_bits;
        std::uint64_t available_bits = 0U;
        for (std::size_t index = 0U; index < ntt_primes.size(); ++index) {
            available_bits += ntt_primes[index].conservative_bits;
            if (available_bits >= required_bits) {
                const auto selected = std::max<std::size_t>(
                    index + 1U, policy_.minimum_crt_primes);
                ledger_.maximum_crt_primes = std::max<std::uint64_t>(
                    ledger_.maximum_crt_primes, selected);
                return selected;
            }
        }
        throw PolynomialResourceError(
            "CRT prime product is insufficient for exact convolution");
    }

    [[nodiscard]] Polynomial ntt_crt_multiply(
        const Polynomial& left, const Polynomial& right) {
        ++ledger_.ntt_convolutions;
        const auto result_size = left.size() + right.size() - 1U;
        const auto transform_size = std::bit_ceil(result_size);
        if (transform_size > policy_.maximum_ntt_length)
            throw PolynomialResourceError("requested NTT exceeds policy limit");
        const auto transform_power = static_cast<std::uint32_t>(
            std::countr_zero(transform_size));
        ledger_.maximum_ntt_length = std::max<std::uint64_t>(
            ledger_.maximum_ntt_length, transform_size);

        const auto prime_count = required_prime_count(
            std::min(left.size(), right.size()));
        for (std::size_t prime_index = 0U;
             prime_index < prime_count; ++prime_index) {
            if (transform_power > ntt_primes[prime_index].maximum_power_of_two)
                throw PolynomialResourceError(
                    "selected CRT prime cannot support requested NTT length");
        }
        std::vector<std::vector<std::uint32_t>> residues(
            prime_count, std::vector<std::uint32_t>(result_size, 0U));
        const auto transform_for_prime = [&](const std::size_t prime_index,
                                             PolynomialLedger& local_ledger) {
            const auto prime = ntt_primes[prime_index];
            std::vector<std::uint32_t> transformed_left(transform_size, 0U);
            std::vector<std::uint32_t> transformed_right(transform_size, 0U);
            for (std::size_t index = 0U; index < left.size(); ++index)
                transformed_left[index] = static_cast<std::uint32_t>(
                    left[index] % prime.modulus);
            for (std::size_t index = 0U; index < right.size(); ++index)
                transformed_right[index] = static_cast<std::uint32_t>(
                    right[index] % prime.modulus);
            ntt(transformed_left, false, prime, local_ledger);
            ntt(transformed_right, false, prime, local_ledger);
            for (std::size_t index = 0U; index < transform_size; ++index)
                transformed_left[index] = static_cast<std::uint32_t>(
                    (static_cast<std::uint64_t>(transformed_left[index]) *
                     transformed_right[index]) % prime.modulus);
            ntt(transformed_left, true, prime, local_ledger);
            std::copy_n(transformed_left.begin(), result_size,
                        residues[prime_index].begin());
        };

        if (policy_.parallel_ntt_primes && prime_count > 1U) {
            ++ledger_.ntt_parallel_batches;
            ledger_.maximum_ntt_workers = std::max<std::uint64_t>(
                ledger_.maximum_ntt_workers, prime_count);
            std::vector<PolynomialLedger> local_ledgers(prime_count);
            if (!ntt_executor_)
                ntt_executor_ = std::make_unique<NTTBatchExecutor>(
                    ntt_primes.size());
            ntt_executor_->run(prime_count, [&](const std::size_t prime_index) {
                transform_for_prime(prime_index, local_ledgers[prime_index]);
            });
            for (const auto& local : local_ledgers) {
                ledger_.ntt_prime_transforms += local.ntt_prime_transforms;
                ledger_.ntt_butterflies += local.ntt_butterflies;
            }
        } else {
            ledger_.maximum_ntt_workers = std::max<std::uint64_t>(
                ledger_.maximum_ntt_workers, 1U);
            for (std::size_t prime_index = 0U;
                 prime_index < prime_count; ++prime_index)
                transform_for_prime(prime_index, ledger_);
        }

        std::array<std::uint64_t, ntt_primes.size()> inverse_prefix{};
        for (std::size_t i = 0U; i < prime_count; ++i) {
            const auto modulus = ntt_primes[i].modulus;
            std::uint64_t prefix = 1U;
            for (std::size_t j = 0U; j < i; ++j)
                prefix = (prefix * ntt_primes[j].modulus) % modulus;
            inverse_prefix[i] = prime_power_mod(
                prefix, modulus - 2U, modulus);
        }

        Polynomial result(result_size, 0U);
        for (std::size_t coefficient = 0U;
             coefficient < result_size; ++coefficient) {
            std::array<std::uint64_t, ntt_primes.size()> constants{};
            std::array<std::uint64_t, ntt_primes.size()> products{};
            products.fill(1U);
            std::uint64_t reconstructed = 0U;
            std::uint64_t product_mod_target = 1U % modulus_;
            for (std::size_t i = 0U; i < prime_count; ++i) {
                const auto prime = ntt_primes[i].modulus;
                const auto residue = residues[i][coefficient];
                const auto delta = residue >= constants[i]
                    ? residue - constants[i]
                    : prime - (constants[i] - residue);
                const auto digit = (delta * inverse_prefix[i]) % prime;
                reconstructed = add_mod(
                    reconstructed,
                    multiply_mod(product_mod_target, digit % modulus_, modulus_),
                    modulus_);
                product_mod_target = multiply_mod(
                    product_mod_target, ntt_primes[i].modulus % modulus_,
                    modulus_);
                for (std::size_t j = i + 1U; j < prime_count; ++j) {
                    const auto next_prime = ntt_primes[j].modulus;
                    constants[j] = (constants[j] + products[j] * digit) %
                                   next_prime;
                    products[j] = (products[j] * prime) % next_prime;
                }
            }
            result[coefficient] = reconstructed;
        }
        ledger_.crt_output_coefficients += result_size;
        ledger_.crt_mixed_radix_digits += result_size * prime_count;
        trim(result);
        return result;
    }
};

} // namespace angel::afac59
