#include "angel/observation_integrity.hpp"
#include "internal/conversions.hpp"
#include "internal/models.hpp"

#include <variant>

namespace angel::boundary {
namespace {

[[nodiscard]] OrdinaryObservation public_observation(
    const detail::frozen::cyclic_boundary::OrdinaryBoundaryObservation& value) {
    if (const auto* order = std::get_if<
            detail::frozen::cyclic_boundary::OrdinaryOrderObservation>(&value)) {
        return OrderObservation{order->order, order->observer_version};
    }
    const auto& primitive = std::get<
        detail::frozen::cyclic_boundary::PrimitiveClosureObservation>(value);
    return PrimitiveObservation{
        primitive.order,
        primitive.first_visible_jet,
        primitive.primitive_column,
        primitive.audit_modulus};
}

} // namespace

ObservationValidation validate_download_packet(
    const ClosedObservation& canonical,
    const DownloadPacket& derived) noexcept {
    const auto& closed = detail::BoundaryAccess::model(canonical).closed;
    const auto& state = closed.state();
    const auto& certificate = closed.certificate();

    ObservationValidation validation{};
    validation.canonical_state_bound =
        certificate.accepted && certificate.payload_already_bound &&
        certificate.no_deferred_execute &&
        certificate.scoped_not_global_future_equivalence &&
        certificate.order == state.order() &&
        certificate.cycles == state.cycles() &&
        certificate.source_session == state.session();

    validation.observation_matches =
        derived.observation == public_observation(closed.bound_observation());

    validation.causal_cut_matches =
        derived.cut.source_session == detail::from_frozen(state.session()) &&
        derived.cut.source_origin == detail::from_frozen(state.origin_kind()) &&
        derived.cut.observer == detail::from_frozen(certificate.observer) &&
        derived.cut.language == detail::from_frozen(certificate.language) &&
        derived.cut.discarded_cycle_multiplicity == state.cycles() &&
        derived.cut.discarded_continuation_epoch ==
            state.continuation_epoch() &&
        derived.cut.discarded_residual_generation ==
            state.singular_residual_generation();

    validation.terminal_only =
        derived.cut.no_resume_capability &&
        derived.cut.fresh_upload_required_for_reentry;
    return validation;
}

} // namespace angel::boundary
