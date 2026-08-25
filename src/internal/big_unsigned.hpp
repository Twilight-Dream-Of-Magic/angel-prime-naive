#pragma once

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace angel::detail {

__extension__ using WideUnsigned = unsigned __int128;

struct BigUnsignedLedger final {
    std::uint64_t small_multiplications{};
    std::uint64_t small_limb_updates{};
    std::uint64_t wide_multiplications{};
    std::uint64_t limb_product_accumulations{};
    std::uint64_t decimal_divisions{};
    std::uint64_t decimal_limb_updates{};
    std::uint64_t maximum_result_limbs{};
};

class BigUnsigned final {
public:
    BigUnsigned() = default;

    explicit BigUnsigned(const std::uint64_t value) {
        if (value == 0U) return;
        limbs_.push_back(static_cast<std::uint32_t>(value));
        const auto high = static_cast<std::uint32_t>(value >> 32U);
        if (high != 0U) limbs_.push_back(high);
    }

    [[nodiscard]] bool is_zero() const noexcept { return limbs_.empty(); }
    [[nodiscard]] std::size_t limb_count() const noexcept {
        return limbs_.size();
    }
    [[nodiscard]] std::size_t payload_bytes() const noexcept {
        return limbs_.size() * sizeof(std::uint32_t);
    }
    [[nodiscard]] std::size_t bit_length() const noexcept {
        if (limbs_.empty()) return 0U;
        return (limbs_.size() - 1U) * 32U +
               std::bit_width(limbs_.back());
    }

    BigUnsigned& multiply_small(
        const std::uint64_t factor, BigUnsignedLedger* ledger = nullptr) {
        if (factor == 0U) {
            limbs_.clear();
            return *this;
        }
        if (is_zero() || factor == 1U) return *this;
        WideUnsigned carry = 0U;
        for (auto& limb : limbs_) {
            const WideUnsigned current =
                static_cast<WideUnsigned>(limb) * factor + carry;
            limb = static_cast<std::uint32_t>(current);
            carry = current >> 32U;
            if (ledger) ++ledger->small_limb_updates;
        }
        while (carry != 0U) {
            limbs_.push_back(static_cast<std::uint32_t>(carry));
            carry >>= 32U;
            if (ledger) ++ledger->small_limb_updates;
        }
        if (ledger) {
            ++ledger->small_multiplications;
            ledger->maximum_result_limbs = std::max<std::uint64_t>(
                ledger->maximum_result_limbs,
                static_cast<std::uint64_t>(limbs_.size()));
        }
        return *this;
    }

    [[nodiscard]] static BigUnsigned multiply(
        const BigUnsigned& left,
        const BigUnsigned& right,
        BigUnsignedLedger* ledger = nullptr) {
        if (left.is_zero() || right.is_zero()) return BigUnsigned{};
        BigUnsigned result;
        result.limbs_.assign(
            left.limbs_.size() + right.limbs_.size() + 1U, 0U);
        for (std::size_t i = 0U; i < left.limbs_.size(); ++i) {
            WideUnsigned carry = 0U;
            for (std::size_t j = 0U; j < right.limbs_.size(); ++j) {
                const auto index = i + j;
                const WideUnsigned current =
                    static_cast<WideUnsigned>(left.limbs_[i]) *
                        right.limbs_[j] +
                    result.limbs_[index] + carry;
                result.limbs_[index] = static_cast<std::uint32_t>(current);
                carry = current >> 32U;
                if (ledger) ++ledger->limb_product_accumulations;
            }
            auto index = i + right.limbs_.size();
            while (carry != 0U) {
                const WideUnsigned current = result.limbs_[index] + carry;
                result.limbs_[index] = static_cast<std::uint32_t>(current);
                carry = current >> 32U;
                ++index;
                if (ledger) ++ledger->limb_product_accumulations;
            }
        }
        result.normalize();
        if (ledger) {
            ++ledger->wide_multiplications;
            ledger->maximum_result_limbs = std::max<std::uint64_t>(
                ledger->maximum_result_limbs,
                static_cast<std::uint64_t>(result.limbs_.size()));
        }
        return result;
    }

    [[nodiscard]] std::uint64_t modulo(
        const std::uint64_t modulus) const noexcept {
        if (modulus == 0U) return 0U;
        WideUnsigned remainder = 0U;
        for (std::size_t index = limbs_.size(); index-- > 0U;)
            remainder = ((remainder << 32U) | limbs_[index]) % modulus;
        return static_cast<std::uint64_t>(remainder);
    }

    [[nodiscard]] std::string hexadecimal() const {
        if (is_zero()) return "0";
        std::ostringstream output;
        output << std::hex << limbs_.back();
        for (std::size_t index = limbs_.size() - 1U; index-- > 0U;)
            output << std::setw(8) << std::setfill('0') << limbs_[index];
        return output.str();
    }

    [[nodiscard]] std::string decimal(BigUnsignedLedger* ledger = nullptr) const {
        if (is_zero()) return "0";
        BigUnsigned copy = *this;
        std::vector<std::uint32_t> chunks;
        while (!copy.is_zero())
            chunks.push_back(copy.divide_small(1'000'000'000U, ledger));
        std::ostringstream output;
        output << chunks.back();
        for (auto cursor = chunks.rbegin() + 1U;
             cursor != chunks.rend(); ++cursor)
            output << std::setw(9) << std::setfill('0') << *cursor;
        return output.str();
    }

    [[nodiscard]] std::uint64_t stable_hash() const noexcept {
        std::uint64_t hash = mix64(
            static_cast<std::uint64_t>(bit_length()) ^
            0x424947554e534947ULL);
        for (std::size_t index = 0U; index < limbs_.size(); ++index)
            hash = mix64(
                hash ^ std::rotl(
                    static_cast<std::uint64_t>(limbs_[index]),
                    static_cast<int>((index * 13U) & 63U)) ^
                (0x9e3779b97f4a7c15ULL * (index + 1U)));
        return hash;
    }

    friend bool operator==(const BigUnsigned&, const BigUnsigned&) = default;

private:
    std::vector<std::uint32_t> limbs_;

    static constexpr std::uint64_t mix64(std::uint64_t value) noexcept {
        value ^= value >> 30U;
        value *= 0xbf58476d1ce4e5b9ULL;
        value ^= value >> 27U;
        value *= 0x94d049bb133111ebULL;
        return value ^ (value >> 31U);
    }

    void normalize() noexcept {
        while (!limbs_.empty() && limbs_.back() == 0U) limbs_.pop_back();
    }

    std::uint32_t divide_small(
        const std::uint32_t divisor,
        BigUnsignedLedger* ledger) {
        std::uint64_t remainder = 0U;
        for (std::size_t index = limbs_.size(); index-- > 0U;) {
            const auto current = (remainder << 32U) | limbs_[index];
            limbs_[index] = static_cast<std::uint32_t>(current / divisor);
            remainder = current % divisor;
            if (ledger) ++ledger->decimal_limb_updates;
        }
        normalize();
        if (ledger) ++ledger->decimal_divisions;
        return static_cast<std::uint32_t>(remainder);
    }
};

[[nodiscard]] inline BigUnsigned factorial_sequential(
    const std::uint64_t argument,
    BigUnsignedLedger& ledger) {
    ledger = {};
    BigUnsigned result{1U};
    ledger.maximum_result_limbs = 1U;
    for (std::uint64_t factor = 2U; factor <= argument; ++factor)
        result.multiply_small(factor, &ledger);
    return result;
}

[[nodiscard]] inline BigUnsigned product_range(
    const std::uint64_t first,
    const std::uint64_t last,
    BigUnsignedLedger& ledger) {
    if (first > last) return BigUnsigned{1U};
    if (first == last) return BigUnsigned{first};
    const auto middle = first + (last - first) / 2U;
    auto left = product_range(first, middle, ledger);
    auto right = product_range(middle + 1U, last, ledger);
    return BigUnsigned::multiply(left, right, &ledger);
}

[[nodiscard]] inline BigUnsigned factorial_product_tree(
    const std::uint64_t argument,
    BigUnsignedLedger& ledger) {
    ledger = {};
    if (argument < 2U) {
        ledger.maximum_result_limbs = 1U;
        return BigUnsigned{1U};
    }
    return product_range(2U, argument, ledger);
}

} // namespace angel::detail
