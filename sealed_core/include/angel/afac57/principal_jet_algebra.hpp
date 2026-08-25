#pragma once

#include "angel/afac56/log_native.hpp"
#include "angel/r23/future_quotient.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac57 {

using angel::afac56::BigNat;

// R57 works with the exact normalized interval jet
//
//   G_[a,b](T) = T^{-(b-a)} product_{k=a+1}^b (1-(1+T)^k).
//
// It does not identify this object with the complete Taylor series of an
// arbitrary input.  Closure is admitted only where the factor intervals prove
// it syntactically.

struct FusionLedger final {
    std::uint64_t coordinate_bit_operations{};
    std::uint64_t interval_checks{};
    std::uint64_t certificate_checks{};
    std::uint64_t continuations_minted{};
    std::uint64_t denotation_events_visited{};
    angel::afac56::NativeLedger r56{};

    [[nodiscard]] std::uint64_t total_native_steps() const noexcept {
        return coordinate_bit_operations + interval_checks + certificate_checks +
               continuations_minted + r56.total_steps();
    }
};

[[nodiscard]] inline BigNat subtract_exact(
    const BigNat& left, const BigNat& right, FusionLedger* ledger = nullptr) {
    if (left < right) throw std::invalid_argument("negative BigNat subtraction");
    if (left == right) return BigNat{};

    const auto bits = left.bit_length();
    std::vector<unsigned char> nibbles((bits + 3U) / 4U, 0U);
    bool borrow = false;
    for (std::size_t i = 0U; i < bits; ++i) {
        const int a = left.bit(i) ? 1 : 0;
        const int b = right.bit(i) ? 1 : 0;
        int digit = a - b - (borrow ? 1 : 0);
        if (digit < 0) {
            digit += 2;
            borrow = true;
        } else {
            borrow = false;
        }
        if (digit != 0)
            nibbles[i / 4U] = static_cast<unsigned char>(
                nibbles[i / 4U] | (1U << static_cast<unsigned>(i % 4U)));
        if (ledger) ++ledger->coordinate_bit_operations;
    }
    if (borrow) throw std::logic_error("BigNat subtraction borrow escaped");
    constexpr char hex[] = "0123456789abcdef";
    std::string text;
    text.reserve(nibbles.size());
    bool started = false;
    for (std::size_t i = nibbles.size(); i-- > 0U;) {
        if (nibbles[i] != 0U || started) {
            text.push_back(hex[nibbles[i]]);
            started = true;
        }
    }
    return BigNat::from_hex(text.empty() ? "0" : text);
}

enum class PrincipalJetOpcode : std::uint8_t {
    UploadInterval,
    ImportR56FactorialJet,
    HMulContiguous,
    HDivUnit,
    HDivCancelPrefix,
    HDivCancelSuffix,
    HSubReflexiveZero,
    HigherJetContinuation
};

enum class ContinuationReason : std::uint8_t {
    NonContiguousFactorIntervals,
    DenominatorNotCertifiedFactor,
    NonReflexiveInterferenceNeedsHigherJet
};

struct PrincipalJetCertificate final {
    PrincipalJetOpcode opcode{PrincipalJetOpcode::UploadInterval};
    std::uint64_t left_parent_seal{};
    std::uint64_t right_parent_seal{};
    std::uint64_t r56_jet_seal{};
    std::uint64_t interval_seal{};
    std::uint64_t valuation_seal{};
    std::uint64_t certificate_seal{};
    bool integer_polynomial_identity{};
    bool exact_interval_factorization{};
    bool exact_principal_coefficient{};
    bool future_live{};

    friend bool operator==(const PrincipalJetCertificate&,
                           const PrincipalJetCertificate&) = default;
};

class CertifiedPrincipalJetState final {
public:
    CertifiedPrincipalJetState() = delete;
    CertifiedPrincipalJetState(const CertifiedPrincipalJetState&) = default;
    CertifiedPrincipalJetState(CertifiedPrincipalJetState&&) noexcept = default;
    CertifiedPrincipalJetState& operator=(const CertifiedPrincipalJetState&) = default;
    CertifiedPrincipalJetState& operator=(CertifiedPrincipalJetState&&) noexcept = default;

    [[nodiscard]] const angel::afac56::CertifiedQpochProgram& source_program() const noexcept {
        return source_program_;
    }
    [[nodiscard]] const angel::afac56::EvaluatedNormalizedJetEffect& effect() const noexcept {
        return effect_;
    }
    [[nodiscard]] const BigNat& source() const noexcept {
        return effect_.leading_interval().source();
    }
    [[nodiscard]] const BigNat& target() const noexcept {
        return effect_.leading_interval().target();
    }
    [[nodiscard]] const BigNat& valuation() const noexcept { return effect_.valuation(); }
    [[nodiscard]] bool negative_principal_coefficient() const noexcept {
        return valuation().bit(0U);
    }
    [[nodiscard]] bool is_unit() const noexcept { return valuation().is_zero(); }
    [[nodiscard]] const PrincipalJetCertificate& certificate() const noexcept {
        return certificate_;
    }
    [[nodiscard]] std::uint64_t seal() const noexcept {
        return certificate_.certificate_seal;
    }
    [[nodiscard]] std::size_t payload_bytes() const noexcept {
        return source_program_.program().metrics().owned_payload_bytes +
               source().exact_payload_bytes() + target().exact_payload_bytes() +
               valuation().exact_payload_bytes() + sizeof(certificate_);
    }
    [[nodiscard]] bool self_contained() const noexcept {
        return effect_.self_contained();
    }

private:
    angel::afac56::CertifiedQpochProgram source_program_;
    angel::afac56::EvaluatedNormalizedJetEffect effect_;
    PrincipalJetCertificate certificate_;

    CertifiedPrincipalJetState(
        angel::afac56::CertifiedQpochProgram source_program,
        angel::afac56::EvaluatedNormalizedJetEffect effect,
        PrincipalJetCertificate certificate)
        : source_program_(std::move(source_program)), effect_(std::move(effect)),
          certificate_(certificate) {}

    friend class PrincipalJetMachine;
    friend class PrincipalJetVerifier;
};

struct ExactZeroJet final {
    std::uint64_t left_parent_seal{};
    std::uint64_t right_parent_seal{};
    std::uint64_t certificate_seal{};
    bool exact_all_coefficients_zero{};

    friend bool operator==(const ExactZeroJet&, const ExactZeroJet&) = default;
};

struct HigherJetContinuation final {
    PrincipalJetOpcode attempted_opcode{PrincipalJetOpcode::HigherJetContinuation};
    ContinuationReason reason{ContinuationReason::NonContiguousFactorIntervals};
    std::uint64_t left_parent_seal{};
    std::uint64_t right_parent_seal{};
    BigNat required_jet_order;
    std::uint64_t certificate_seal{};
    bool operands_retained_by_binding{};
    bool no_false_scalar_collapse{};
    bool future_live{};

    friend bool operator==(const HigherJetContinuation&,
                           const HigherJetContinuation&) = default;
};

using HMulResult = std::variant<CertifiedPrincipalJetState, HigherJetContinuation>;
using HDivResult = std::variant<CertifiedPrincipalJetState, HigherJetContinuation>;
using HSubResult = std::variant<ExactZeroJet, HigherJetContinuation>;

[[nodiscard]] inline bool same_principal_jet(
    const CertifiedPrincipalJetState& left,
    const CertifiedPrincipalJetState& right) noexcept {
    if (left.is_unit() && right.is_unit()) return true;
    return left.source() == right.source() && left.target() == right.target() &&
           left.valuation() == right.valuation();
}

[[nodiscard]] inline std::uint64_t make_state_seal(
    const PrincipalJetOpcode opcode,
    const angel::afac56::EvaluatedNormalizedJetEffect& effect,
    const std::uint64_t left_parent,
    const std::uint64_t right_parent) noexcept {
    return angel::afac56::mix64(
        static_cast<std::uint64_t>(opcode) ^
        std::rotl(effect.certificate().certificate_seal, 5) ^
        std::rotl(effect.leading_interval().seal(), 17) ^
        std::rotl(effect.valuation().stable_hash(), 29) ^
        std::rotl(left_parent, 37) ^ std::rotl(right_parent, 47) ^
        0x4146414335374a54ULL);
}

class PrincipalJetMachine final {
public:
    [[nodiscard]] CertifiedPrincipalJetState upload_interval(
        const BigNat& source, const BigNat& length,
        FusionLedger* ledger = nullptr) const {
        return compile(source, length, PrincipalJetOpcode::UploadInterval, 0U, 0U,
                       ledger);
    }

    [[nodiscard]] CertifiedPrincipalJetState import_r56_factorial_jet(
        const BigNat& input, const angel::afac56::AFACExecution& execution,
        FusionLedger* ledger = nullptr) const {
        const auto report = angel::afac56::IndependentVerifier::verify(
            input, execution.source_program, execution.jet_effect,
            execution.result, ledger ? &ledger->r56 : nullptr);
        if (!report.accepted())
            throw std::invalid_argument("R56 factorial jet failed independent replay");
        return compile(BigNat{}, input, PrincipalJetOpcode::ImportR56FactorialJet,
                       execution.jet_effect.certificate().certificate_seal, 0U,
                       ledger);
    }

    [[nodiscard]] HMulResult hmul(
        const CertifiedPrincipalJetState& left,
        const CertifiedPrincipalJetState& right,
        FusionLedger* ledger = nullptr) const {
        if (ledger) ++ledger->interval_checks;
        if (left.target() != right.source()) {
            return continuation(PrincipalJetOpcode::HMulContiguous,
                                ContinuationReason::NonContiguousFactorIntervals,
                                left, right, max_plus_one(left.valuation(),
                                                         right.valuation(), ledger),
                                ledger);
        }
        auto length = BigNat::add(left.valuation(), right.valuation(),
                                  ledger ? &ledger->r56.limb_operations : nullptr);
        return compile(left.source(), length, PrincipalJetOpcode::HMulContiguous,
                       left.seal(), right.seal(), ledger);
    }

    [[nodiscard]] HDivResult hdiv(
        const CertifiedPrincipalJetState& numerator,
        const CertifiedPrincipalJetState& denominator,
        FusionLedger* ledger = nullptr) const {
        if (ledger) ++ledger->interval_checks;
        if (denominator.is_unit()) {
            return compile(numerator.source(), numerator.valuation(),
                           PrincipalJetOpcode::HDivUnit,
                           numerator.seal(), denominator.seal(), ledger);
        }
        if (numerator.source() == denominator.source() &&
            denominator.target() <= numerator.target()) {
            auto length = subtract_exact(numerator.valuation(),
                                         denominator.valuation(), ledger);
            return compile(denominator.target(), length,
                           PrincipalJetOpcode::HDivCancelPrefix,
                           numerator.seal(), denominator.seal(), ledger);
        }
        if (numerator.target() == denominator.target() &&
            numerator.source() <= denominator.source()) {
            auto length = subtract_exact(numerator.valuation(),
                                         denominator.valuation(), ledger);
            return compile(numerator.source(), length,
                           PrincipalJetOpcode::HDivCancelSuffix,
                           numerator.seal(), denominator.seal(), ledger);
        }
        return continuation(PrincipalJetOpcode::HDivCancelPrefix,
                            ContinuationReason::DenominatorNotCertifiedFactor,
                            numerator, denominator,
                            max_plus_one(numerator.valuation(),
                                         denominator.valuation(), ledger), ledger);
    }

    [[nodiscard]] HSubResult hsub(
        const CertifiedPrincipalJetState& left,
        const CertifiedPrincipalJetState& right,
        FusionLedger* ledger = nullptr) const {
        if (ledger) ++ledger->interval_checks;
        if (same_principal_jet(left, right)) {
            ExactZeroJet zero{};
            zero.left_parent_seal = left.seal();
            zero.right_parent_seal = right.seal();
            zero.exact_all_coefficients_zero = true;
            zero.certificate_seal = angel::afac56::mix64(
                std::rotl(left.seal(), 11) ^ std::rotl(right.seal(), 31) ^
                0x485355425a45524fULL);
            if (ledger) ledger->certificate_checks += 3U;
            return zero;
        }
        return continuation(PrincipalJetOpcode::HSubReflexiveZero,
                            ContinuationReason::NonReflexiveInterferenceNeedsHigherJet,
                            left, right,
                            max_plus_one(left.valuation(), right.valuation(), ledger),
                            ledger);
    }

private:
    [[nodiscard]] static BigNat max_plus_one(
        const BigNat& left, const BigNat& right, FusionLedger* ledger) {
        const auto& maximum = left < right ? right : left;
        return maximum.incremented(
            ledger ? &ledger->r56.limb_operations : nullptr);
    }

    [[nodiscard]] static HigherJetContinuation continuation(
        const PrincipalJetOpcode attempted,
        const ContinuationReason reason,
        const CertifiedPrincipalJetState& left,
        const CertifiedPrincipalJetState& right,
        BigNat required_order,
        FusionLedger* ledger) {
        HigherJetContinuation out{};
        out.attempted_opcode = attempted;
        out.reason = reason;
        out.left_parent_seal = left.seal();
        out.right_parent_seal = right.seal();
        out.required_jet_order = std::move(required_order);
        out.operands_retained_by_binding = true;
        out.no_false_scalar_collapse = true;
        out.future_live = true;
        out.certificate_seal = angel::afac56::mix64(
            static_cast<std::uint64_t>(attempted) ^
            std::rotl(static_cast<std::uint64_t>(reason), 9) ^
            std::rotl(out.left_parent_seal, 21) ^
            std::rotl(out.right_parent_seal, 39) ^
            std::rotl(out.required_jet_order.stable_hash(), 51) ^
            0x434f4e54494e5535ULL);
        if (ledger) {
            ++ledger->continuations_minted;
            ledger->certificate_checks += 4U;
        }
        return out;
    }

    [[nodiscard]] static CertifiedPrincipalJetState compile(
        const BigNat& source, const BigNat& length,
        const PrincipalJetOpcode opcode,
        const std::uint64_t left_parent,
        const std::uint64_t right_parent,
        FusionLedger* ledger) {
        auto program = angel::afac56::build_qpoch_program(
            length, ledger ? &ledger->r56 : nullptr);
        auto certified = angel::afac56::certify_qpoch_program(
            std::move(program), ledger ? &ledger->r56 : nullptr);
        if (!certified) throw std::logic_error("R57 internal q-Pochhammer rejection");
        auto effect = angel::afac56::execute_normalized_jet(
            *certified, source, ledger ? &ledger->r56 : nullptr);

        PrincipalJetCertificate certificate{};
        certificate.opcode = opcode;
        certificate.left_parent_seal = left_parent;
        certificate.right_parent_seal = right_parent;
        certificate.r56_jet_seal = effect.certificate().certificate_seal;
        certificate.interval_seal = effect.leading_interval().seal();
        certificate.valuation_seal = effect.valuation().stable_hash();
        certificate.integer_polynomial_identity = true;
        certificate.exact_interval_factorization = true;
        certificate.exact_principal_coefficient = true;
        certificate.future_live = true;
        certificate.certificate_seal = make_state_seal(
            opcode, effect, left_parent, right_parent);
        if (ledger) ledger->certificate_checks += 8U;
        return CertifiedPrincipalJetState{
            std::move(*certified), std::move(effect), certificate};
    }
};

struct PrincipalJetVerification final {
    bool qpoch_program_valid{};
    bool interval_and_valuation_valid{};
    bool principal_sign_valid{};
    bool certificate_valid{};
    bool accepted{};
};

class PrincipalJetVerifier final {
public:
    [[nodiscard]] static PrincipalJetVerification verify(
        const CertifiedPrincipalJetState& state,
        FusionLedger* ledger = nullptr) {
        PrincipalJetVerification out{};
        out.qpoch_program_valid = angel::afac56::verify_qpoch_program(
            state.source_program_.program(), ledger ? &ledger->r56 : nullptr).accepted();
        const auto expected_target = BigNat::add(
            state.source(), state.source_program_.program().length(),
            ledger ? &ledger->r56.limb_operations : nullptr);
        out.interval_and_valuation_valid =
            state.target() == expected_target &&
            state.valuation() == state.source_program_.program().length();
        out.principal_sign_valid = state.negative_principal_coefficient() ==
                                   state.valuation().bit(0U);
        const auto& cert = state.certificate_;
        out.certificate_valid =
            cert.r56_jet_seal == state.effect_.certificate().certificate_seal &&
            cert.interval_seal == state.effect_.leading_interval().seal() &&
            cert.valuation_seal == state.effect_.valuation().stable_hash() &&
            cert.integer_polynomial_identity && cert.exact_interval_factorization &&
            cert.exact_principal_coefficient && cert.future_live &&
            cert.certificate_seal == make_state_seal(
                cert.opcode, state.effect_, cert.left_parent_seal,
                cert.right_parent_seal);
        out.accepted = out.qpoch_program_valid &&
                       out.interval_and_valuation_valid &&
                       out.principal_sign_valid && out.certificate_valid;
        if (ledger) ledger->certificate_checks += 8U;
        return out;
    }

    [[nodiscard]] static bool verify(const ExactZeroJet& zero) noexcept {
        return zero.exact_all_coefficients_zero &&
               zero.certificate_seal == angel::afac56::mix64(
                   std::rotl(zero.left_parent_seal, 11) ^
                   std::rotl(zero.right_parent_seal, 31) ^
                   0x485355425a45524fULL);
    }

    [[nodiscard]] static bool verify(const HigherJetContinuation& packet) noexcept {
        const auto expected = angel::afac56::mix64(
            static_cast<std::uint64_t>(packet.attempted_opcode) ^
            std::rotl(static_cast<std::uint64_t>(packet.reason), 9) ^
            std::rotl(packet.left_parent_seal, 21) ^
            std::rotl(packet.right_parent_seal, 39) ^
            std::rotl(packet.required_jet_order.stable_hash(), 51) ^
            0x434f4e54494e5535ULL);
        return packet.certificate_seal == expected &&
               packet.operands_retained_by_binding &&
               packet.no_false_scalar_collapse && packet.future_live;
    }
};

struct FutureQuotientBridge final {
    angel::r23::FutureLanguageProgram language;
    angel::r23::CertifiedFutureQuotient quotient;
    angel::r23::QuotientVerification verification;
};

[[nodiscard]] inline FutureQuotientBridge compile_scoped_future_quotient() {
    angel::r23::FutureLanguageProgram language{};
    language.observer_names = {"principal", "unit", "zero", "continuation"};
    language.operator_names = {"HMUL_VALID", "HDIV_VALID", "HSUB_SELF", "UNRESOLVED"};
    // The first two states are deliberately observational/future equivalent.
    // The compiler, not the caller, decides whether to merge them.
    language.states = {
        {"principal-a", {1U, 0U, 0U, 0U}, {0U, 2U, 3U, 4U}},
        {"principal-alias", {1U, 0U, 0U, 0U}, {1U, 2U, 3U, 4U}},
        {"unit", {0U, 1U, 0U, 0U}, {0U, 2U, 3U, 4U}},
        {"exact-zero", {0U, 0U, 1U, 0U}, {3U, 3U, 3U, 3U}},
        {"higher-continuation", {0U, 0U, 0U, 1U}, {4U, 4U, 4U, 4U}},
    };
    angel::r23::FutureQuotientCompiler compiler;
    auto quotient = compiler.compile(language);
    auto verification = compiler.verify(language, quotient);
    return FutureQuotientBridge{
        std::move(language), std::move(quotient), std::move(verification)};
}

struct AFAC57Execution final {
    angel::afac56::AFACExecution r56_factorial;
    CertifiedPrincipalJetState principal_jet;
    FusionLedger fusion_ledger;
};

// Direct factorial-to-consumable-state entry point.  The R56 scalar result and
// the R57 principal jet are independently certified; neither is an ordinary
// binary materialization of n!.
[[nodiscard]] inline AFAC57Execution AFAC57_LOG_NATIVE(const BigNat& n) {
    auto r56 = angel::afac56::AFAC_LOG_NATIVE(n);
    FusionLedger ledger{};
    PrincipalJetMachine machine;
    auto state = machine.import_r56_factorial_jet(n, r56, &ledger);
    return AFAC57Execution{std::move(r56), std::move(state), ledger};
}

struct OrdinaryBinaryInteger final {};
OrdinaryBinaryInteger implicit_binary_projection(
    const CertifiedPrincipalJetState&) = delete;
CertifiedPrincipalJetState promote_continuation_without_execution(
    const HigherJetContinuation&) = delete;

static_assert(!std::is_default_constructible_v<CertifiedPrincipalJetState>);
static_assert(!std::is_convertible_v<CertifiedPrincipalJetState,
                                     OrdinaryBinaryInteger>);
static_assert(!std::is_convertible_v<HigherJetContinuation,
                                     CertifiedPrincipalJetState>);

} // namespace angel::afac57
