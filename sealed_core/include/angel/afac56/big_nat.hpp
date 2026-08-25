#pragma once

#include <algorithm>
#include <bit>
#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace angel::afac56 {

__extension__ using wide_uint = unsigned __int128;

// Minimal owned natural-number carrier.  AFAC56 needs only input-coordinate
// operations (bit access, comparison, addition and increment); it deliberately
// contains no factorial or general multiplication routine.
class BigNat final {
public:
    BigNat() = default;

    explicit BigNat(const std::uint64_t value) {
        if (value != 0U) limbs_.push_back(value);
    }

    [[nodiscard]] static BigNat from_hex(std::string_view text) {
        if (text.starts_with("0x") || text.starts_with("0X")) text.remove_prefix(2U);
        std::string digits;
        digits.reserve(text.size());
        for (const char ch : text) {
            if (ch == '_') continue;
            if (!std::isxdigit(static_cast<unsigned char>(ch)))
                throw std::invalid_argument("invalid hexadecimal natural");
            digits.push_back(ch);
        }
        const auto first = digits.find_first_not_of('0');
        if (first == std::string::npos) return BigNat{};
        digits.erase(0U, first);

        BigNat out;
        for (std::size_t end = digits.size(); end != 0U;) {
            const std::size_t begin = end > 16U ? end - 16U : 0U;
            std::uint64_t limb = 0U;
            for (std::size_t i = begin; i < end; ++i) {
                limb = (limb << 4U) | hex_value(digits[i]);
            }
            out.limbs_.push_back(limb);
            end = begin;
        }
        out.normalize();
        return out;
    }

    [[nodiscard]] static BigNat one_at_bit(const std::size_t bit_index) {
        BigNat out;
        out.limbs_.assign(bit_index / 64U + 1U, 0U);
        out.limbs_[bit_index / 64U] =
            std::uint64_t{1} << static_cast<unsigned>(bit_index % 64U);
        return out;
    }

    [[nodiscard]] static BigNat all_ones(const std::size_t bit_count) {
        if (bit_count == 0U) return BigNat{};
        BigNat out;
        out.limbs_.assign((bit_count + 63U) / 64U, ~std::uint64_t{0});
        const auto remainder = static_cast<unsigned>(bit_count % 64U);
        if (remainder != 0U)
            out.limbs_.back() = (std::uint64_t{1} << remainder) - 1U;
        return out;
    }

    [[nodiscard]] bool is_zero() const noexcept { return limbs_.empty(); }
    [[nodiscard]] bool is_one() const noexcept {
        return limbs_.size() == 1U && limbs_.front() == 1U;
    }

    [[nodiscard]] std::size_t limb_count() const noexcept { return limbs_.size(); }
    [[nodiscard]] std::size_t payload_bytes() const noexcept {
        return limbs_.capacity() * sizeof(std::uint64_t);
    }
    [[nodiscard]] std::size_t exact_payload_bytes() const noexcept {
        return limbs_.size() * sizeof(std::uint64_t);
    }

    [[nodiscard]] std::size_t bit_length() const noexcept {
        if (limbs_.empty()) return 0U;
        return (limbs_.size() - 1U) * 64U + std::bit_width(limbs_.back());
    }

    [[nodiscard]] bool bit(const std::size_t bit_index) const noexcept {
        const auto limb = bit_index / 64U;
        return limb < limbs_.size() &&
               ((limbs_[limb] >> static_cast<unsigned>(bit_index % 64U)) & 1U) != 0U;
    }

    [[nodiscard]] bool bit_from_msb(const std::size_t offset) const {
        const auto bits = bit_length();
        if (offset >= bits) throw std::out_of_range("BigNat MSB offset");
        return bit(bits - 1U - offset);
    }

    [[nodiscard]] bool fits_u64() const noexcept { return limbs_.size() <= 1U; }
    [[nodiscard]] std::uint64_t to_u64() const {
        if (!fits_u64()) throw std::overflow_error("BigNat does not fit uint64");
        return limbs_.empty() ? 0U : limbs_.front();
    }

    [[nodiscard]] std::string hex() const {
        if (limbs_.empty()) return "0";
        constexpr char digits[] = "0123456789abcdef";
        std::string output;
        output.reserve((bit_length() + 3U) / 4U);
        bool started = false;
        for (std::size_t nibble = (bit_length() + 3U) / 4U; nibble-- > 0U;) {
            unsigned value = 0U;
            for (unsigned b = 0U; b < 4U; ++b)
                if (bit(nibble * 4U + b)) value |= 1U << b;
            if (value != 0U || started) {
                output.push_back(digits[value]);
                started = true;
            }
        }
        return output.empty() ? "0" : output;
    }

    [[nodiscard]] std::uint64_t stable_hash() const noexcept {
        std::uint64_t h = mix64(static_cast<std::uint64_t>(bit_length()) ^
                                0x4249474e41543536ULL);
        for (std::size_t i = 0U; i < limbs_.size(); ++i)
            h = mix64(h ^ std::rotl(limbs_[i], static_cast<int>((11U * i) & 63U)) ^
                      (0x9e3779b97f4a7c15ULL * (i + 1U)));
        return h;
    }

    [[nodiscard]] BigNat incremented(std::uint64_t* limb_operations = nullptr) const {
        BigNat out = *this;
        std::uint64_t carry = 1U;
        for (auto& limb : out.limbs_) {
            if (limb_operations) ++*limb_operations;
            const auto old = limb;
            limb += carry;
            carry = limb < old ? 1U : 0U;
            if (carry == 0U) return out;
        }
        if (carry != 0U) out.limbs_.push_back(1U);
        return out;
    }

    [[nodiscard]] static BigNat add(const BigNat& left, const BigNat& right,
                                    std::uint64_t* limb_operations = nullptr) {
        BigNat out;
        const auto count = std::max(left.limbs_.size(), right.limbs_.size());
        out.limbs_.assign(count, 0U);
        wide_uint carry = 0U;
        for (std::size_t i = 0U; i < count; ++i) {
            if (limb_operations) ++*limb_operations;
            const wide_uint a = i < left.limbs_.size() ? left.limbs_[i] : 0U;
            const wide_uint b = i < right.limbs_.size() ? right.limbs_[i] : 0U;
            const auto sum = a + b + carry;
            out.limbs_[i] = static_cast<std::uint64_t>(sum);
            carry = sum >> 64U;
        }
        if (carry != 0U) out.limbs_.push_back(static_cast<std::uint64_t>(carry));
        return out;
    }

    [[nodiscard]] int compare_u64(const std::uint64_t value) const noexcept {
        if (limbs_.size() > 1U) return 1;
        const auto mine = limbs_.empty() ? 0U : limbs_.front();
        return mine < value ? -1 : (mine > value ? 1 : 0);
    }

    friend bool operator==(const BigNat&, const BigNat&) = default;
    friend std::strong_ordering operator<=>(const BigNat& left,
                                             const BigNat& right) noexcept {
        if (left.limbs_.size() != right.limbs_.size())
            return left.limbs_.size() < right.limbs_.size()
                       ? std::strong_ordering::less
                       : std::strong_ordering::greater;
        for (std::size_t i = left.limbs_.size(); i-- > 0U;) {
            if (left.limbs_[i] != right.limbs_[i])
                return left.limbs_[i] < right.limbs_[i]
                           ? std::strong_ordering::less
                           : std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

private:
    std::vector<std::uint64_t> limbs_;

    static constexpr std::uint64_t mix64(std::uint64_t z) noexcept {
        z ^= z >> 30U;
        z *= 0xbf58476d1ce4e5b9ULL;
        z ^= z >> 27U;
        z *= 0x94d049bb133111ebULL;
        return z ^ (z >> 31U);
    }

    static std::uint64_t hex_value(const char ch) {
        if (ch >= '0' && ch <= '9') return static_cast<std::uint64_t>(ch - '0');
        if (ch >= 'a' && ch <= 'f') return static_cast<std::uint64_t>(10 + ch - 'a');
        if (ch >= 'A' && ch <= 'F') return static_cast<std::uint64_t>(10 + ch - 'A');
        throw std::invalid_argument("invalid hexadecimal digit");
    }

    void normalize() noexcept {
        while (!limbs_.empty() && limbs_.back() == 0U) limbs_.pop_back();
    }
};

} // namespace angel::afac56
