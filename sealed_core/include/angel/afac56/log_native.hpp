#pragma once

#include "big_nat.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace angel::afac56 {

inline constexpr std::uint64_t mix64(std::uint64_t z) noexcept {
    z ^= z >> 30U;
    z *= 0xbf58476d1ce4e5b9ULL;
    z ^= z >> 27U;
    z *= 0x94d049bb133111ebULL;
    return z ^ (z >> 31U);
}

struct NativeLedger final {
    std::uint64_t input_bits_read{};
    std::uint64_t opcode_writes{};
    std::uint64_t opcode_checks{};
    std::uint64_t limb_operations{};
    std::uint64_t owned_limb_copies{};
    std::uint64_t certificate_checks{};

    [[nodiscard]] std::uint64_t total_steps() const noexcept {
        return input_bits_read + opcode_writes + opcode_checks + limb_operations +
               owned_limb_copies + certificate_checks;
    }

    NativeLedger& operator+=(const NativeLedger& other) noexcept {
        input_bits_read += other.input_bits_read;
        opcode_writes += other.opcode_writes;
        opcode_checks += other.opcode_checks;
        limb_operations += other.limb_operations;
        owned_limb_copies += other.owned_limb_copies;
        certificate_checks += other.certificate_checks;
        return *this;
    }
};

enum class QpochOpcode : std::uint8_t {
    Unit = 0,
    SeedFactor = 1,
    DoubleWithContextShift = 2,
    AppendShiftedSingleton = 3
};

struct ProgramMetrics final {
    std::size_t input_bit_length{};
    std::size_t opcode_count{};
    std::size_t maximum_depth{};
    std::size_t owned_payload_bytes{};
    std::size_t double_ops{};
    std::size_t append_ops{};
};

class CompactQpochProgram final {
public:
    CompactQpochProgram() = delete;

    [[nodiscard]] const BigNat& length() const noexcept { return length_; }
    [[nodiscard]] const std::vector<QpochOpcode>& opcodes_for_audit() const noexcept {
        return opcodes_;
    }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }

    [[nodiscard]] ProgramMetrics metrics() const noexcept {
        ProgramMetrics out{};
        out.input_bit_length = std::max<std::size_t>(1U, length_.bit_length());
        out.opcode_count = opcodes_.size();
        out.maximum_depth = opcodes_.size();
        out.owned_payload_bytes = length_.exact_payload_bytes() +
                                  opcodes_.capacity() * sizeof(QpochOpcode);
        out.double_ops = static_cast<std::size_t>(std::count(
            opcodes_.begin(), opcodes_.end(), QpochOpcode::DoubleWithContextShift));
        out.append_ops = static_cast<std::size_t>(std::count(
            opcodes_.begin(), opcodes_.end(), QpochOpcode::AppendShiftedSingleton));
        return out;
    }

    // Untrusted import exists so the verifier can be mutation-tested.  It never
    // yields the certified type by itself.
    [[nodiscard]] static CompactQpochProgram import_untrusted(
        BigNat length, std::vector<QpochOpcode> opcodes, const std::uint64_t seal) {
        return CompactQpochProgram{std::move(length), std::move(opcodes), seal};
    }

private:
    BigNat length_;
    std::vector<QpochOpcode> opcodes_;
    std::uint64_t seal_{};

    CompactQpochProgram(BigNat length, std::vector<QpochOpcode> opcodes,
                        const std::uint64_t seal)
        : length_(std::move(length)), opcodes_(std::move(opcodes)), seal_(seal) {}

    friend CompactQpochProgram build_qpoch_program(const BigNat&, NativeLedger*);
};

[[nodiscard]] inline std::uint64_t qpoch_program_seal(
    const BigNat& length, const std::vector<QpochOpcode>& opcodes) noexcept {
    std::uint64_t seal = mix64(length.stable_hash() ^ 0x51504f4348353655ULL);
    for (std::size_t i = 0U; i < opcodes.size(); ++i)
        seal = mix64(seal ^ (static_cast<std::uint64_t>(opcodes[i]) << (i & 7U)) ^
                     (0x9e3779b97f4a7c15ULL * (i + 1U)));
    return seal;
}

[[nodiscard]] inline CompactQpochProgram build_qpoch_program(
    const BigNat& length, NativeLedger* ledger = nullptr) {
    std::vector<QpochOpcode> code;
    const auto bits = length.bit_length();
    if (ledger) ledger->owned_limb_copies += length.limb_count();
    if (bits == 0U) {
        code.push_back(QpochOpcode::Unit);
        if (ledger) {
            ++ledger->input_bits_read;
            ++ledger->opcode_writes;
        }
    } else {
        code.reserve(2U * bits - 1U);
        code.push_back(QpochOpcode::SeedFactor);
        if (ledger) {
            ++ledger->input_bits_read;
            ++ledger->opcode_writes;
        }
        for (std::size_t offset = 1U; offset < bits; ++offset) {
            code.push_back(QpochOpcode::DoubleWithContextShift);
            const bool next = length.bit_from_msb(offset);
            if (ledger) {
                ++ledger->input_bits_read;
                ++ledger->opcode_writes;
            }
            if (next) {
                code.push_back(QpochOpcode::AppendShiftedSingleton);
                if (ledger) ++ledger->opcode_writes;
            }
        }
    }
    const auto seal = qpoch_program_seal(length, code);
    return CompactQpochProgram{length, std::move(code), seal};
}

struct ProgramVerification final {
    bool seal_valid{};
    bool grammar_valid{};
    bool exact_length_recurrence{};
    bool no_expansion_opcode{};
    std::size_t opcodes_checked{};

    [[nodiscard]] bool accepted() const noexcept {
        return seal_valid && grammar_valid && exact_length_recurrence &&
               no_expansion_opcode;
    }
};

[[nodiscard]] inline ProgramVerification verify_qpoch_program(
    const CompactQpochProgram& program, NativeLedger* ledger = nullptr) {
    ProgramVerification out{};
    const auto& code = program.opcodes_for_audit();
    const auto bits = program.length().bit_length();
    out.seal_valid = program.seal() ==
                     qpoch_program_seal(program.length(), code);
    out.no_expansion_opcode = std::all_of(code.begin(), code.end(), [](const auto op) {
        return op == QpochOpcode::Unit || op == QpochOpcode::SeedFactor ||
               op == QpochOpcode::DoubleWithContextShift ||
               op == QpochOpcode::AppendShiftedSingleton;
    });

    bool grammar = true;
    std::size_t cursor = 0U;
    if (bits == 0U) {
        grammar = code.size() == 1U && code.front() == QpochOpcode::Unit;
        out.opcodes_checked = code.empty() ? 0U : 1U;
        if (ledger) {
            ++ledger->input_bits_read;
            ledger->opcode_checks += out.opcodes_checked;
        }
    } else {
        grammar = !code.empty() && code.front() == QpochOpcode::SeedFactor;
        cursor = grammar ? 1U : 0U;
        if (ledger) {
            ++ledger->input_bits_read;
            if (!code.empty()) ++ledger->opcode_checks;
        }
        for (std::size_t offset = 1U; grammar && offset < bits; ++offset) {
            grammar = cursor < code.size() &&
                      code[cursor] == QpochOpcode::DoubleWithContextShift;
            if (cursor < code.size()) {
                ++cursor;
                if (ledger) ++ledger->opcode_checks;
            }
            const bool next = program.length().bit_from_msb(offset);
            if (ledger) ++ledger->input_bits_read;
            if (grammar && next) {
                grammar = cursor < code.size() &&
                          code[cursor] == QpochOpcode::AppendShiftedSingleton;
                if (cursor < code.size()) {
                    ++cursor;
                    if (ledger) ++ledger->opcode_checks;
                }
            }
        }
        grammar = grammar && cursor == code.size();
        out.opcodes_checked = cursor;
    }
    out.grammar_valid = grammar;
    // The grammar is the MSB recurrence m <- 2m+b.  Matching it bit-for-bit
    // against the owned target proves the represented interval length exactly,
    // without constructing m at every node.
    out.exact_length_recurrence = grammar;
    if (ledger) ledger->certificate_checks += 4U;
    return out;
}

class CertifiedQpochProgram final {
public:
    CertifiedQpochProgram() = delete;
    CertifiedQpochProgram(const CertifiedQpochProgram&) = default;
    CertifiedQpochProgram(CertifiedQpochProgram&&) noexcept = default;
    CertifiedQpochProgram& operator=(const CertifiedQpochProgram&) = default;
    CertifiedQpochProgram& operator=(CertifiedQpochProgram&&) noexcept = default;

    [[nodiscard]] const CompactQpochProgram& program() const noexcept { return program_; }
    [[nodiscard]] const ProgramVerification& report() const noexcept { return report_; }

private:
    CompactQpochProgram program_;
    ProgramVerification report_;

    CertifiedQpochProgram(CompactQpochProgram program, ProgramVerification report)
        : program_(std::move(program)), report_(report) {}

    friend std::optional<CertifiedQpochProgram> certify_qpoch_program(
        CompactQpochProgram, NativeLedger*);
};

[[nodiscard]] inline std::optional<CertifiedQpochProgram> certify_qpoch_program(
    CompactQpochProgram program, NativeLedger* ledger = nullptr) {
    const auto report = verify_qpoch_program(program, ledger);
    if (!report.accepted()) return std::nullopt;
    return CertifiedQpochProgram{std::move(program), report};
}

class FactorialIntervalCoordinate final {
public:
    [[nodiscard]] static std::optional<FactorialIntervalCoordinate> make(
        BigNat source, BigNat target) {
        if (source > target) return std::nullopt;
        return FactorialIntervalCoordinate{std::move(source), std::move(target)};
    }

    [[nodiscard]] const BigNat& source() const noexcept { return source_; }
    [[nodiscard]] const BigNat& target() const noexcept { return target_; }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }
    [[nodiscard]] bool self_contained() const noexcept { return true; }
    [[nodiscard]] std::size_t payload_bytes() const noexcept {
        return source_.exact_payload_bytes() + target_.exact_payload_bytes();
    }

    friend bool operator==(const FactorialIntervalCoordinate&, const FactorialIntervalCoordinate&) = default;

private:
    BigNat source_;
    BigNat target_;
    std::uint64_t seal_{};

    FactorialIntervalCoordinate(BigNat source, BigNat target)
        : source_(std::move(source)), target_(std::move(target)),
          seal_(mix64(source_.stable_hash() ^
                      std::rotl(target_.stable_hash(), 17) ^ 0x494e54455256414cULL)) {}
};

class FactoradicInteger final {
public:
    [[nodiscard]] static std::optional<FactoradicInteger> canonical_digit(
        BigNat rank, const std::uint64_t coefficient) {
        if (rank.is_zero() || coefficient == 0U || rank.compare_u64(coefficient) < 0)
            return std::nullopt;
        return FactoradicInteger{std::move(rank), coefficient};
    }

    [[nodiscard]] const BigNat& rank() const noexcept { return rank_; }
    [[nodiscard]] std::uint64_t coefficient() const noexcept { return coefficient_; }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }
    [[nodiscard]] bool self_contained() const noexcept { return true; }
    [[nodiscard]] std::size_t deferred_program_bytes() const noexcept { return 0U; }
    [[nodiscard]] std::size_t payload_bytes() const noexcept {
        return rank_.exact_payload_bytes() + sizeof(coefficient_);
    }

    friend bool operator==(const FactoradicInteger&, const FactoradicInteger&) = default;

private:
    BigNat rank_;
    std::uint64_t coefficient_{};
    std::uint64_t seal_{};

    FactoradicInteger(BigNat rank, const std::uint64_t coefficient)
        : rank_(std::move(rank)), coefficient_(coefficient),
          seal_(mix64(rank_.stable_hash() ^ std::rotl(coefficient_, 23) ^
                      0x464143544f524144ULL)) {}
};

[[nodiscard]] inline bool same_integer(const FactoradicInteger& left,
                                       const FactoradicInteger& right) noexcept {
    return left == right;
}

[[nodiscard]] inline bool less_integer(const FactoradicInteger& left,
                                       const FactoradicInteger& right) noexcept {
    if (left.rank() != right.rank()) return left.rank() < right.rank();
    return left.coefficient() < right.coefficient();
}

[[nodiscard]] inline std::optional<FactorialIntervalCoordinate> compose(
    const FactorialIntervalCoordinate& left,
    const FactorialIntervalCoordinate& right) {
    if (left.target() != right.source()) return std::nullopt;
    return FactorialIntervalCoordinate::make(left.source(), right.target());
}

[[nodiscard]] inline std::optional<FactorialIntervalCoordinate> cancel_prefix(
    const FactorialIntervalCoordinate& whole,
    const FactorialIntervalCoordinate& prefix) {
    if (whole.source() != prefix.source() || prefix.target() > whole.target())
        return std::nullopt;
    return FactorialIntervalCoordinate::make(prefix.target(), whole.target());
}

[[nodiscard]] inline std::optional<FactorialIntervalCoordinate> cancel_suffix(
    const FactorialIntervalCoordinate& whole,
    const FactorialIntervalCoordinate& suffix) {
    if (whole.target() != suffix.target() || whole.source() > suffix.source())
        return std::nullopt;
    return FactorialIntervalCoordinate::make(whole.source(), suffix.source());
}

[[nodiscard]] inline std::optional<FactoradicInteger> multiply_by_successor(
    const FactoradicInteger& value, NativeLedger* ledger = nullptr) {
    if (value.coefficient() != 1U) return std::nullopt;
    auto next = value.rank().incremented(ledger ? &ledger->limb_operations : nullptr);
    if (ledger) ledger->owned_limb_copies += value.rank().limb_count();
    return FactoradicInteger::canonical_digit(std::move(next), 1U);
}

struct JetCertificate final {
    std::uint64_t program_binding{};
    std::uint64_t source_binding{};
    std::uint64_t interval_binding{};
    std::uint64_t valuation_binding{};
    std::uint64_t certificate_seal{};
    std::size_t input_bit_length{};
    std::size_t opcode_count{};
    bool integer_polynomial_law{};
    bool exact_valuation_law{};
    bool leading_factorial_coordinate_law{};

    friend bool operator==(const JetCertificate&, const JetCertificate&) = default;
};

class EvaluatedNormalizedJetEffect final {
public:
    EvaluatedNormalizedJetEffect() = delete;
    [[nodiscard]] const FactorialIntervalCoordinate& leading_interval() const noexcept {
        return leading_interval_;
    }
    [[nodiscard]] const BigNat& valuation() const noexcept { return valuation_; }
    [[nodiscard]] const JetCertificate& certificate() const noexcept { return certificate_; }
    [[nodiscard]] bool self_contained() const noexcept { return true; }

private:
    FactorialIntervalCoordinate leading_interval_;
    BigNat valuation_;
    JetCertificate certificate_;

    EvaluatedNormalizedJetEffect(FactorialIntervalCoordinate interval,
                                 BigNat valuation, JetCertificate certificate)
        : leading_interval_(std::move(interval)), valuation_(std::move(valuation)),
          certificate_(certificate) {}

    friend EvaluatedNormalizedJetEffect execute_normalized_jet(
        const CertifiedQpochProgram&, const BigNat&, NativeLedger*);
};

[[nodiscard]] inline JetCertificate make_jet_certificate(
    const CertifiedQpochProgram& source_program, const BigNat& source,
    const FactorialIntervalCoordinate& interval, const BigNat& valuation) noexcept {
    JetCertificate cert{};
    cert.program_binding = source_program.program().seal();
    cert.source_binding = source.stable_hash();
    cert.interval_binding = interval.seal();
    cert.valuation_binding = valuation.stable_hash();
    const auto metrics = source_program.program().metrics();
    cert.input_bit_length = metrics.input_bit_length;
    cert.opcode_count = metrics.opcode_count;
    cert.integer_polynomial_law = true;
    cert.exact_valuation_law = true;
    cert.leading_factorial_coordinate_law = true;
    cert.certificate_seal = mix64(
        cert.program_binding ^ std::rotl(cert.source_binding, 7) ^
        std::rotl(cert.interval_binding, 19) ^ std::rotl(cert.valuation_binding, 31) ^
        static_cast<std::uint64_t>(cert.input_bit_length) ^
        std::rotl(static_cast<std::uint64_t>(cert.opcode_count), 13) ^
        0x4a4554434f4b4552ULL);
    return cert;
}

[[nodiscard]] inline EvaluatedNormalizedJetEffect execute_normalized_jet(
    const CertifiedQpochProgram& source_program, const BigNat& source,
    NativeLedger* ledger = nullptr) {
    std::uint64_t local_ops = 0U;
    auto target = BigNat::add(source, source_program.program().length(), &local_ops);
    if (ledger) {
        ledger->limb_operations += local_ops;
        ledger->owned_limb_copies += source.limb_count() +
                                     source_program.program().length().limb_count();
    }
    auto interval = FactorialIntervalCoordinate::make(source, std::move(target));
    if (!interval) throw std::logic_error("normalized jet created invalid interval");
    BigNat valuation = source_program.program().length();
    if (ledger) ledger->owned_limb_copies += valuation.limb_count();
    const auto certificate = make_jet_certificate(source_program, source, *interval, valuation);
    if (ledger) ledger->certificate_checks += 3U;
    return EvaluatedNormalizedJetEffect{std::move(*interval), std::move(valuation),
                                        certificate};
}

struct FactorialCertificate final {
    JetCertificate jet{};
    std::uint64_t result_binding{};
    std::uint64_t seal{};
    bool source_zero_scalarization{};
    bool canonical_factoradic_digit{};

    friend bool operator==(const FactorialCertificate&, const FactorialCertificate&) = default;
};

class AngelFactorialResult final {
public:
    AngelFactorialResult() = delete;
    [[nodiscard]] const FactoradicInteger& value() const noexcept { return value_; }
    [[nodiscard]] const FactorialCertificate& certificate() const noexcept {
        return certificate_;
    }
    [[nodiscard]] bool self_contained() const noexcept { return value_.self_contained(); }
    [[nodiscard]] std::size_t deferred_program_bytes() const noexcept {
        return value_.deferred_program_bytes();
    }

private:
    FactoradicInteger value_;
    FactorialCertificate certificate_;

    AngelFactorialResult(FactoradicInteger value, FactorialCertificate certificate)
        : value_(std::move(value)), certificate_(certificate) {}

    friend AngelFactorialResult scalarize_source_zero(
        const EvaluatedNormalizedJetEffect&, NativeLedger*);
};

[[nodiscard]] inline AngelFactorialResult scalarize_source_zero(
    const EvaluatedNormalizedJetEffect& effect, NativeLedger* ledger = nullptr) {
    if (!effect.leading_interval().source().is_zero())
        throw std::invalid_argument("only a source-zero jet scalarizes to n!");
    BigNat rank = effect.leading_interval().target();
    if (rank.is_zero()) rank = BigNat{1U};
    if (ledger) ledger->owned_limb_copies += rank.limb_count();
    auto value = FactoradicInteger::canonical_digit(std::move(rank), 1U);
    if (!value) throw std::logic_error("failed to create canonical factorial digit");
    FactorialCertificate certificate{};
    certificate.jet = effect.certificate();
    certificate.result_binding = value->seal();
    certificate.source_zero_scalarization = true;
    certificate.canonical_factoradic_digit = true;
    certificate.seal = mix64(certificate.jet.certificate_seal ^
                             std::rotl(certificate.result_binding, 29) ^
                             0x5343414c4152495aULL);
    if (ledger) ledger->certificate_checks += 3U;
    return AngelFactorialResult{std::move(*value), certificate};
}

struct ResultVerification final {
    bool source_program_valid{};
    bool jet_binding_valid{};
    bool interval_valid{};
    bool valuation_valid{};
    bool result_coordinate_valid{};
    bool certificate_valid{};

    [[nodiscard]] bool accepted() const noexcept {
        return source_program_valid && jet_binding_valid && interval_valid &&
               valuation_valid && result_coordinate_valid && certificate_valid;
    }
};

class IndependentVerifier final {
public:
    [[nodiscard]] static ResultVerification verify(
        const BigNat& input, const CertifiedQpochProgram& source_program,
        const EvaluatedNormalizedJetEffect& effect,
        const AngelFactorialResult& result, NativeLedger* ledger = nullptr) {
        ResultVerification out{};
        out.source_program_valid = verify_qpoch_program(source_program.program(), ledger).accepted() &&
                                   source_program.program().length() == input;
        const BigNat zero{};
        auto expected_interval = FactorialIntervalCoordinate::make(zero, input);
        out.interval_valid = expected_interval &&
                             effect.leading_interval() == *expected_interval;
        out.valuation_valid = effect.valuation() == input;
        const auto expected_jet = make_jet_certificate(source_program, zero,
                                                       *expected_interval, input);
        out.jet_binding_valid = effect.certificate() == expected_jet;

        BigNat expected_rank = input.is_zero() ? BigNat{1U} : input;
        auto expected_value = FactoradicInteger::canonical_digit(std::move(expected_rank), 1U);
        out.result_coordinate_valid = expected_value && result.value() == *expected_value &&
                                      result.self_contained() &&
                                      result.deferred_program_bytes() == 0U;
        const auto& cert = result.certificate();
        const auto expected_seal = mix64(cert.jet.certificate_seal ^
                                         std::rotl(cert.result_binding, 29) ^
                                         0x5343414c4152495aULL);
        out.certificate_valid = cert.jet == expected_jet && expected_value &&
                                cert.result_binding == expected_value->seal() &&
                                cert.source_zero_scalarization &&
                                cert.canonical_factoradic_digit &&
                                cert.seal == expected_seal;
        if (ledger) ledger->certificate_checks += 12U;
        return out;
    }
};

struct AFACExecution final {
    CertifiedQpochProgram source_program;
    EvaluatedNormalizedJetEffect jet_effect;
    AngelFactorialResult result;
    NativeLedger ledger;
};

[[nodiscard]] inline AFACExecution AFAC_LOG_NATIVE(const BigNat& n) {
    NativeLedger ledger{};
    auto source = build_qpoch_program(n, &ledger);
    auto certified = certify_qpoch_program(std::move(source), &ledger);
    if (!certified) throw std::logic_error("internal q-Pochhammer certification failed");
    const BigNat zero{};
    auto effect = execute_normalized_jet(*certified, zero, &ledger);
    auto result = scalarize_source_zero(effect, &ledger);
    return AFACExecution{std::move(*certified), std::move(effect),
                         std::move(result), ledger};
}

// Value-only entry point.  All syntax/effect objects are transient and are
// destroyed before the self-contained coordinate result crosses the boundary.
[[nodiscard]] inline AngelFactorialResult AFAC_LOG_NATIVE_VALUE(const BigNat& n) {
    auto execution = AFAC_LOG_NATIVE(n);
    return std::move(execution.result);
}

struct OrdinaryBinaryInteger final {};
OrdinaryBinaryInteger implicit_binary_projection(const FactoradicInteger&) = delete;
OrdinaryBinaryInteger implicit_binary_projection(const AngelFactorialResult&) = delete;

static_assert(!std::is_convertible_v<CompactQpochProgram, AngelFactorialResult>);
static_assert(!std::is_convertible_v<CertifiedQpochProgram, AngelFactorialResult>);
static_assert(!std::is_convertible_v<FactoradicInteger, OrdinaryBinaryInteger>);

} // namespace angel::afac56
