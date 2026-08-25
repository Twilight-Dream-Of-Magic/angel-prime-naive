#include "angel/cyclic_structure.hpp"
#include "internal/conversions.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace angel::prime {

namespace {

struct PrimeFactor final {
    std::uint64_t value{};
    std::uint32_t exponent{};
};

struct IndependentAudit final {
    bool ramanujan_valid{};
    bool normalized_checked{};
    bool valuation_valid{};
    bool constant_mode_valid{};
    bool kernel_checked{};
    bool kernel_valid{};
    bool classification_valid{};
    std::uint64_t number_theory_steps{};
    std::uint64_t relation_updates{};

    [[nodiscard]] bool accepted() const noexcept {
        return ramanujan_valid &&
               (!normalized_checked ||
                (valuation_valid && constant_mode_valid)) &&
               (!kernel_checked || kernel_valid) && classification_valid;
    }
};

[[nodiscard]] std::vector<PrimeFactor> factor_order(
    std::uint64_t value, std::uint64_t& steps) {
    std::vector<PrimeFactor> factors;
    for (std::uint64_t divisor = 2U; divisor <= value / divisor;
         divisor += divisor == 2U ? 1U : 2U) {
        ++steps;
        if (value % divisor != 0U) continue;
        PrimeFactor factor{divisor, 0U};
        do {
            value /= divisor;
            ++factor.exponent;
            ++steps;
        } while (value % divisor == 0U);
        factors.push_back(factor);
    }
    if (value > 1U) factors.push_back(PrimeFactor{value, 1U});
    return factors;
}

[[nodiscard]] std::uint64_t euler_phi(
    const std::uint64_t order,
    const std::vector<PrimeFactor>& factors,
    std::uint64_t& steps) noexcept {
    std::uint64_t result = order;
    for (const auto factor : factors) {
        result -= result / factor.value;
        ++steps;
    }
    return result;
}

struct QuotientArithmetic final {
    int mobius{1};
    std::uint64_t phi{1U};
};

[[nodiscard]] QuotientArithmetic quotient_arithmetic(
    const std::uint64_t quotient,
    const std::vector<PrimeFactor>& factors,
    std::uint64_t& steps) noexcept {
    std::uint64_t remainder = quotient;
    int mobius = 1;
    std::uint64_t phi = quotient;
    for (const auto factor : factors) {
        if (remainder % factor.value != 0U) {
            ++steps;
            continue;
        }
        phi -= phi / factor.value;
        std::uint32_t exponent = 0U;
        do {
            remainder /= factor.value;
            ++exponent;
            ++steps;
        } while (remainder % factor.value == 0U);
        if (exponent > 1U) mobius = 0;
        else if (mobius != 0) mobius = -mobius;
    }
    ++steps;
    return QuotientArithmetic{mobius, phi};
}

[[nodiscard]] std::uint64_t signed_residue(
    const int sign,
    const std::uint64_t magnitude,
    const std::uint64_t modulus) noexcept {
    if (sign == 0 || magnitude == 0U) return 0U;
    const auto reduced = magnitude % modulus;
    return sign > 0 || reduced == 0U ? reduced : modulus - reduced;
}

[[nodiscard]] bool verify_ramanujan_column(
    const detail::frozen::cyclic_action::SelfCyclicReferenceResponse& response,
    const std::vector<PrimeFactor>& factors,
    const std::uint64_t phi_order,
    std::uint64_t& steps) noexcept {
    if (response.unnormalized_cyclic_column.size() != response.order)
        return false;
    for (std::uint64_t index = 0U; index < response.order; ++index) {
        const auto coefficient = response.unnormalized_cyclic_column[index];
        if (coefficient >= response.audit_modulus) return false;
        const auto common = std::gcd(response.order, index);
        const auto quotient = response.order / common;
        const auto arithmetic = quotient_arithmetic(quotient, factors, steps);
        const auto magnitude = arithmetic.mobius == 0
            ? 0U
            : phi_order / arithmetic.phi;
        const auto expected = signed_residue(
            arithmetic.mobius, magnitude, response.audit_modulus);
        ++steps;
        if (coefficient != expected) return false;
    }
    return true;
}

[[nodiscard]] std::uint64_t add_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= modulus - right
        ? left - (modulus - right)
        : left + right;
}

[[nodiscard]] std::uint64_t sub_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    return left >= right ? left - right : modulus - (right - left);
}

[[nodiscard]] std::uint64_t mul_mod(
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t modulus) noexcept {
    __extension__ using Wide = unsigned __int128;
    return static_cast<std::uint64_t>(
        (static_cast<Wide>(left) * right) % modulus);
}

[[nodiscard]] std::vector<std::uint64_t> reattach_valuation(
    std::vector<std::uint64_t> value,
    const std::uint64_t modulus,
    std::uint64_t& updates) {
    const auto order = static_cast<std::uint64_t>(value.size());
    std::vector<std::uint64_t> next(order, 0U);
    for (std::uint64_t round = 0U; round + 1U < order; ++round) {
        for (std::uint64_t index = 0U; index < order; ++index) {
            next[(index + 1U) % order] = add_mod(
                next[(index + 1U) % order], value[index], modulus);
            next[index] = sub_mod(next[index], value[index], modulus);
            updates += 2U;
        }
        value.swap(next);
        std::fill(next.begin(), next.end(), 0U);
    }
    return value;
}

[[nodiscard]] bool verify_constant_mode(
    const detail::frozen::cyclic_action::SelfCyclicReferenceResponse& response,
    std::uint64_t& updates) noexcept {
    std::uint64_t constant_mode = 0U;
    for (const auto coefficient : response.normalized_cyclic_column) {
        if (coefficient >= response.audit_modulus) return false;
        constant_mode = add_mod(
            constant_mode, coefficient, response.audit_modulus);
        ++updates;
    }
    std::uint64_t factorial = 1U % response.audit_modulus;
    for (std::uint64_t value = 1U; value < response.order; ++value) {
        factorial = mul_mod(
            factorial, value % response.audit_modulus,
            response.audit_modulus);
        ++updates;
    }
    if (((response.order - 1U) & 1U) != 0U && factorial != 0U)
        factorial = response.audit_modulus - factorial;
    return constant_mode == factorial;
}

[[nodiscard]] IndependentAudit independently_verify(
    const detail::frozen::cyclic_action::SelfCyclicReferenceResponse& response) {
    IndependentAudit audit{};
    auto factors = factor_order(response.order, audit.number_theory_steps);
    const auto phi_order = euler_phi(
        response.order, factors, audit.number_theory_steps);
    audit.ramanujan_valid = verify_ramanujan_column(
        response, factors, phi_order, audit.number_theory_steps);

    const auto expected_primitive_only = phi_order == response.order - 1U;
    audit.classification_valid =
        response.prime_projector_shape == expected_primitive_only;

    audit.normalized_checked =
        !response.normalized_cyclic_column.empty();
    if (audit.normalized_checked) {
        const auto reattached = reattach_valuation(
            response.normalized_cyclic_column,
            response.audit_modulus,
            audit.relation_updates);
        audit.valuation_valid =
            reattached == response.unnormalized_cyclic_column;
        audit.constant_mode_valid = verify_constant_mode(
            response, audit.relation_updates);
    }

    audit.kernel_checked = response.normalized_kernel_degree_computed;
    if (audit.kernel_checked) {
        const auto expected_kernel = response.order - 1U - phi_order;
        audit.kernel_valid =
            !response.normalized_cyclic_column.empty() &&
            response.normalized_kernel_degree_mod_audit_field ==
                expected_kernel &&
            (expected_kernel == 0U) == expected_primitive_only;
    }
    return audit;
}

[[nodiscard]] detail::frozen::cyclic_action::SelfCyclicProbePolicy
to_frozen(const CyclicActionPolicy& policy) noexcept {
    return detail::frozen::cyclic_action::SelfCyclicProbePolicy{
        policy.maximum_order,
        policy.audit_modulus,
        policy.include_normalized_action,
        policy.compute_kernel_dimension};
}

[[nodiscard]] detail::frozen::cyclic_action::SelfCyclicStateIntegrityWitness
snapshot_integrity(
    const detail::frozen::factorial_boundary::CertifiedWilsonRequest& request)
    noexcept {
    const auto& state = request.factorial_execution().principal_jet;
    detail::frozen::cyclic_action::SelfCyclicStateIntegrityWitness witness{};
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

[[nodiscard]] detail::frozen::cyclic_action::SelfCyclicProbeDownload
make_resource_limit(
    const detail::frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const detail::frozen::cyclic_action::CertifiedSelfCyclicActionView& view,
    const CyclicActionPolicy& policy) noexcept {
    detail::frozen::cyclic_action::SelfCyclicResourceLimit limit{};
    limit.requested_order = view.order();
    limit.maximum_order = policy.maximum_order;
    limit.integrity = snapshot_integrity(request);
    limit.certificate_seal = detail::frozen::hashing::mix64(
        view.view_seal() ^ std::rotl(limit.requested_order, 17) ^
        std::rotl(limit.maximum_order, 43) ^ 0x5245534c494d3631ULL);
    return limit;
}

[[nodiscard]] CyclicObservationCertificate certificate_from(
    const detail::ClosedCyclicActionModel& model) noexcept {
    if (const auto* response = std::get_if<
            detail::frozen::cyclic_action::SelfCyclicReferenceResponse>(
            &model.download)) {
        return CyclicObservationCertificate{
            response->order,
            model.view.view_seal(),
            response->response_seal,
            model.policy.maximum_order,
            true,
            false,
            true,
            true};
    }
    const auto& limit = std::get<
        detail::frozen::cyclic_action::SelfCyclicResourceLimit>(
        model.download);
    return CyclicObservationCertificate{
        limit.requested_order,
        model.view.view_seal(),
        limit.certificate_seal,
        limit.maximum_order,
        true,
        true,
        true,
        true};
}

[[nodiscard]] StateIntegrity integrity_from(
    const detail::frozen::cyclic_action::SelfCyclicStateIntegrityWitness& witness,
    const std::uint64_t view_seal,
    const bool verified) noexcept {
    return StateIntegrity{
        witness.request_binding_before,
        witness.request_binding_after,
        witness.state_seal_before,
        witness.state_seal_after,
        witness.program_seal_before,
        witness.program_seal_after,
        view_seal,
        view_seal,
        witness.payload_bytes_before,
        witness.payload_bytes_after,
        witness.angel_nodes_rewritten,
        witness.angel_nodes_merged,
        witness.angel_state_compressed,
        witness.ordinary_result_fed_back_to_angel,
        verified};
}

[[nodiscard]] CyclicActionLedger ledger_from(
    const detail::frozen::cyclic_action::SelfCyclicProbeLedger& ledger,
    const IndependentAudit& audit) noexcept {
    return CyclicActionLedger{
        ledger.qpoch_factors_applied,
        ledger.cyclic_coefficient_updates,
        ledger.normalized_window_updates,
        ledger.polynomial_gcd_coefficient_steps,
        audit.number_theory_steps,
        audit.relation_updates,
        ledger.maximum_live_coefficients,
        ledger.angel_nodes_rewritten,
        ledger.angel_nodes_merged,
        ledger.ordinary_result_fed_back_to_angel};
}

} // namespace

CyclicActionView operator|(
    const FactorialState& state, const BindCyclicAction) {
    const auto& request = detail::PrimeAccess::model(state).request;
    detail::frozen::cyclic_action::SelfCyclicActionBoundary boundary;
    auto view = boundary.bind(request);
    return detail::CyclicStructureAccess::make_view(request, std::move(view));
}

ClosedCyclicAction operator|(
    const CyclicActionView& public_view,
    const EvaluateCyclicAction operation) {
    const auto& model = detail::CyclicStructureAccess::model(public_view);
    const auto binding = detail::frozen::cyclic_action::
        SelfCyclicViewVerifier::verify(model.request, model.view);
    if (!binding.accepted)
        throw std::invalid_argument("cyclic action view binding rejected");

    detail::frozen::cyclic_action::SelfCyclicProbeDownload download =
        model.view.order() > operation.policy.maximum_order
        ? make_resource_limit(model.request, model.view, operation.policy)
        : detail::frozen::cyclic_action::SelfCyclicActionObserver{}.observe(
            model.request, model.view, to_frozen(operation.policy));

    return detail::CyclicStructureAccess::make_closed(
        model.request, model.view, std::move(download), operation.policy);
}

CyclicStructureDownload operator|(
    const ClosedCyclicAction& public_closed,
    const DownloadCyclicStructure) {
    const auto& model = detail::CyclicStructureAccess::model(public_closed);
    const auto frozen_verification = detail::frozen::cyclic_action::
        SelfCyclicResponseVerifier::verify(
            model.request, model.view, model.download);

    CyclicStructureDownload result{};
    result.certificate = certificate_from(model);
    result.verification.frozen_binding_valid = frozen_verification.view_valid;
    result.verification.response_integrity_valid =
        frozen_verification.integrity_valid;
    result.verification.dimensions_valid =
        frozen_verification.dimensions_valid;
    result.verification.response_seal_valid =
        frozen_verification.response_seal_valid;
    result.verification.no_feedback = frozen_verification.no_feedback;

    if (const auto* response = std::get_if<
            detail::frozen::cyclic_action::SelfCyclicReferenceResponse>(
            &model.download)) {
        const auto audit = independently_verify(*response);
        result.verification.response_materialized = true;
        result.verification.ramanujan_replay_checked = true;
        result.verification.ramanujan_replay_valid = audit.ramanujan_valid;
        result.verification.normalized_action_checked =
            audit.normalized_checked;
        result.verification.valuation_reattachment_valid =
            audit.valuation_valid;
        result.verification.constant_mode_factorial_valid =
            audit.constant_mode_valid;
        result.verification.kernel_dimension_checked = audit.kernel_checked;
        result.verification.kernel_dimension_valid = audit.kernel_valid;
        result.verification.period_classification_valid =
            audit.classification_valid;
        result.verification.accepted =
            frozen_verification.accepted && audit.accepted();
        result.ledger = ledger_from(response->ledger, audit);
        result.integrity = integrity_from(
            response->integrity, model.view.view_seal(),
            result.verification.accepted);

        CyclicStructureObservation observation{};
        observation.order = response->order;
        observation.audit_modulus = response->audit_modulus;
        observation.primitive_period_response =
            response->unnormalized_cyclic_column;
        observation.normalized_action_response =
            response->normalized_cyclic_column;
        if (response->normalized_kernel_degree_computed)
            observation.proper_period_kernel_dimension =
                response->normalized_kernel_degree_mod_audit_field;
        observation.structure = response->prime_projector_shape
            ? PeriodStructure::PrimitiveOnly
            : PeriodStructure::ProperPeriodKernel;
        result.observation = std::move(observation);
        return result;
    }

    const auto& limit = std::get<
        detail::frozen::cyclic_action::SelfCyclicResourceLimit>(model.download);
    result.observation = CyclicStructureResourceLimit{
        limit.requested_order, limit.maximum_order};
    result.verification.response_materialized = false;
    result.verification.ramanujan_replay_checked = false;
    result.verification.accepted = frozen_verification.accepted;
    result.integrity = integrity_from(
        limit.integrity, model.view.view_seal(),
        result.verification.accepted);
    return result;
}

CyclicActionViewSummary CyclicActionView::summary() const noexcept {
    const auto& view = detail::CyclicStructureAccess::model(*this).view;
    return CyclicActionViewSummary{
        view.order(),
        view.represented_factor_count(),
        view.request_binding(),
        view.principal_state_seal(),
        view.source_program_seal(),
        view.valuation_hash(),
        view.view_seal(),
        static_cast<std::uint64_t>(view.descriptor_words()),
        view.contains_angel_state(),
        view.compresses_angel_state(),
        view.can_feed_back_to_angel()};
}

bool CyclicActionView::preserves_complete_state_identity() const noexcept {
    const auto& model = detail::CyclicStructureAccess::model(*this);
    return detail::frozen::cyclic_action::SelfCyclicViewVerifier::verify(
        model.request, model.view).accepted;
}

CyclicObservationCertificate ClosedCyclicAction::certificate() const noexcept {
    return certificate_from(detail::CyclicStructureAccess::model(*this));
}

} // namespace angel::prime
