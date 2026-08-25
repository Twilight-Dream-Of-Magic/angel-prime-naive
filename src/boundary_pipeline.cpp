#include "angel/boundary/pipeline.hpp"
#include "internal/conversions.hpp"

#include <stdexcept>
#include <utility>

namespace angel::boundary {

namespace {

[[noreturn]] void reject(const char* operation) {
    throw std::invalid_argument(operation);
}

} // namespace

State operator|(const EncodedOrder& input, const UploadOperation operation) {
    if (operation.authority == nullptr)
        reject("upload requires a live session authority");
    detail::LedgerBridge bridge{operation.ledger};
    auto state = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::upload(
        detail::to_frozen(input),
        detail::BoundaryAccess::model(*operation.authority).authority,
        bridge.pointer());
    if (!state) reject("the encoded cyclic order was rejected");
    return detail::BoundaryAccess::make_state(std::move(*state));
}

State operator|(
    const State& state, const ChangePresentationOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    detail::frozen::cyclic_boundary::PresentationTransportCertificate evidence{};
    auto changed = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        change_presentation(
            detail::BoundaryAccess::model(state).state,
            detail::to_frozen(operation.target),
            operation.evidence ? &evidence : nullptr,
            bridge.pointer());
    if (!changed) reject("presentation transport was rejected");
    if (operation.evidence) {
        *operation.evidence = PresentationEvidence{
            detail::from_frozen(evidence.source),
            detail::from_frozen(evidence.target),
            evidence.forward_exponent,
            evidence.backward_exponent,
            evidence.exact_inverse,
            evidence.accepted};
    }
    return detail::BoundaryAccess::make_state(std::move(*changed));
}

State operator|(const State& state, const QuotientOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    auto result = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        native_quotient(
            detail::BoundaryAccess::model(state).state,
            operation.target_order,
            bridge.pointer());
    if (!result) reject("native quotient was rejected");
    return detail::BoundaryAccess::make_state(std::move(*result));
}

State operator|(const State& state, const ContinueOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    auto result = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        native_same_frame_continue(
            detail::BoundaryAccess::model(state).state,
            operation.available_jet_order,
            bridge.pointer());
    return detail::BoundaryAccess::make_state(std::move(result));
}

NativeCheckpoint operator|(
    const State& state, const ExportCheckpointOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    auto checkpoint = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        native_export(detail::BoundaryAccess::model(state).state, bridge.pointer());
    return detail::BoundaryAccess::make_checkpoint(std::move(checkpoint));
}

State operator|(
    const NativeCheckpoint& checkpoint, const ImportCheckpointOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    auto state = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        native_import(
            detail::BoundaryAccess::model(checkpoint).checkpoint,
            bridge.pointer());
    if (!state) reject("native checkpoint import was rejected");
    return detail::BoundaryAccess::make_state(std::move(*state));
}

ClosedObservation operator|(
    const State& state, const ObserveOrderOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    const detail::frozen::cyclic_boundary::ObservationContract contract{
        detail::frozen::cyclic_boundary::ObserverKind::OrderOnlyV1,
        detail::to_frozen(operation.language),
        operation.language_version,
        0U};
    auto closed = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        close_order_observation(
            detail::BoundaryAccess::model(state).state,
            contract,
            bridge.pointer());
    if (!closed) reject("order observation did not close");
    return detail::BoundaryAccess::make_closed(std::move(*closed));
}

ClosedObservation operator|(
    const State& state, const ObservePrimitiveOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    const detail::frozen::cyclic_boundary::ObservationContract contract{
        detail::frozen::cyclic_boundary::ObserverKind::PrimitiveClosureJetV1,
        detail::frozen::cyclic_boundary::ContinuationLanguage::FrozenTerminalV1,
        operation.language_version,
        operation.declared_jet_horizon};
    auto closed = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        reference_close_primitive_observation(
            detail::BoundaryAccess::model(state).state,
            contract,
            operation.audit_modulus,
            bridge.pointer());
    if (!closed) reject("primitive observation did not close");
    return detail::BoundaryAccess::make_closed(std::move(*closed));
}

DownloadPacket operator|(
    const ClosedObservation& closed, const DownloadOperation operation) {
    detail::LedgerBridge bridge{operation.ledger};
    const auto packet = detail::frozen::cyclic_boundary::ThreeGateBoundaryProtocol::
        download(detail::BoundaryAccess::model(closed).closed, bridge.pointer());

    OrdinaryObservation observation{};
    if (const auto* order = std::get_if<
            detail::frozen::cyclic_boundary::OrdinaryOrderObservation>(
            &packet.observation)) {
        observation = OrderObservation{order->order, order->observer_version};
    } else {
        const auto& primitive = std::get<
            detail::frozen::cyclic_boundary::PrimitiveClosureObservation>(
            packet.observation);
        observation = PrimitiveObservation{
            primitive.order,
            primitive.first_visible_jet,
            primitive.primitive_column,
            primitive.audit_modulus};
    }

    return DownloadPacket{
        std::move(observation),
        CausalCut{
            detail::from_frozen(packet.cut.source_session),
            detail::from_frozen(packet.cut.source_origin_kind),
            detail::from_frozen(packet.cut.observer),
            detail::from_frozen(packet.cut.language),
            packet.cut.discarded_cycle_multiplicity,
            packet.cut.discarded_continuation_epoch,
            packet.cut.discarded_singular_residual_generation,
            packet.cut.no_resume_capability,
            packet.cut.fresh_upload_required_for_reentry}};
}

} // namespace angel::boundary
