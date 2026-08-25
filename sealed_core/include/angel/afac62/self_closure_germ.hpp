#pragma once

#include "angel/afac61/self_cyclic_probe.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac62 {

// R62 asks the R61 self-cyclic object one more native structural question:
// what is retained if the first forced closure factor k=m is included, but the
// resulting zero is kept as an exact formal germ instead of collapsed?
//
// The reference observer evaluates
//   F_{m,N}(eps) = product_{k=1}^{Nm}(I-(1+eps)^k S_m^k)
// in F_p[eps]/(eps^(R+1))[C_m].  It never calls primality, gcd,
// factorization, phi, Mobius, Ramanujan, or a first-support selector.

struct SelfClosureGermLedger final {
    std::uint64_t qpoch_factors_applied{};
    std::uint64_t binomial_coefficients_built{};
    std::uint64_t jet_cyclic_coefficient_updates{};
    std::uint64_t quotient_coefficient_folds{};
    std::uint64_t audit_mode_evaluations{};
    std::uint64_t audit_number_theory_steps{};
    std::uint64_t maximum_live_coefficients{};
    std::uint64_t angel_nodes_rewritten{};
    std::uint64_t angel_nodes_merged{};
    bool ordinary_result_fed_back_to_angel{};

    friend bool operator==(const SelfClosureGermLedger&,
                           const SelfClosureGermLedger&) = default;
};

class CertifiedSelfClosureGermView final {
public:
    CertifiedSelfClosureGermView() = delete;
    CertifiedSelfClosureGermView(const CertifiedSelfClosureGermView&) = default;
    CertifiedSelfClosureGermView(CertifiedSelfClosureGermView&&) noexcept = default;
    CertifiedSelfClosureGermView& operator=(const CertifiedSelfClosureGermView&) = default;
    CertifiedSelfClosureGermView& operator=(CertifiedSelfClosureGermView&&) noexcept = default;

    [[nodiscard]] std::uint64_t order() const noexcept { return order_; }
    [[nodiscard]] std::uint64_t cycles() const noexcept { return cycles_; }
    [[nodiscard]] std::uint64_t jet_order() const noexcept { return jet_order_; }
    [[nodiscard]] std::uint64_t parent_view_seal() const noexcept {
        return parent_view_seal_;
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
    [[nodiscard]] std::uint64_t view_seal() const noexcept { return view_seal_; }

    [[nodiscard]] bool contains_angel_state() const noexcept { return false; }
    [[nodiscard]] bool compresses_angel_state() const noexcept { return false; }
    [[nodiscard]] bool can_feed_back_to_angel() const noexcept { return false; }
    [[nodiscard]] std::size_t descriptor_words() const noexcept { return 8U; }

private:
    std::uint64_t order_{};
    std::uint64_t cycles_{};
    std::uint64_t jet_order_{};
    std::uint64_t parent_view_seal_{};
    std::uint64_t request_binding_{};
    std::uint64_t principal_state_seal_{};
    std::uint64_t source_program_seal_{};
    std::uint64_t view_seal_{};

    CertifiedSelfClosureGermView(
        const std::uint64_t order,
        const std::uint64_t cycles,
        const std::uint64_t jet_order,
        const std::uint64_t parent_view_seal,
        const std::uint64_t request_binding,
        const std::uint64_t principal_state_seal,
        const std::uint64_t source_program_seal,
        const std::uint64_t view_seal) noexcept
        : order_(order), cycles_(cycles), jet_order_(jet_order),
          parent_view_seal_(parent_view_seal),
          request_binding_(request_binding),
          principal_state_seal_(principal_state_seal),
          source_program_seal_(source_program_seal), view_seal_(view_seal) {}

    friend class SelfClosureGermBoundary;
};

[[nodiscard]] inline std::uint64_t self_closure_germ_view_seal(
    const std::uint64_t order,
    const std::uint64_t cycles,
    const std::uint64_t jet_order,
    const std::uint64_t parent_view_seal,
    const std::uint64_t request_binding,
    const std::uint64_t principal_state_seal,
    const std::uint64_t source_program_seal) noexcept {
    return angel::afac56::mix64(
        order ^ std::rotl(cycles, 7) ^ std::rotl(jet_order, 15) ^
        std::rotl(parent_view_seal, 23) ^ std::rotl(request_binding, 31) ^
        std::rotl(principal_state_seal, 43) ^
        std::rotl(source_program_seal, 53) ^ 0x53434c4f53453632ULL);
}

class SelfClosureGermBoundary final {
public:
    [[nodiscard]] CertifiedSelfClosureGermView bind(
        const angel::afac58::CertifiedWilsonRequest& request,
        const angel::afac61::CertifiedSelfCyclicActionView& parent_view,
        const std::uint64_t cycles,
        const std::uint64_t jet_order) const {
        if (!angel::afac61::SelfCyclicViewVerifier::verify(request, parent_view).accepted)
            throw std::invalid_argument("R62 rejected invalid R61 parent view");
        if (cycles == 0U)
            throw std::invalid_argument("R62 requires at least one self-cycle");
        if (jet_order == 0U)
            throw std::invalid_argument("R62 requires a positive jet order");
        const auto& state = request.factorial_execution().principal_jet;
        const auto seal = self_closure_germ_view_seal(
            parent_view.order(), cycles, jet_order, parent_view.view_seal(),
            request.binding_seal(), state.seal(),
            state.source_program().program().seal());
        return CertifiedSelfClosureGermView{
            parent_view.order(), cycles, jet_order, parent_view.view_seal(),
            request.binding_seal(), state.seal(),
            state.source_program().program().seal(), seal};
    }
};

struct SelfClosureGermViewVerification final {
    bool parent_valid{};
    bool dimensions_valid{};
    bool complete_state_identity_valid{};
    bool view_seal_valid{};
    bool no_state_compression{};
    bool no_feedback_channel{};
    bool accepted{};
};

class SelfClosureGermViewVerifier final {
public:
    [[nodiscard]] static SelfClosureGermViewVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const angel::afac61::CertifiedSelfCyclicActionView& parent_view,
        const CertifiedSelfClosureGermView& view) noexcept {
        SelfClosureGermViewVerification out{};
        out.parent_valid =
            angel::afac61::SelfCyclicViewVerifier::verify(request, parent_view).accepted;
        if (!out.parent_valid) return out;
        const auto& state = request.factorial_execution().principal_jet;
        out.dimensions_valid = view.order() == parent_view.order() &&
                               view.cycles() > 0U && view.jet_order() > 0U;
        out.complete_state_identity_valid =
            view.parent_view_seal() == parent_view.view_seal() &&
            view.request_binding() == request.binding_seal() &&
            view.principal_state_seal() == state.seal() &&
            view.source_program_seal() == state.source_program().program().seal();
        out.view_seal_valid = view.view_seal() == self_closure_germ_view_seal(
            view.order(), view.cycles(), view.jet_order(),
            view.parent_view_seal(), view.request_binding(),
            view.principal_state_seal(), view.source_program_seal());
        out.no_state_compression = !view.contains_angel_state() &&
                                   !view.compresses_angel_state();
        out.no_feedback_channel = !view.can_feed_back_to_angel();
        out.accepted = out.parent_valid && out.dimensions_valid &&
                       out.complete_state_identity_valid && out.view_seal_valid &&
                       out.no_state_compression && out.no_feedback_channel;
        return out;
    }
};

struct SelfClosureGermPolicy final {
    std::uint64_t maximum_order{256U};
    std::uint64_t maximum_cycles{4U};
    std::uint64_t maximum_jet_order{256U};
    std::uint64_t audit_modulus{998'244'353U};
};

struct SelfClosureGermIntegrityWitness final {
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

using CyclicJetTensor = std::vector<std::vector<std::uint64_t>>;

struct SelfClosureGermReferenceResponse final {
    std::uint64_t order{};
    std::uint64_t cycles{};
    std::uint64_t jet_order{};
    std::uint64_t audit_modulus{};
    CyclicJetTensor jet_coefficients;
    std::uint64_t response_seal{};
    SelfClosureGermLedger ledger{};
    SelfClosureGermIntegrityWitness integrity{};
};

struct SelfClosureGermResourceLimit final {
    std::uint64_t requested_order{};
    std::uint64_t requested_cycles{};
    std::uint64_t requested_jet_order{};
    std::uint64_t maximum_order{};
    std::uint64_t maximum_cycles{};
    std::uint64_t maximum_jet_order{};
    std::uint64_t certificate_seal{};
    SelfClosureGermIntegrityWitness integrity{};
};

using SelfClosureGermDownload =
    std::variant<SelfClosureGermReferenceResponse, SelfClosureGermResourceLimit>;

[[nodiscard]] inline std::vector<std::uint64_t> modular_inverses(
    const std::uint64_t count, const std::uint64_t modulus) {
    if (count >= modulus)
        throw std::invalid_argument("jet order must be below audit modulus");
    std::vector<std::uint64_t> inv(count + 1U, 0U);
    if (count != 0U) inv[1U] = 1U;
    for (std::uint64_t i = 2U; i <= count; ++i) {
        const auto q = modulus / i;
        const auto r = modulus % i;
        inv[i] = angel::afac61::sub_mod(
            0U, angel::afac61::mul_mod(q % modulus, inv[r], modulus), modulus);
    }
    return inv;
}

[[nodiscard]] inline std::vector<std::uint64_t> truncated_binomial_row(
    const std::uint64_t exponent,
    const std::uint64_t jet_order,
    const std::uint64_t modulus,
    const std::vector<std::uint64_t>& inverses,
    SelfClosureGermLedger* ledger = nullptr) {
    const auto limit = std::min(exponent, jet_order);
    std::vector<std::uint64_t> row(limit + 1U, 1U);
    for (std::uint64_t j = 1U; j <= limit; ++j) {
        row[j] = angel::afac61::mul_mod(
            row[j - 1U], (exponent - j + 1U) % modulus, modulus);
        row[j] = angel::afac61::mul_mod(row[j], inverses[j], modulus);
        if (ledger) ++ledger->binomial_coefficients_built;
    }
    return row;
}

[[nodiscard]] inline CyclicJetTensor materialize_self_closure_germ(
    const std::uint64_t order,
    const std::uint64_t cycles,
    const std::uint64_t jet_order,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) {
    if (order < 2U || cycles == 0U || jet_order == 0U || modulus < 3U)
        throw std::invalid_argument("invalid self-closure germ domain");
    if (jet_order >= modulus)
        throw std::invalid_argument("jet order must be below audit modulus");
    if (cycles > std::numeric_limits<std::uint64_t>::max() / order)
        throw std::overflow_error("self-closure factor count overflow");

    const auto factor_count = cycles * order;
    auto inverses = modular_inverses(jet_order, modulus);
    CyclicJetTensor current(jet_order + 1U,
                            std::vector<std::uint64_t>(order, 0U));
    CyclicJetTensor next = current;
    current[0U][0U] = 1U;
    if (ledger)
        ledger->maximum_live_coefficients =
            2U * (jet_order + 1U) * order;

    for (std::uint64_t k = 1U; k <= factor_count; ++k) {
        const auto shift = k % order;
        const auto binomial = truncated_binomial_row(
            k, jet_order, modulus, inverses, ledger);
        next = current; // contribution of the identity term in the factor
        for (std::uint64_t degree = 0U; degree <= jet_order; ++degree) {
            const auto maximum_j = std::min<std::uint64_t>(
                degree, static_cast<std::uint64_t>(binomial.size() - 1U));
            for (std::uint64_t j = 0U; j <= maximum_j; ++j) {
                const auto weight = binomial[j];
                const auto& source = current[degree - j];
                auto& target = next[degree];
                for (std::uint64_t index = 0U; index < order; ++index) {
                    const auto destination = (index + shift) % order;
                    target[destination] = angel::afac61::sub_mod(
                        target[destination],
                        angel::afac61::mul_mod(source[index], weight, modulus),
                        modulus);
                    if (ledger) ++ledger->jet_cyclic_coefficient_updates;
                }
            }
        }
        current.swap(next);
        if (ledger) ++ledger->qpoch_factors_applied;
    }
    return current;
}

[[nodiscard]] inline std::vector<std::uint64_t> quotient_pushforward_column(
    const std::vector<std::uint64_t>& source,
    const std::uint64_t target_order,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) {
    if (target_order < 2U || source.empty() || source.size() % target_order != 0U)
        throw std::invalid_argument("invalid cyclic quotient pushforward");
    std::vector<std::uint64_t> target(target_order, 0U);
    for (std::size_t i = 0U; i < source.size(); ++i) {
        target[i % target_order] = angel::afac61::add_mod(
            target[i % target_order], source[i], modulus);
        if (ledger) ++ledger->quotient_coefficient_folds;
    }
    return target;
}

[[nodiscard]] inline CyclicJetTensor quotient_pushforward_germ(
    const CyclicJetTensor& source,
    const std::uint64_t target_order,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) {
    CyclicJetTensor target;
    target.reserve(source.size());
    for (const auto& coefficient : source)
        target.push_back(quotient_pushforward_column(
            coefficient, target_order, modulus, ledger));
    return target;
}

[[nodiscard]] inline std::uint64_t germ_response_seal(
    const CertifiedSelfClosureGermView& view,
    const CyclicJetTensor& tensor,
    const std::uint64_t modulus) noexcept {
    std::uint64_t seal = angel::afac56::mix64(
        view.view_seal() ^ std::rotl(modulus, 13) ^ 0x4745524d52363231ULL);
    for (std::size_t degree = 0U; degree < tensor.size(); ++degree) {
        for (std::size_t index = 0U; index < tensor[degree].size(); ++index) {
            seal = angel::afac56::mix64(
                seal ^ std::rotl(tensor[degree][index],
                                 (degree + index) & 63U) ^
                (0x9e3779b97f4a7c15ULL * (1U + degree * 131U + index)));
        }
    }
    return seal;
}

class SelfClosureGermObserver final {
public:
    [[nodiscard]] SelfClosureGermDownload observe(
        const angel::afac58::CertifiedWilsonRequest& request,
        const angel::afac61::CertifiedSelfCyclicActionView& parent_view,
        const CertifiedSelfClosureGermView& view,
        const SelfClosureGermPolicy policy = {}) const {
        if (!SelfClosureGermViewVerifier::verify(request, parent_view, view).accepted)
            throw std::invalid_argument("R62 self-closure germ view rejected");
        if (policy.audit_modulus < 3U || view.order() >= policy.audit_modulus ||
            view.jet_order() >= policy.audit_modulus)
            throw std::invalid_argument("R62 audit modulus too small");

        const auto before = snapshot(request);
        if (view.order() > policy.maximum_order ||
            view.cycles() > policy.maximum_cycles ||
            view.jet_order() > policy.maximum_jet_order) {
            SelfClosureGermResourceLimit limit{};
            limit.requested_order = view.order();
            limit.requested_cycles = view.cycles();
            limit.requested_jet_order = view.jet_order();
            limit.maximum_order = policy.maximum_order;
            limit.maximum_cycles = policy.maximum_cycles;
            limit.maximum_jet_order = policy.maximum_jet_order;
            limit.integrity = snapshot(request);
            copy_before(before, limit.integrity);
            limit.certificate_seal = angel::afac56::mix64(
                view.view_seal() ^ std::rotl(limit.requested_order, 7) ^
                std::rotl(limit.requested_cycles, 19) ^
                std::rotl(limit.requested_jet_order, 31) ^
                std::rotl(limit.maximum_order, 43) ^
                std::rotl(limit.maximum_cycles, 51) ^
                std::rotl(limit.maximum_jet_order, 59) ^
                0x5245534c494d3632ULL);
            return limit;
        }

        SelfClosureGermReferenceResponse response{};
        response.order = view.order();
        response.cycles = view.cycles();
        response.jet_order = view.jet_order();
        response.audit_modulus = policy.audit_modulus;
        response.jet_coefficients = materialize_self_closure_germ(
            view.order(), view.cycles(), view.jet_order(),
            policy.audit_modulus, &response.ledger);
        response.integrity = snapshot(request);
        copy_before(before, response.integrity);
        response.ledger.angel_nodes_rewritten = 0U;
        response.ledger.angel_nodes_merged = 0U;
        response.ledger.ordinary_result_fed_back_to_angel = false;
        response.response_seal = germ_response_seal(
            view, response.jet_coefficients, response.audit_modulus);
        return response;
    }

private:
    [[nodiscard]] static SelfClosureGermIntegrityWitness snapshot(
        const angel::afac58::CertifiedWilsonRequest& request) noexcept {
        const auto& state = request.factorial_execution().principal_jet;
        SelfClosureGermIntegrityWitness witness{};
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
        const SelfClosureGermIntegrityWitness& before,
        SelfClosureGermIntegrityWitness& after) noexcept {
        after.request_binding_before = before.request_binding_before;
        after.state_seal_before = before.state_seal_before;
        after.program_seal_before = before.program_seal_before;
        after.payload_bytes_before = before.payload_bytes_before;
    }
};

struct SelfClosureGermResponseVerification final {
    bool view_valid{};
    bool integrity_valid{};
    bool dimensions_valid{};
    bool response_seal_valid{};
    bool no_feedback{};
    bool accepted{};
};

class SelfClosureGermResponseVerifier final {
public:
    [[nodiscard]] static SelfClosureGermResponseVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const angel::afac61::CertifiedSelfCyclicActionView& parent_view,
        const CertifiedSelfClosureGermView& view,
        const SelfClosureGermDownload& download) noexcept {
        SelfClosureGermResponseVerification out{};
        out.view_valid =
            SelfClosureGermViewVerifier::verify(request, parent_view, view).accepted;
        if (const auto* response =
                std::get_if<SelfClosureGermReferenceResponse>(&download)) {
            out.integrity_valid = response->integrity.accepted();
            out.dimensions_valid = response->order == view.order() &&
                response->cycles == view.cycles() &&
                response->jet_order == view.jet_order() &&
                response->jet_coefficients.size() == view.jet_order() + 1U &&
                std::all_of(response->jet_coefficients.begin(),
                            response->jet_coefficients.end(),
                            [&](const auto& coefficient) {
                                return coefficient.size() == view.order();
                            });
            out.response_seal_valid = response->response_seal ==
                germ_response_seal(view, response->jet_coefficients,
                                   response->audit_modulus);
            out.no_feedback =
                !response->ledger.ordinary_result_fed_back_to_angel &&
                response->ledger.angel_nodes_rewritten == 0U &&
                response->ledger.angel_nodes_merged == 0U;
        } else {
            const auto& limit = std::get<SelfClosureGermResourceLimit>(download);
            out.integrity_valid = limit.integrity.accepted();
            out.dimensions_valid = limit.requested_order == view.order() &&
                limit.requested_cycles == view.cycles() &&
                limit.requested_jet_order == view.jet_order();
            out.response_seal_valid = limit.certificate_seal ==
                angel::afac56::mix64(
                    view.view_seal() ^ std::rotl(limit.requested_order, 7) ^
                    std::rotl(limit.requested_cycles, 19) ^
                    std::rotl(limit.requested_jet_order, 31) ^
                    std::rotl(limit.maximum_order, 43) ^
                    std::rotl(limit.maximum_cycles, 51) ^
                    std::rotl(limit.maximum_jet_order, 59) ^
                    0x5245534c494d3632ULL);
            out.no_feedback = !limit.integrity.ordinary_result_fed_back_to_angel;
        }
        out.accepted = out.view_valid && out.integrity_valid &&
                       out.dimensions_valid && out.response_seal_valid &&
                       out.no_feedback;
        return out;
    }
};

// ------------------------- freeze-after-run audit -------------------------

[[nodiscard]] inline std::uint64_t evaluate_cyclic_column_at(
    const std::vector<std::uint64_t>& column,
    const std::uint64_t eigenvalue,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) noexcept {
    std::uint64_t result = 0U;
    std::uint64_t power = 1U;
    for (const auto coefficient : column) {
        result = angel::afac61::add_mod(
            result, angel::afac61::mul_mod(coefficient, power, modulus),
            modulus);
        power = angel::afac61::mul_mod(power, eigenvalue, modulus);
        if (ledger) ++ledger->audit_mode_evaluations;
    }
    return result;
}

[[nodiscard]] inline std::uint64_t mode_exact_period_audit(
    const std::uint64_t order, const std::uint64_t mode_index,
    SelfClosureGermLedger* ledger = nullptr) noexcept {
    if (ledger) ++ledger->audit_number_theory_steps;
    return mode_index == 0U ? 1U : order / std::gcd(order, mode_index);
}

[[nodiscard]] inline std::uint64_t factorial_mod_audit(
    const std::uint64_t n, const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) noexcept {
    std::uint64_t value = 1U;
    for (std::uint64_t k = 2U; k <= n; ++k) {
        value = angel::afac61::mul_mod(value, k % modulus, modulus);
        if (ledger) ++ledger->audit_number_theory_steps;
    }
    return value;
}

[[nodiscard]] inline std::uint64_t expected_leading_jet_coefficient_audit(
    const std::uint64_t exact_period,
    const std::uint64_t valuation,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) noexcept {
    auto coefficient = angel::afac61::pow_mod(
        exact_period % modulus, 2U * valuation, modulus);
    coefficient = angel::afac61::mul_mod(
        coefficient, factorial_mod_audit(valuation, modulus, ledger), modulus);
    if ((valuation & 1U) != 0U && coefficient != 0U)
        coefficient = modulus - coefficient;
    return coefficient;
}

[[nodiscard]] inline std::uint64_t first_nonzero_jet_degree_audit(
    const CyclicJetTensor& tensor,
    const std::uint64_t eigenvalue,
    const std::uint64_t modulus,
    SelfClosureGermLedger* ledger = nullptr) noexcept {
    for (std::uint64_t degree = 0U; degree < tensor.size(); ++degree)
        if (evaluate_cyclic_column_at(
                tensor[degree], eigenvalue, modulus, ledger) != 0U)
            return degree;
    return static_cast<std::uint64_t>(tensor.size());
}

[[nodiscard]] inline std::vector<std::uint64_t> scale_column(
    const std::vector<std::uint64_t>& column,
    const std::uint64_t scalar,
    const std::uint64_t modulus) {
    auto out = column;
    for (auto& value : out)
        value = angel::afac61::mul_mod(value, scalar, modulus);
    return out;
}

// Named forbidden bridges.
angel::afac57::CertifiedPrincipalJetState compress_angel_state_from_self_closure_germ(
    const CertifiedSelfClosureGermView&) = delete;
angel::afac59::AngelRunningState feed_self_closure_germ_back_to_angel(
    const SelfClosureGermDownload&) = delete;
CertifiedSelfClosureGermView forge_self_closure_germ_without_r61_view(
    std::uint64_t, std::uint64_t, std::uint64_t) = delete;

static_assert(!std::is_default_constructible_v<CertifiedSelfClosureGermView>);
static_assert(!std::is_convertible_v<CertifiedSelfClosureGermView,
                                     angel::afac57::CertifiedPrincipalJetState>);
static_assert(!std::is_convertible_v<SelfClosureGermDownload,
                                     angel::afac59::AngelRunningState>);

} // namespace angel::afac62
