#pragma once

#include "angel/afac57/principal_jet_algebra.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac58 {

__extension__ using wide_uint = unsigned __int128;

enum class WilsonBackend : std::uint8_t {
    StrictSerial,
    StrictParallel
};

struct WilsonPolicy final {
    WilsonBackend backend{WilsonBackend::StrictSerial};
    std::uint32_t worker_count{1U};
    std::uint64_t max_factor_visits{100'000'000U};

    [[nodiscard]] static WilsonPolicy strict_serial(
        const std::uint64_t max_factors = 100'000'000U) noexcept {
        return WilsonPolicy{WilsonBackend::StrictSerial, 1U, max_factors};
    }

    [[nodiscard]] static WilsonPolicy strict_parallel(
        const std::uint32_t workers,
        const std::uint64_t max_factors = 100'000'000U) noexcept {
        return WilsonPolicy{WilsonBackend::StrictParallel,
                            std::max<std::uint32_t>(1U, workers), max_factors};
    }
};

struct WilsonBoundaryLedger final {
    std::uint64_t angel_input_bits{};
    std::uint64_t angel_native_steps{};
    std::uint64_t angel_state_payload_bytes{};
    std::uint64_t ordinary_factor_visits{};
    std::uint64_t ordinary_modular_multiplications{};
    std::uint64_t ordinary_block_combinations{};
    std::uint64_t ordinary_elapsed_nanoseconds{};
    std::uint32_t workers_used{};
    bool ordinary_projection_executed{};
    bool ordinary_result_fed_back_to_angel{};

    friend bool operator==(const WilsonBoundaryLedger&,
                           const WilsonBoundaryLedger&) = default;
};

class CertifiedWilsonRequest final {
public:
    CertifiedWilsonRequest() = delete;
    CertifiedWilsonRequest(const CertifiedWilsonRequest&) = default;
    CertifiedWilsonRequest(CertifiedWilsonRequest&&) noexcept = default;
    CertifiedWilsonRequest& operator=(const CertifiedWilsonRequest&) = default;
    CertifiedWilsonRequest& operator=(CertifiedWilsonRequest&&) noexcept = default;

    [[nodiscard]] std::uint64_t candidate() const noexcept { return candidate_; }
    [[nodiscard]] const angel::afac57::AFAC57Execution& factorial_execution() const noexcept {
        return factorial_execution_;
    }
    [[nodiscard]] std::uint64_t binding_seal() const noexcept { return binding_seal_; }

private:
    std::uint64_t candidate_{};
    angel::afac57::AFAC57Execution factorial_execution_;
    std::uint64_t binding_seal_{};

    CertifiedWilsonRequest(
        const std::uint64_t candidate,
        angel::afac57::AFAC57Execution factorial_execution,
        const std::uint64_t binding_seal)
        : candidate_(candidate),
          factorial_execution_(std::move(factorial_execution)),
          binding_seal_(binding_seal) {}

    friend class WilsonBoundary;
    friend class WilsonRequestVerifier;
};

[[nodiscard]] inline std::uint64_t request_binding_seal(
    const std::uint64_t candidate,
    const angel::afac57::CertifiedPrincipalJetState& state) noexcept {
    return angel::afac56::mix64(
        candidate ^ std::rotl(candidate - 1U, 13) ^
        std::rotl(state.seal(), 31) ^
        std::rotl(state.source_program().program().seal(), 47) ^
        0x57494c534f4e3538ULL);
}

struct WilsonRequestVerification final {
    bool candidate_domain_valid{};
    bool r57_state_valid{};
    bool source_zero{};
    bool target_is_candidate_minus_one{};
    bool valuation_is_candidate_minus_one{};
    bool binding_valid{};
    bool accepted{};
};

class WilsonRequestVerifier final {
public:
    [[nodiscard]] static WilsonRequestVerification verify(
        const CertifiedWilsonRequest& request) {
        WilsonRequestVerification out{};
        out.candidate_domain_valid = request.candidate_ >= 2U;
        const auto& state = request.factorial_execution_.principal_jet;
        out.r57_state_valid =
            angel::afac57::PrincipalJetVerifier::verify(state).accepted;
        out.source_zero = state.source().is_zero();
        const angel::afac56::BigNat expected{request.candidate_ - 1U};
        out.target_is_candidate_minus_one = state.target() == expected;
        out.valuation_is_candidate_minus_one = state.valuation() == expected;
        out.binding_valid = request.binding_seal_ ==
                            request_binding_seal(request.candidate_, state);
        out.accepted = out.candidate_domain_valid && out.r57_state_valid &&
                       out.source_zero && out.target_is_candidate_minus_one &&
                       out.valuation_is_candidate_minus_one && out.binding_valid;
        return out;
    }
};

class WilsonObservation final {
public:
    WilsonObservation() = delete;

    [[nodiscard]] std::uint64_t candidate() const noexcept { return candidate_; }
    [[nodiscard]] std::uint64_t residue() const noexcept { return residue_; }
    [[nodiscard]] bool is_prime() const noexcept { return prime_; }
    [[nodiscard]] const WilsonBoundaryLedger& ledger() const noexcept { return ledger_; }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }

private:
    std::uint64_t candidate_{};
    std::uint64_t residue_{};
    bool prime_{};
    WilsonBoundaryLedger ledger_{};
    std::uint64_t request_binding_{};
    std::uint64_t seal_{};

    WilsonObservation(
        const std::uint64_t candidate, const std::uint64_t residue,
        const bool prime, WilsonBoundaryLedger ledger,
        const std::uint64_t request_binding, const std::uint64_t seal)
        : candidate_(candidate), residue_(residue), prime_(prime),
          ledger_(ledger), request_binding_(request_binding), seal_(seal) {}

    friend class WilsonResidueObserver;
    friend class WilsonObservationVerifier;
};

struct WilsonResourceLimit final {
    std::uint64_t candidate{};
    std::uint64_t required_factor_visits{};
    std::uint64_t allowed_factor_visits{};
    std::uint64_t request_binding{};
    WilsonBoundaryLedger ledger{};
    std::uint64_t seal{};

    friend bool operator==(const WilsonResourceLimit&,
                           const WilsonResourceLimit&) = default;
};

using WilsonDownloadResult =
    std::variant<WilsonObservation, WilsonResourceLimit>;

[[nodiscard]] inline std::uint64_t multiply_mod(
    const std::uint64_t left, const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return static_cast<std::uint64_t>(
        (static_cast<wide_uint>(left) * right) % modulus);
}

[[nodiscard]] inline std::uint64_t boundary_ledger_seal(
    const WilsonBoundaryLedger& ledger) noexcept {
    return angel::afac56::mix64(
        ledger.angel_input_bits ^
        std::rotl(ledger.angel_native_steps, 7) ^
        std::rotl(ledger.angel_state_payload_bytes, 13) ^
        std::rotl(ledger.ordinary_factor_visits, 19) ^
        std::rotl(ledger.ordinary_modular_multiplications, 29) ^
        std::rotl(ledger.ordinary_block_combinations, 37) ^
        std::rotl(ledger.ordinary_elapsed_nanoseconds, 43) ^
        std::rotl(static_cast<std::uint64_t>(ledger.workers_used), 53) ^
        std::rotl(static_cast<std::uint64_t>(ledger.ordinary_projection_executed), 59) ^
        std::rotl(static_cast<std::uint64_t>(
            ledger.ordinary_result_fed_back_to_angel), 61) ^
        0x4c45444745523538ULL);
}

[[nodiscard]] inline std::uint64_t observation_seal(
    const std::uint64_t candidate, const std::uint64_t residue,
    const bool prime, const std::uint64_t request_binding,
    const WilsonBoundaryLedger& ledger) noexcept {
    return angel::afac56::mix64(
        candidate ^ std::rotl(residue, 9) ^
        std::rotl(static_cast<std::uint64_t>(prime), 19) ^
        std::rotl(request_binding, 31) ^
        std::rotl(boundary_ledger_seal(ledger), 43) ^
        0x57494c534f4e4f42ULL);
}

class WilsonBoundary final {
public:
    [[nodiscard]] CertifiedWilsonRequest bind(
        const std::uint64_t candidate) const {
        if (candidate < 2U)
            throw std::invalid_argument("Wilson candidate must be at least two");
        auto factorial = angel::afac57::AFAC57_LOG_NATIVE(
            angel::afac56::BigNat{candidate - 1U});
        const auto seal = request_binding_seal(candidate, factorial.principal_jet);
        return CertifiedWilsonRequest{candidate, std::move(factorial), seal};
    }
};

class WilsonResidueObserver final {
public:
    [[nodiscard]] WilsonDownloadResult download(
        const CertifiedWilsonRequest& request,
        const WilsonPolicy policy = WilsonPolicy::strict_serial()) const {
        const auto verification = WilsonRequestVerifier::verify(request);
        if (!verification.accepted)
            throw std::invalid_argument("Wilson request failed independent binding replay");

        WilsonBoundaryLedger ledger{};
        const auto& execution = request.factorial_execution();
        ledger.angel_input_bits = execution.principal_jet.valuation().bit_length();
        ledger.angel_native_steps =
            execution.r56_factorial.ledger.total_steps() +
            execution.fusion_ledger.total_native_steps();
        ledger.angel_state_payload_bytes = execution.principal_jet.payload_bytes();
        ledger.ordinary_result_fed_back_to_angel = false;

        const auto factor_count = request.candidate() - 1U;
        if (factor_count > policy.max_factor_visits) {
            WilsonResourceLimit limit{};
            limit.candidate = request.candidate();
            limit.required_factor_visits = factor_count;
            limit.allowed_factor_visits = policy.max_factor_visits;
            limit.request_binding = request.binding_seal();
            limit.ledger = ledger;
            limit.seal = angel::afac56::mix64(
                limit.candidate ^ std::rotl(limit.required_factor_visits, 17) ^
                std::rotl(limit.allowed_factor_visits, 37) ^
                std::rotl(limit.request_binding, 51) ^
                std::rotl(boundary_ledger_seal(limit.ledger), 59) ^
                0x5245534c494d4954ULL);
            return limit;
        }

        const auto start = std::chrono::steady_clock::now();
        std::uint64_t residue = 1U % request.candidate();
        if (policy.backend == WilsonBackend::StrictSerial ||
            policy.worker_count <= 1U || factor_count <= 1U) {
            ledger.workers_used = 1U;
            for (std::uint64_t factor = 1U; factor <= factor_count; ++factor) {
                residue = multiply_mod(residue, factor, request.candidate());
                ++ledger.ordinary_factor_visits;
                ++ledger.ordinary_modular_multiplications;
            }
        } else {
            const auto workers = static_cast<std::uint32_t>(
                std::min<std::uint64_t>(policy.worker_count, factor_count));
            ledger.workers_used = workers;
            std::vector<std::uint64_t> partial(workers, 1U);
            std::vector<std::uint64_t> visits(workers, 0U);
            std::vector<std::thread> threads;
            threads.reserve(workers);
            for (std::uint32_t worker = 0U; worker < workers; ++worker) {
                const auto begin = 1U +
                    static_cast<std::uint64_t>(
                        (static_cast<wide_uint>(factor_count) * worker) / workers);
                const auto end = static_cast<std::uint64_t>(
                    (static_cast<wide_uint>(factor_count) * (worker + 1U)) / workers);
                threads.emplace_back([&, worker, begin, end] {
                    std::uint64_t value = 1U;
                    std::uint64_t count = 0U;
                    for (std::uint64_t factor = begin; factor <= end; ++factor) {
                        value = multiply_mod(value, factor, request.candidate());
                        ++count;
                    }
                    partial[worker] = value;
                    visits[worker] = count;
                });
            }
            for (auto& thread : threads) thread.join();
            for (std::uint32_t worker = 0U; worker < workers; ++worker) {
                residue = multiply_mod(residue, partial[worker], request.candidate());
                ledger.ordinary_factor_visits += visits[worker];
                ledger.ordinary_modular_multiplications += visits[worker] + 1U;
                ++ledger.ordinary_block_combinations;
            }
        }
        const auto stop = std::chrono::steady_clock::now();
        ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count());
        ledger.ordinary_projection_executed = true;

        const bool prime = residue == request.candidate() - 1U;
        const auto seal = observation_seal(
            request.candidate(), residue, prime, request.binding_seal(), ledger);
        return WilsonObservation{request.candidate(), residue, prime, ledger,
                                 request.binding_seal(), seal};
    }
};

struct WilsonObservationVerification final {
    bool request_valid{};
    bool binding_valid{};
    bool ledger_boundary_valid{};
    bool decision_matches_wilson{};
    bool seal_valid{};
    bool accepted{};
};

class WilsonObservationVerifier final {
public:
    [[nodiscard]] static WilsonObservationVerification verify(
        const CertifiedWilsonRequest& request,
        const WilsonObservation& observation) noexcept {
        WilsonObservationVerification out{};
        out.request_valid = WilsonRequestVerifier::verify(request).accepted;
        out.binding_valid = observation.candidate_ == request.candidate() &&
                            observation.request_binding_ == request.binding_seal();
        out.ledger_boundary_valid =
            observation.ledger_.ordinary_projection_executed &&
            !observation.ledger_.ordinary_result_fed_back_to_angel &&
            observation.ledger_.ordinary_factor_visits == request.candidate() - 1U;
        out.decision_matches_wilson =
            observation.prime_ ==
                (observation.residue_ == observation.candidate_ - 1U);
        out.seal_valid = observation.seal_ == observation_seal(
            observation.candidate_, observation.residue_, observation.prime_,
            observation.request_binding_, observation.ledger_);
        out.accepted = out.request_valid && out.binding_valid &&
                       out.ledger_boundary_valid && out.decision_matches_wilson &&
                       out.seal_valid;
        return out;
    }

    [[nodiscard]] static bool verify(
        const CertifiedWilsonRequest& request,
        const WilsonResourceLimit& limit) noexcept {
        const auto expected = angel::afac56::mix64(
            limit.candidate ^ std::rotl(limit.required_factor_visits, 17) ^
            std::rotl(limit.allowed_factor_visits, 37) ^
            std::rotl(limit.request_binding, 51) ^
            std::rotl(boundary_ledger_seal(limit.ledger), 59) ^
            0x5245534c494d4954ULL);
        return WilsonRequestVerifier::verify(request).accepted &&
               limit.candidate == request.candidate() &&
               limit.required_factor_visits == request.candidate() - 1U &&
               limit.required_factor_visits > limit.allowed_factor_visits &&
               limit.request_binding == request.binding_seal() &&
               !limit.ledger.ordinary_projection_executed &&
               !limit.ledger.ordinary_result_fed_back_to_angel &&
               limit.seal == expected;
    }
};

struct WilsonApplicationResult final {
    CertifiedWilsonRequest request;
    WilsonDownloadResult download;
};

[[nodiscard]] inline WilsonApplicationResult ANGEL_WILSON_PRIME(
    const std::uint64_t candidate,
    const WilsonPolicy policy = WilsonPolicy::strict_serial()) {
    WilsonBoundary boundary;
    WilsonResidueObserver observer;
    auto request = boundary.bind(candidate);
    auto result = observer.download(request, policy);
    return WilsonApplicationResult{std::move(request), std::move(result)};
}

struct AngelRunningState final {};
AngelRunningState feed_wilson_observation_back(const WilsonObservation&) = delete;
WilsonObservation implicit_download(const CertifiedWilsonRequest&) = delete;

static_assert(!std::is_default_constructible_v<CertifiedWilsonRequest>);
static_assert(!std::is_default_constructible_v<WilsonObservation>);
static_assert(!std::is_convertible_v<WilsonObservation, AngelRunningState>);

} // namespace angel::afac58
