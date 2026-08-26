#pragma once

#include <cstdint>
#include <map>
#include <variant>
#include <vector>

namespace angel::high {

using Coefficient = std::int64_t;
using HistoryToken = std::uint32_t;
using HistoryWord = std::vector<HistoryToken>;
using SparseAxis = std::map<HistoryWord, Coefficient>;

enum class Sector : std::uint8_t {
    Ordinary = 1,
    History = 2,
    Singular = 3
};

enum class ArithmeticOpcode : std::uint8_t {
    HAdd = 1,
    HSub = 2,
    HMul = 3,
    HDiv = 4
};

enum class ContinuationReason : std::uint8_t {
    CoefficientOverflow = 1,
    NonCentralDenominator = 2,
    ExactDivisionRequired = 3,
    FrameMismatch = 4,
    CausalCoordinateOverflow = 5
};

// A finite exact realization of Ord + Hist + Sing. History words retain the
// ordered causal path. Singular words form a two-sided ideal under HMUL.
class TriClassValue final {
public:
    TriClassValue() = default;

    [[nodiscard]] static TriClassValue ordinary(Coefficient value);
    [[nodiscard]] static TriClassValue history(
        HistoryToken token, Coefficient coefficient = 1);
    [[nodiscard]] static TriClassValue singular(
        HistoryToken token, Coefficient coefficient = 1);
    [[nodiscard]] static TriClassValue exact(
        Coefficient ordinary,
        SparseAxis history,
        SparseAxis singular);

    [[nodiscard]] Coefficient ordinary_coordinate() const noexcept {
        return ordinary_;
    }
    [[nodiscard]] const SparseAxis& history_coordinate() const noexcept {
        return history_;
    }
    [[nodiscard]] const SparseAxis& singular_coordinate() const noexcept {
        return singular_;
    }
    [[nodiscard]] bool is_zero() const noexcept;
    [[nodiscard]] bool is_pure_ordinary() const noexcept;
    [[nodiscard]] std::uint64_t seal() const noexcept;

    friend bool operator==(const TriClassValue&, const TriClassValue&) = default;

private:
    Coefficient ordinary_{};
    SparseAxis history_;
    SparseAxis singular_;

    TriClassValue(
        Coefficient ordinary,
        SparseAxis history,
        SparseAxis singular);
};

struct ArithmeticContinuation final {
    ArithmeticOpcode attempted{ArithmeticOpcode::HAdd};
    ContinuationReason reason{ContinuationReason::CoefficientOverflow};
    TriClassValue left;
    TriClassValue right;
    std::uint64_t left_parent_seal{};
    std::uint64_t right_parent_seal{};
    std::uint64_t certificate_seal{};
    bool operands_retained{};
    bool no_false_scalar_collapse{};
    bool future_live{};

    friend bool operator==(
        const ArithmeticContinuation&,
        const ArithmeticContinuation&) = default;
};

using ArithmeticResult = std::variant<TriClassValue, ArithmeticContinuation>;

struct SingularPayload final {
    TriClassValue unresolved;
    bool typed_as_singular_payload{};

    friend bool operator==(const SingularPayload&, const SingularPayload&) = default;
};

struct DivisionPacket final {
    TriClassValue numerator;
    TriClassValue denominator;
    TriClassValue quotient;
    SingularPayload residual;
    std::uint64_t numerator_seal{};
    std::uint64_t denominator_seal{};
    std::uint64_t certificate_seal{};
    bool reconstruction_verified{};

    friend bool operator==(const DivisionPacket&, const DivisionPacket&) = default;
};

using DivisionResult = std::variant<DivisionPacket, ArithmeticContinuation>;

// HADD  X boxplus_A Y
[[nodiscard]] ArithmeticResult hadd(
    const TriClassValue& left,
    const TriClassValue& right);

// HSUB  X boxminus_A Y
[[nodiscard]] ArithmeticResult hsub(
    const TriClassValue& left,
    const TriClassValue& right);

// HMUL  X boxtimes_A Y. Ordered history concatenation makes the complete
// algebra generally noncommutative even when the ordinary shadow is not.
[[nodiscard]] ArithmeticResult hmul(
    const TriClassValue& left,
    const TriClassValue& right);

// HDIV  X oslash_A Y. The executable chart is total for central scalar
// denominators: it returns quotient plus an exact residual payload. General
// noncentral denominators remain live continuations.
[[nodiscard]] DivisionResult hdiv(
    const TriClassValue& numerator,
    const TriClassValue& denominator);

[[nodiscard]] bool verify(const ArithmeticContinuation& continuation) noexcept;
[[nodiscard]] bool verify(const DivisionPacket& packet);

} // namespace angel::high
