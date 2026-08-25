#include "angel/boundary/evidence.hpp"
#include "angel/boundary/observation.hpp"
#include "angel/boundary/state.hpp"
#include "internal/conversions.hpp"

namespace angel::boundary {

StateSummary State::summary() const noexcept {
    const auto& state = detail::BoundaryAccess::model(*this).state;
    return StateSummary{
        state.order(),
        state.cycles(),
        state.source_order(),
        state.available_jet_order(),
        state.continuation_epoch(),
        state.singular_residual_generation(),
        detail::from_frozen(state.session()),
        detail::from_frozen(state.origin_kind()),
        detail::from_frozen(state.presentation())};
}

bool State::exactly_equal(const State& other) const noexcept {
    return detail::frozen::cyclic_boundary::BoundaryAuditView::exact_state_equal(
        detail::BoundaryAccess::model(*this).state,
        detail::BoundaryAccess::model(other).state);
}

bool State::presentation_equivalent(const State& other) const noexcept {
    return detail::frozen::cyclic_boundary::presentation_equivalent(
        detail::BoundaryAccess::model(*this).state,
        detail::BoundaryAccess::model(other).state);
}

ClosureCertificate ClosedObservation::certificate() const {
    const auto& certificate =
        detail::BoundaryAccess::model(*this).closed.certificate();
    return ClosureCertificate{
        detail::from_frozen(certificate.observer),
        detail::from_frozen(certificate.language),
        certificate.language_version,
        certificate.order,
        certificate.cycles,
        certificate.required_jet_horizon,
        certificate.available_jet_horizon,
        detail::from_frozen(certificate.source_session),
        detail::from_frozen(certificate.mode),
        certificate.payload_already_bound,
        certificate.no_deferred_execute,
        certificate.scoped_not_global_future_equivalence,
        certificate.accepted};
}

LeastCommitmentEvidence verify_least_commitment(
    const State& fresh, const State& compared) noexcept {
    const auto witness = detail::frozen::cyclic_boundary::
        ThreeGateBoundaryProtocol::verify_least_ramification(
            detail::BoundaryAccess::model(fresh).state,
            detail::BoundaryAccess::model(compared).state);
    return LeastCommitmentEvidence{
        witness.order,
        witness.fresh_cycles,
        witness.compared_cycles,
        witness.fresh_is_initial_in_divisibility_fibre,
        witness.accepted};
}

RamifiedTransportEvidence verify_ramified_transport(
    const std::uint64_t source_order,
    const std::uint64_t target_order,
    const std::uint64_t audit_modulus,
    BoundaryLedger* ledger) {
    detail::LedgerBridge bridge{ledger};
    const auto witness = detail::frozen::cyclic_boundary::
        ThreeGateBoundaryProtocol::reference_verify_ramified_compatibility(
            source_order, target_order, audit_modulus, bridge.pointer());
    return RamifiedTransportEvidence{
        witness.source_order,
        witness.target_order,
        witness.ramification_index,
        witness.exact_quotient_transport,
        witness.terminal_projector_collapses,
        witness.primitive_content_first_visible_at_ramification,
        witness.accepted};
}

} // namespace angel::boundary
