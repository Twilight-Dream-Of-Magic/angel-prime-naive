#pragma once

#include "angel/afac60/external_quotient_view.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac61 {

// R61 does not replace or compress the R57/R60 Angel state.  It binds the
// certified q-Pochhammer factorial program to a new *external* cyclic frame.
// The reference evaluator is an observer-side denotation replay only.

struct SelfCyclicProbeLedger final {
    std::uint64_t qpoch_factors_applied{};
    std::uint64_t cyclic_coefficient_updates{};
    std::uint64_t normalized_window_updates{};
    std::uint64_t polynomial_gcd_coefficient_steps{};
    std::uint64_t audit_number_theory_steps{};
    std::uint64_t maximum_live_coefficients{};
    std::uint64_t angel_nodes_rewritten{};
    std::uint64_t angel_nodes_merged{};
    bool ordinary_result_fed_back_to_angel{};

    friend bool operator==(const SelfCyclicProbeLedger&,
                           const SelfCyclicProbeLedger&) = default;
};

class CertifiedSelfCyclicActionView final {
public:
    CertifiedSelfCyclicActionView() = delete;
    CertifiedSelfCyclicActionView(const CertifiedSelfCyclicActionView&) = default;
    CertifiedSelfCyclicActionView(CertifiedSelfCyclicActionView&&) noexcept = default;
    CertifiedSelfCyclicActionView& operator=(const CertifiedSelfCyclicActionView&) = default;
    CertifiedSelfCyclicActionView& operator=(CertifiedSelfCyclicActionView&&) noexcept = default;

    [[nodiscard]] std::uint64_t order() const noexcept { return order_; }
    [[nodiscard]] std::uint64_t represented_factor_count() const noexcept {
        return represented_factor_count_;
    }
    [[nodiscard]] std::uint64_t request_binding() const noexcept {
        return request_binding_;
    }
    [[nodiscard]] std::uint64_t principal_state_seal() const noexcept {
        return principal_state_seal_;
    }
    [[nodiscard]] std::uint64_t source_program_seal() const noexcept {
        return source_program_seal_;
    }
    [[nodiscard]] std::uint64_t valuation_hash() const noexcept {
        return valuation_hash_;
    }
    [[nodiscard]] std::uint64_t view_seal() const noexcept { return view_seal_; }

    [[nodiscard]] bool contains_angel_state() const noexcept { return false; }
    [[nodiscard]] bool compresses_angel_state() const noexcept { return false; }
    [[nodiscard]] bool can_feed_back_to_angel() const noexcept { return false; }
    [[nodiscard]] std::size_t descriptor_words() const noexcept { return 7U; }

private:
    std::uint64_t order_{};
    std::uint64_t represented_factor_count_{};
    std::uint64_t request_binding_{};
    std::uint64_t principal_state_seal_{};
    std::uint64_t source_program_seal_{};
    std::uint64_t valuation_hash_{};
    std::uint64_t view_seal_{};

    CertifiedSelfCyclicActionView(
        const std::uint64_t order,
        const std::uint64_t represented_factor_count,
        const std::uint64_t request_binding,
        const std::uint64_t principal_state_seal,
        const std::uint64_t source_program_seal,
        const std::uint64_t valuation_hash,
        const std::uint64_t view_seal) noexcept
        : order_(order), represented_factor_count_(represented_factor_count),
          request_binding_(request_binding),
          principal_state_seal_(principal_state_seal),
          source_program_seal_(source_program_seal),
          valuation_hash_(valuation_hash), view_seal_(view_seal) {}

    friend class SelfCyclicActionBoundary;
};

[[nodiscard]] inline std::uint64_t self_cyclic_view_seal(
    const std::uint64_t order,
    const std::uint64_t represented_factor_count,
    const std::uint64_t request_binding,
    const std::uint64_t principal_state_seal,
    const std::uint64_t source_program_seal,
    const std::uint64_t valuation_hash) noexcept {
    return angel::afac56::mix64(
        order ^ std::rotl(represented_factor_count, 7) ^
        std::rotl(request_binding, 13) ^
        std::rotl(principal_state_seal, 23) ^
        std::rotl(source_program_seal, 37) ^
        std::rotl(valuation_hash, 51) ^ 0x53454c4643594331ULL);
}

class SelfCyclicActionBoundary final {
public:
    [[nodiscard]] CertifiedSelfCyclicActionView bind(
        const angel::afac58::CertifiedWilsonRequest& request) const {
        const auto request_report =
            angel::afac58::WilsonRequestVerifier::verify(request);
        if (!request_report.accepted)
            throw std::invalid_argument("R61 rejected invalid R60/R58 request");

        const auto& state = request.factorial_execution().principal_jet;
        const auto state_report = angel::afac57::PrincipalJetVerifier::verify(state);
        if (!state_report.accepted)
            throw std::invalid_argument("R61 rejected invalid principal-jet state");
        if (!state.source().is_zero())
            throw std::invalid_argument("R61 self-cyclic probe requires source zero");
        if (request.candidate() < 2U)
            throw std::invalid_argument("R61 cyclic order must be at least two");
        if (state.target().compare_u64(request.candidate() - 1U) != 0 ||
            state.valuation().compare_u64(request.candidate() - 1U) != 0)
            throw std::invalid_argument("R61 request/state diagonal mismatch");

        const auto factor_count = request.candidate() - 1U;
        const auto seal = self_cyclic_view_seal(
            request.candidate(), factor_count, request.binding_seal(),
            state.seal(), state.source_program().program().seal(),
            state.valuation().stable_hash());
        return CertifiedSelfCyclicActionView{
            request.candidate(), factor_count, request.binding_seal(),
            state.seal(), state.source_program().program().seal(),
            state.valuation().stable_hash(), seal};
    }
};

struct SelfCyclicViewVerification final {
    bool request_valid{};
    bool diagonal_binding_valid{};
    bool complete_state_identity_valid{};
    bool view_seal_valid{};
    bool no_state_compression{};
    bool no_feedback_channel{};
    bool accepted{};
};

class SelfCyclicViewVerifier final {
public:
    [[nodiscard]] static SelfCyclicViewVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedSelfCyclicActionView& view) noexcept {
        SelfCyclicViewVerification out{};
        const auto request_report =
            angel::afac58::WilsonRequestVerifier::verify(request);
        out.request_valid = request_report.accepted;
        if (!out.request_valid) return out;
        const auto& state = request.factorial_execution().principal_jet;
        out.diagonal_binding_valid =
            view.order() == request.candidate() &&
            view.represented_factor_count() == request.candidate() - 1U &&
            state.source().is_zero() &&
            state.target().compare_u64(request.candidate() - 1U) == 0 &&
            state.valuation().compare_u64(request.candidate() - 1U) == 0;
        out.complete_state_identity_valid =
            view.request_binding() == request.binding_seal() &&
            view.principal_state_seal() == state.seal() &&
            view.source_program_seal() ==
                state.source_program().program().seal() &&
            view.valuation_hash() == state.valuation().stable_hash();
        out.view_seal_valid = view.view_seal() == self_cyclic_view_seal(
            view.order(), view.represented_factor_count(),
            view.request_binding(), view.principal_state_seal(),
            view.source_program_seal(), view.valuation_hash());
        out.no_state_compression = !view.contains_angel_state() &&
                                   !view.compresses_angel_state();
        out.no_feedback_channel = !view.can_feed_back_to_angel();
        out.accepted = out.request_valid && out.diagonal_binding_valid &&
                       out.complete_state_identity_valid && out.view_seal_valid &&
                       out.no_state_compression && out.no_feedback_channel;
        return out;
    }
};

struct SelfCyclicProbePolicy final {
    std::uint64_t maximum_order{4096U};
    std::uint64_t audit_modulus{1'000'000'007U};
    bool materialize_normalized_state{};
    bool compute_modular_kernel_gcd{};
};

[[nodiscard]] inline bool supported_kernel_audit_prime(
    const std::uint64_t modulus) noexcept {
    return modulus == 998'244'353U || modulus == 1'000'000'007U ||
           modulus == 1'000'000'009U;
}

struct SelfCyclicStateIntegrityWitness final {
    std::uint64_t request_binding_before{};
    std::uint64_t request_binding_after{};
    std::uint64_t state_seal_before{};
    std::uint64_t state_seal_after{};
    std::uint64_t program_seal_before{};
    std::uint64_t program_seal_after{};
    std::uint64_t payload_bytes_before{};
    std::uint64_t payload_bytes_after{};
    std::uint64_t angel_nodes_rewritten{};
    std::uint64_t angel_nodes_merged{};
    bool angel_state_compressed{};
    bool ordinary_result_fed_back_to_angel{};

    [[nodiscard]] bool accepted() const noexcept {
        return request_binding_before == request_binding_after &&
               state_seal_before == state_seal_after &&
               program_seal_before == program_seal_after &&
               payload_bytes_before == payload_bytes_after &&
               angel_nodes_rewritten == 0U && angel_nodes_merged == 0U &&
               !angel_state_compressed && !ordinary_result_fed_back_to_angel;
    }
};

struct SelfCyclicReferenceResponse final {
    std::uint64_t order{};
    std::uint64_t audit_modulus{};
    std::vector<std::uint64_t> unnormalized_cyclic_column;
    std::vector<std::uint64_t> normalized_cyclic_column;
    std::uint64_t normalized_kernel_degree_mod_audit_field{};
    bool normalized_kernel_degree_computed{};
    bool prime_projector_shape{};
    std::uint64_t response_seal{};
    SelfCyclicProbeLedger ledger{};
    SelfCyclicStateIntegrityWitness integrity{};
};

struct SelfCyclicResourceLimit final {
    std::uint64_t requested_order{};
    std::uint64_t maximum_order{};
    std::uint64_t certificate_seal{};
    SelfCyclicStateIntegrityWitness integrity{};
};

using SelfCyclicProbeDownload =
    std::variant<SelfCyclicReferenceResponse, SelfCyclicResourceLimit>;

[[nodiscard]] inline std::uint64_t add_mod(
    const std::uint64_t a, const std::uint64_t b,
    const std::uint64_t modulus) noexcept {
    return a >= modulus - b ? a - (modulus - b) : a + b;
}

[[nodiscard]] inline std::uint64_t sub_mod(
    const std::uint64_t a, const std::uint64_t b,
    const std::uint64_t modulus) noexcept {
    return a >= b ? a - b : modulus - (b - a);
}

[[nodiscard]] inline std::uint64_t mul_mod(
    const std::uint64_t a, const std::uint64_t b,
    const std::uint64_t modulus) noexcept {
    __extension__ using wide = unsigned __int128;
    return static_cast<std::uint64_t>((static_cast<wide>(a) * b) % modulus);
}

[[nodiscard]] inline std::uint64_t pow_mod(
    std::uint64_t base, std::uint64_t exponent,
    const std::uint64_t modulus) noexcept {
    std::uint64_t result = 1U % modulus;
    while (exponent != 0U) {
        if ((exponent & 1U) != 0U) result = mul_mod(result, base, modulus);
        base = mul_mod(base, base, modulus);
        exponent >>= 1U;
    }
    return result;
}

[[nodiscard]] inline std::vector<std::uint64_t>
materialize_unnormalized_self_cyclic_column(
    const std::uint64_t order,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    if (order < 2U || modulus < 3U)
        throw std::invalid_argument("invalid self-cyclic reference domain");
    std::vector<std::uint64_t> current(order, 0U);
    std::vector<std::uint64_t> next(order, 0U);
    current[0] = 1U;
    if (ledger) ledger->maximum_live_coefficients = 2U * order;
    for (std::uint64_t k = 1U; k < order; ++k) {
        next = current;
        for (std::uint64_t index = 0U; index < order; ++index) {
            const auto shifted = (index + k) % order;
            next[shifted] = sub_mod(next[shifted], current[index], modulus);
            if (ledger) ++ledger->cyclic_coefficient_updates;
        }
        current.swap(next);
        if (ledger) ++ledger->qpoch_factors_applied;
    }
    return current;
}

// Materializes (-1)^n [n]_S! for n<order, the normalized R57
// q-Pochhammer interval state evaluated at T=S-I.  Each convolution with
// [k]_S=1+S+...+S^(k-1) is evaluated by a cyclic sliding window in O(order).
[[nodiscard]] inline std::vector<std::uint64_t>
materialize_signed_normalized_interval_cyclic_column(
    const std::uint64_t factor_count,
    const std::uint64_t order,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    if (order < 2U || modulus < 3U || factor_count >= order)
        throw std::invalid_argument("invalid normalized interval/cyclic domain");
    std::vector<std::uint64_t> current(order, 0U);
    std::vector<std::uint64_t> next(order, 0U);
    current[0] = 1U;
    if (ledger) ledger->maximum_live_coefficients =
        std::max<std::uint64_t>(ledger->maximum_live_coefficients, 2U * order);

    for (std::uint64_t k = 1U; k <= factor_count; ++k) {
        std::uint64_t window = 0U;
        for (std::uint64_t j = 0U; j < k; ++j) {
            const auto index = (order - (j % order)) % order;
            window = add_mod(window, current[index], modulus);
            if (ledger) ++ledger->normalized_window_updates;
        }
        next[0] = window;
        for (std::uint64_t i = 0U; i + 1U < order; ++i) {
            const auto incoming = current[i + 1U];
            const auto outgoing = current[(i + 1U + order - k) % order];
            window = add_mod(window, incoming, modulus);
            window = sub_mod(window, outgoing, modulus);
            next[i + 1U] = window;
            if (ledger) ledger->normalized_window_updates += 2U;
        }
        current.swap(next);
        if (ledger) ++ledger->qpoch_factors_applied;
    }
    if ((factor_count & 1U) != 0U) {
        for (auto& coefficient : current)
            if (coefficient != 0U) coefficient = modulus - coefficient;
    }
    return current;
}

[[nodiscard]] inline std::vector<std::uint64_t>
materialize_signed_normalized_self_cyclic_column(
    const std::uint64_t order,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    return materialize_signed_normalized_interval_cyclic_column(
        order - 1U, order, modulus, ledger);
}

[[nodiscard]] inline std::vector<std::uint64_t> apply_s_minus_identity_power(
    std::vector<std::uint64_t> value,
    const std::uint64_t exponent,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    const auto order = static_cast<std::uint64_t>(value.size());
    std::vector<std::uint64_t> next(order, 0U);
    for (std::uint64_t round = 0U; round < exponent; ++round) {
        for (std::uint64_t i = 0U; i < order; ++i) {
            next[(i + 1U) % order] = add_mod(
                next[(i + 1U) % order], value[i], modulus);
            next[i] = sub_mod(next[i], value[i], modulus);
            if (ledger) ledger->cyclic_coefficient_updates += 2U;
        }
        value.swap(next);
        std::fill(next.begin(), next.end(), 0U);
    }
    return value;
}

inline void trim_polynomial(std::vector<std::uint64_t>& polynomial) {
    while (!polynomial.empty() && polynomial.back() == 0U)
        polynomial.pop_back();
}

[[nodiscard]] inline std::vector<std::uint64_t> polynomial_remainder_prime_field(
    std::vector<std::uint64_t> dividend,
    const std::vector<std::uint64_t>& divisor,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    auto normalized_divisor = divisor;
    trim_polynomial(dividend);
    trim_polynomial(normalized_divisor);
    if (normalized_divisor.empty())
        throw std::invalid_argument("polynomial division by zero");
    const auto divisor_degree = normalized_divisor.size() - 1U;
    const auto inverse_lead = pow_mod(
        normalized_divisor.back(), modulus - 2U, modulus);
    while (!dividend.empty() && dividend.size() >= normalized_divisor.size()) {
        const auto shift = dividend.size() - normalized_divisor.size();
        const auto factor = mul_mod(dividend.back(), inverse_lead, modulus);
        for (std::size_t j = 0U; j <= divisor_degree; ++j) {
            const auto position = shift + j;
            dividend[position] = sub_mod(
                dividend[position],
                mul_mod(factor, normalized_divisor[j], modulus), modulus);
            if (ledger) ++ledger->polynomial_gcd_coefficient_steps;
        }
        trim_polynomial(dividend);
    }
    return dividend;
}

[[nodiscard]] inline std::uint64_t normalized_kernel_degree_prime_field(
    std::vector<std::uint64_t> normalized_column,
    const std::uint64_t order,
    const std::uint64_t modulus,
    SelfCyclicProbeLedger* ledger = nullptr) {
    trim_polynomial(normalized_column);
    std::vector<std::uint64_t> cyclic(order + 1U, 0U);
    cyclic[0] = modulus - 1U;
    cyclic[order] = 1U;
    auto left = std::move(cyclic);
    auto right = std::move(normalized_column);
    while (!right.empty()) {
        auto remainder = polynomial_remainder_prime_field(
            std::move(left), right, modulus, ledger);
        left = std::move(right);
        right = std::move(remainder);
    }
    trim_polynomial(left);
    return left.empty() ? 0U : static_cast<std::uint64_t>(left.size() - 1U);
}

[[nodiscard]] inline bool has_prime_projector_shape(
    const std::vector<std::uint64_t>& column,
    const std::uint64_t order,
    const std::uint64_t modulus) noexcept {
    if (column.size() != order) return false;
    if (column[0] != (order - 1U) % modulus) return false;
    const auto minus_one = modulus - 1U;
    return std::all_of(column.begin() + 1, column.end(),
                       [&](const auto value) { return value == minus_one; });
}

[[nodiscard]] inline std::uint64_t response_seal(
    const CertifiedSelfCyclicActionView& view,
    const std::vector<std::uint64_t>& unnormalized,
    const std::vector<std::uint64_t>& normalized,
    const std::uint64_t audit_modulus,
    const std::uint64_t kernel_degree,
    const bool kernel_computed,
    const bool prime_projector_shape) noexcept {
    std::uint64_t seal = angel::afac56::mix64(
        view.view_seal() ^ std::rotl(audit_modulus, 11) ^
        std::rotl(kernel_degree, 29) ^
        std::rotl(static_cast<std::uint64_t>(kernel_computed), 41) ^
        std::rotl(static_cast<std::uint64_t>(prime_projector_shape), 53) ^
        0x50524f4a45435431ULL);
    for (std::size_t i = 0U; i < unnormalized.size(); ++i)
        seal = angel::afac56::mix64(
            seal ^ std::rotl(unnormalized[i], i & 63U) ^
            (0x9e3779b97f4a7c15ULL * (i + 1U)));
    for (std::size_t i = 0U; i < normalized.size(); ++i)
        seal = angel::afac56::mix64(
            seal ^ std::rotl(normalized[i], (i + 17U) & 63U) ^
            (0xd1b54a32d192ed03ULL * (i + 1U)));
    return seal;
}

class SelfCyclicActionObserver final {
public:
    [[nodiscard]] SelfCyclicProbeDownload observe(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedSelfCyclicActionView& view,
        const SelfCyclicProbePolicy policy = {}) const {
        if (!SelfCyclicViewVerifier::verify(request, view).accepted)
            throw std::invalid_argument("R61 self-cyclic view binding rejected");
        if (policy.audit_modulus < 3U || view.order() >= policy.audit_modulus)
            throw std::invalid_argument(
                "R61 audit modulus must exceed the cyclic order");
        if (policy.compute_modular_kernel_gcd &&
            !supported_kernel_audit_prime(policy.audit_modulus))
            throw std::invalid_argument(
                "R61 kernel GCD requires a frozen supported audit prime");

        const auto before = snapshot(request);
        if (view.order() > policy.maximum_order) {
            SelfCyclicResourceLimit limit{};
            limit.requested_order = view.order();
            limit.maximum_order = policy.maximum_order;
            limit.integrity = snapshot(request);
            copy_before(before, limit.integrity);
            limit.certificate_seal = angel::afac56::mix64(
                view.view_seal() ^ std::rotl(limit.requested_order, 17) ^
                std::rotl(limit.maximum_order, 43) ^ 0x5245534c494d3631ULL);
            return limit;
        }

        SelfCyclicReferenceResponse response{};
        response.order = view.order();
        response.audit_modulus = policy.audit_modulus;
        response.unnormalized_cyclic_column =
            materialize_unnormalized_self_cyclic_column(
                view.order(), policy.audit_modulus, &response.ledger);
        response.prime_projector_shape = has_prime_projector_shape(
            response.unnormalized_cyclic_column, view.order(),
            policy.audit_modulus);
        if (policy.materialize_normalized_state ||
            policy.compute_modular_kernel_gcd) {
            response.normalized_cyclic_column =
                materialize_signed_normalized_self_cyclic_column(
                    view.order(), policy.audit_modulus, &response.ledger);
        }
        if (policy.compute_modular_kernel_gcd) {
            response.normalized_kernel_degree_mod_audit_field =
                normalized_kernel_degree_prime_field(
                    response.normalized_cyclic_column, view.order(),
                    policy.audit_modulus, &response.ledger);
            response.normalized_kernel_degree_computed = true;
        }
        response.integrity = snapshot(request);
        copy_before(before, response.integrity);
        response.ledger.angel_nodes_rewritten = 0U;
        response.ledger.angel_nodes_merged = 0U;
        response.ledger.ordinary_result_fed_back_to_angel = false;
        response.response_seal = response_seal(
            view, response.unnormalized_cyclic_column,
            response.normalized_cyclic_column, response.audit_modulus,
            response.normalized_kernel_degree_mod_audit_field,
            response.normalized_kernel_degree_computed,
            response.prime_projector_shape);
        return response;
    }

private:
    [[nodiscard]] static SelfCyclicStateIntegrityWitness snapshot(
        const angel::afac58::CertifiedWilsonRequest& request) noexcept {
        const auto& state = request.factorial_execution().principal_jet;
        SelfCyclicStateIntegrityWitness witness{};
        witness.request_binding_before = request.binding_seal();
        witness.request_binding_after = request.binding_seal();
        witness.state_seal_before = state.seal();
        witness.state_seal_after = state.seal();
        witness.program_seal_before = state.source_program().program().seal();
        witness.program_seal_after = state.source_program().program().seal();
        witness.payload_bytes_before = state.payload_bytes();
        witness.payload_bytes_after = state.payload_bytes();
        witness.angel_nodes_rewritten = 0U;
        witness.angel_nodes_merged = 0U;
        witness.angel_state_compressed = false;
        witness.ordinary_result_fed_back_to_angel = false;
        return witness;
    }

    static void copy_before(
        const SelfCyclicStateIntegrityWitness& before,
        SelfCyclicStateIntegrityWitness& after) noexcept {
        after.request_binding_before = before.request_binding_before;
        after.state_seal_before = before.state_seal_before;
        after.program_seal_before = before.program_seal_before;
        after.payload_bytes_before = before.payload_bytes_before;
    }
};

struct SelfCyclicResponseVerification final {
    bool view_valid{};
    bool integrity_valid{};
    bool dimensions_valid{};
    bool response_seal_valid{};
    bool no_feedback{};
    bool accepted{};
};

class SelfCyclicResponseVerifier final {
public:
    [[nodiscard]] static SelfCyclicResponseVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedSelfCyclicActionView& view,
        const SelfCyclicProbeDownload& download) noexcept {
        SelfCyclicResponseVerification out{};
        out.view_valid = SelfCyclicViewVerifier::verify(request, view).accepted;
        if (const auto* response =
                std::get_if<SelfCyclicReferenceResponse>(&download)) {
            out.integrity_valid = response->integrity.accepted();
            out.dimensions_valid =
                response->order == view.order() &&
                response->unnormalized_cyclic_column.size() == view.order() &&
                (response->normalized_cyclic_column.empty() ||
                 response->normalized_cyclic_column.size() == view.order());
            out.response_seal_valid = response->response_seal == response_seal(
                view, response->unnormalized_cyclic_column,
                response->normalized_cyclic_column, response->audit_modulus,
                response->normalized_kernel_degree_mod_audit_field,
                response->normalized_kernel_degree_computed,
                response->prime_projector_shape);
            out.no_feedback =
                !response->ledger.ordinary_result_fed_back_to_angel &&
                response->ledger.angel_nodes_rewritten == 0U &&
                response->ledger.angel_nodes_merged == 0U;
        } else {
            const auto& limit = std::get<SelfCyclicResourceLimit>(download);
            out.integrity_valid = limit.integrity.accepted();
            out.dimensions_valid = limit.requested_order == view.order();
            out.response_seal_valid = limit.certificate_seal ==
                angel::afac56::mix64(
                    view.view_seal() ^
                    std::rotl(limit.requested_order, 17) ^
                    std::rotl(limit.maximum_order, 43) ^
                    0x5245534c494d3631ULL);
            out.no_feedback = !limit.integrity.ordinary_result_fed_back_to_angel;
        }
        out.accepted = out.view_valid && out.integrity_valid &&
                       out.dimensions_valid && out.response_seal_valid &&
                       out.no_feedback;
        return out;
    }
};

// ------------------------- freeze-after-run audit -------------------------
// None of the following functions is called by SelfCyclicActionObserver.

[[nodiscard]] inline bool ordinary_prime_audit(const std::uint64_t n) noexcept {
    if (n < 2U) return false;
    if ((n & 1U) == 0U) return n == 2U;
    for (std::uint64_t d = 3U; d <= n / d; d += 2U)
        if (n % d == 0U) return false;
    return true;
}

[[nodiscard]] inline std::uint64_t euler_phi_audit(std::uint64_t n) noexcept {
    std::uint64_t result = n;
    for (std::uint64_t p = 2U; p <= n / p; ++p) {
        if (n % p != 0U) continue;
        while (n % p == 0U) n /= p;
        result -= result / p;
    }
    if (n > 1U) result -= result / n;
    return result;
}

[[nodiscard]] inline int mobius_audit(std::uint64_t n) noexcept {
    int sign = 1;
    for (std::uint64_t p = 2U; p <= n / p; ++p) {
        if (n % p != 0U) continue;
        n /= p;
        sign = -sign;
        if (n % p == 0U) return 0;
        while (n % p == 0U) n /= p;
    }
    if (n > 1U) sign = -sign;
    return sign;
}

[[nodiscard]] inline std::int64_t ramanujan_sum_audit(
    const std::uint64_t n, const std::uint64_t k,
    SelfCyclicProbeLedger* ledger = nullptr) noexcept {
    const auto g = std::gcd(n, k);
    const auto quotient = n / g;
    const auto mu = mobius_audit(quotient);
    if (ledger) ++ledger->audit_number_theory_steps;
    if (mu == 0) return 0;
    const auto phi_n = euler_phi_audit(n);
    const auto phi_q = euler_phi_audit(quotient);
    if (ledger) ledger->audit_number_theory_steps += 2U;
    const auto magnitude = phi_n / phi_q;
    return mu > 0 ? static_cast<std::int64_t>(magnitude)
                  : -static_cast<std::int64_t>(magnitude);
}

[[nodiscard]] inline bool ramanujan_response_matches_audit(
    const SelfCyclicReferenceResponse& response,
    SelfCyclicProbeLedger* ledger = nullptr) noexcept {
    for (std::uint64_t k = 0U; k < response.order; ++k) {
        const auto expected_signed = ramanujan_sum_audit(
            response.order, k, ledger);
        const auto expected = expected_signed >= 0
            ? static_cast<std::uint64_t>(expected_signed) % response.audit_modulus
            : response.audit_modulus -
                (static_cast<std::uint64_t>(-expected_signed) %
                 response.audit_modulus);
        if (response.unnormalized_cyclic_column[k] != expected)
            return false;
    }
    return true;
}

[[nodiscard]] inline std::uint64_t predicted_normalized_kernel_dimension_audit(
    const std::uint64_t order) noexcept {
    return order - 1U - euler_phi_audit(order);
}

[[nodiscard]] inline std::uint64_t predicted_unnormalized_kernel_dimension_audit(
    const std::uint64_t order) noexcept {
    return order - euler_phi_audit(order);
}

// Named forbidden bridges.
angel::afac57::CertifiedPrincipalJetState compress_angel_state_from_self_cyclic_view(
    const CertifiedSelfCyclicActionView&) = delete;
angel::afac59::AngelRunningState feed_self_cyclic_response_back_to_angel(
    const SelfCyclicProbeDownload&) = delete;
CertifiedSelfCyclicActionView forge_self_cyclic_view_without_request(
    const std::uint64_t) = delete;

static_assert(!std::is_default_constructible_v<CertifiedSelfCyclicActionView>);
static_assert(!std::is_convertible_v<CertifiedSelfCyclicActionView,
                                     angel::afac57::CertifiedPrincipalJetState>);
static_assert(!std::is_convertible_v<SelfCyclicProbeDownload,
                                     angel::afac59::AngelRunningState>);

} // namespace angel::afac61
