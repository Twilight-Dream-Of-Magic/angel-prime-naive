#pragma once

#include "angel/afac62/self_closure_germ.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac63 {

// R63 is a scoped boundary-protocol experiment over the proved R61/R62 cyclic
// chart.  It does not implement Prime Birth and does not replace the complete
// R54 Angel state.  It isolates the compatibility laws that Upload, native
// continuation transport, and Download must satisfy without allowing ordinary
// observations to steer high-dimensional execution.

// ---------------------------------------------------------------------------
// Gate 1: exact ordinary specification and representation normalization
// ---------------------------------------------------------------------------

enum class ByteOrder : std::uint8_t {
    LittleEndian = 1,
    BigEndian = 2
};

enum class BoundarySchema : std::uint16_t {
    CyclicOrderV1 = 0x6301U
};

struct EncodedCyclicOrderSpec final {
    std::array<std::uint8_t, 8> bytes{};
    std::uint8_t byte_count{1U};
    ByteOrder byte_order{ByteOrder::LittleEndian};
    BoundarySchema schema{BoundarySchema::CyclicOrderV1};

    friend bool operator==(const EncodedCyclicOrderSpec&,
                           const EncodedCyclicOrderSpec&) = default;
};

struct RepresentationNormalizationWitness final {
    EncodedCyclicOrderSpec source{};
    std::uint64_t canonical_order{};
    std::uint8_t canonical_byte_count{};
    std::uint8_t removed_high_zero_bytes{};
    bool endian_reordered{};
    bool exact_discrete_value_preserved{};

    friend bool operator==(const RepresentationNormalizationWitness&,
                           const RepresentationNormalizationWitness&) = default;
};

class CertifiedCyclicOrderSpec final {
public:
    CertifiedCyclicOrderSpec() = delete;
    CertifiedCyclicOrderSpec(const CertifiedCyclicOrderSpec&) = default;

    [[nodiscard]] std::uint64_t order() const noexcept { return order_; }
    [[nodiscard]] BoundarySchema schema() const noexcept { return schema_; }
    [[nodiscard]] const RepresentationNormalizationWitness& witness() const noexcept {
        return witness_;
    }

private:
    std::uint64_t order_{};
    BoundarySchema schema_{BoundarySchema::CyclicOrderV1};
    RepresentationNormalizationWitness witness_{};

    CertifiedCyclicOrderSpec(const std::uint64_t order,
                             const BoundarySchema schema,
                             RepresentationNormalizationWitness witness) noexcept
        : order_(order), schema_(schema), witness_(std::move(witness)) {}

    friend class SpecificationGate;
};

class SpecificationGate final {
public:
    [[nodiscard]] static std::optional<CertifiedCyclicOrderSpec> normalize(
        const EncodedCyclicOrderSpec& encoded) noexcept {
        if (encoded.schema != BoundarySchema::CyclicOrderV1 ||
            encoded.byte_count == 0U ||
            encoded.byte_count > encoded.bytes.size())
            return std::nullopt;

        std::array<std::uint8_t, 8> little{};
        for (std::uint8_t i = 0U; i < encoded.byte_count; ++i) {
            const auto source_index = encoded.byte_order == ByteOrder::LittleEndian
                ? i
                : static_cast<std::uint8_t>(encoded.byte_count - 1U - i);
            little[i] = encoded.bytes[source_index];
        }

        std::uint8_t canonical_count = encoded.byte_count;
        while (canonical_count > 1U && little[canonical_count - 1U] == 0U)
            --canonical_count;

        std::uint64_t order = 0U;
        for (std::uint8_t i = 0U; i < canonical_count; ++i)
            order |= static_cast<std::uint64_t>(little[i]) << (8U * i);
        if (order < 2U) return std::nullopt;

        RepresentationNormalizationWitness witness{};
        witness.source = encoded;
        witness.canonical_order = order;
        witness.canonical_byte_count = canonical_count;
        witness.removed_high_zero_bytes =
            static_cast<std::uint8_t>(encoded.byte_count - canonical_count);
        witness.endian_reordered = encoded.byte_order == ByteOrder::BigEndian;
        witness.exact_discrete_value_preserved = true;
        return CertifiedCyclicOrderSpec{order, encoded.schema, witness};
    }

    [[nodiscard]] static EncodedCyclicOrderSpec encode_canonical(
        const std::uint64_t order) {
        if (order < 2U)
            throw std::invalid_argument("R63 requires cyclic order >= 2");
        EncodedCyclicOrderSpec out{};
        out.byte_order = ByteOrder::LittleEndian;
        out.byte_count = 1U;
        auto value = order;
        for (std::uint8_t i = 0U; i < out.bytes.size(); ++i) {
            out.bytes[i] = static_cast<std::uint8_t>(value & 0xffU);
            value >>= 8U;
            if (value != 0U)
                out.byte_count = static_cast<std::uint8_t>(i + 2U);
        }
        return out;
    }
};

// ---------------------------------------------------------------------------
// Gate 2: fresh least-ramification high origin and presentation groupoid
// ---------------------------------------------------------------------------

struct SessionId final {
    std::uint64_t authority{};
    std::uint64_t sequence{};

    [[nodiscard]] bool valid() const noexcept {
        return authority != 0U && sequence != 0U;
    }
    friend bool operator==(const SessionId&, const SessionId&) = default;
};

class SessionAuthority final {
public:
    explicit SessionAuthority(const std::uint64_t authority) : authority_(authority) {
        if (authority_ == 0U)
            throw std::invalid_argument("R63 session authority must be nonzero");
    }

    [[nodiscard]] SessionId issue_fresh() {
        if (next_ == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("R63 session sequence exhausted");
        ++next_;
        return SessionId{authority_, next_};
    }

private:
    std::uint64_t authority_{};
    std::uint64_t next_{};
};

struct CyclicPresentation final {
    // The generator is represented as S^exponent.  inverse_exponent is a
    // proof-carrying witness that exponent is a unit modulo order.
    std::uint64_t order{};
    std::uint64_t exponent{1U};
    std::uint64_t inverse_exponent{1U};

    [[nodiscard]] bool valid() const noexcept {
        if (order < 2U || exponent == 0U || exponent >= order ||
            inverse_exponent == 0U || inverse_exponent >= order)
            return false;
        return angel::afac61::mul_mod(exponent, inverse_exponent, order) == 1U;
    }

    [[nodiscard]] static CyclicPresentation standard(const std::uint64_t order) {
        if (order < 2U)
            throw std::invalid_argument("R63 invalid standard presentation order");
        return CyclicPresentation{order, 1U, 1U};
    }

    friend bool operator==(const CyclicPresentation&,
                           const CyclicPresentation&) = default;
};

struct PresentationTransportCertificate final {
    CyclicPresentation source{};
    CyclicPresentation target{};
    std::uint64_t forward_exponent{};
    std::uint64_t backward_exponent{};
    bool exact_inverse{};
    bool accepted{};
};

[[nodiscard]] inline PresentationTransportCertificate certify_presentation_transport(
    const CyclicPresentation& source,
    const CyclicPresentation& target) noexcept {
    PresentationTransportCertificate cert{};
    cert.source = source;
    cert.target = target;
    if (!source.valid() || !target.valid() || source.order != target.order)
        return cert;
    cert.forward_exponent = angel::afac61::mul_mod(
        target.exponent, source.inverse_exponent, source.order);
    cert.backward_exponent = angel::afac61::mul_mod(
        source.exponent, target.inverse_exponent, source.order);
    cert.exact_inverse = angel::afac61::mul_mod(
        cert.forward_exponent, cert.backward_exponent, source.order) == 1U;
    cert.accepted = cert.exact_inverse;
    return cert;
}

enum class OriginKind : std::uint8_t {
    ExternalFreshUpload = 1,
    NativeRamifiedQuotient = 2,
    NativeContinuationImport = 3,
    NativeSameFrameContinuation = 4
};

struct BoundaryLedger final {
    std::uint64_t representation_normalizations{};
    std::uint64_t fresh_origins_created{};
    std::uint64_t presentation_transports{};
    std::uint64_t native_quotients{};
    std::uint64_t native_same_frame_continuations{};
    std::uint64_t native_exports{};
    std::uint64_t native_imports{};
    std::uint64_t closure_checks{};
    std::uint64_t reference_germ_materializations{};
    std::uint64_t dense_reference_coefficients{};
    std::uint64_t dense_reference_updates{};
    std::uint64_t downloads{};
    std::uint64_t angel_nodes_rewritten{};
    std::uint64_t angel_nodes_merged{};
    bool ordinary_observation_fed_back_to_native{};

    friend bool operator==(const BoundaryLedger&, const BoundaryLedger&) = default;
};

class CyclicBoundaryState final {
public:
    CyclicBoundaryState() = delete;
    CyclicBoundaryState(const CyclicBoundaryState&) = default;

    [[nodiscard]] std::uint64_t order() const noexcept { return order_; }
    [[nodiscard]] std::uint64_t cycles() const noexcept { return cycles_; }
    [[nodiscard]] std::uint64_t source_order() const noexcept { return source_order_; }
    [[nodiscard]] std::uint64_t available_jet_order() const noexcept {
        return available_jet_order_;
    }
    [[nodiscard]] std::uint64_t continuation_epoch() const noexcept {
        return continuation_epoch_;
    }
    [[nodiscard]] std::uint64_t singular_residual_generation() const noexcept {
        return singular_residual_generation_;
    }
    [[nodiscard]] SessionId session() const noexcept { return session_; }
    [[nodiscard]] OriginKind origin_kind() const noexcept { return origin_kind_; }
    [[nodiscard]] BoundarySchema schema() const noexcept { return schema_; }
    [[nodiscard]] const CyclicPresentation& presentation() const noexcept {
        return presentation_;
    }

private:
    std::uint64_t order_{};
    std::uint64_t cycles_{};
    std::uint64_t source_order_{};
    std::uint64_t available_jet_order_{};
    std::uint64_t continuation_epoch_{};
    std::uint64_t singular_residual_generation_{};
    SessionId session_{};
    OriginKind origin_kind_{OriginKind::ExternalFreshUpload};
    BoundarySchema schema_{BoundarySchema::CyclicOrderV1};
    CyclicPresentation presentation_{};

    CyclicBoundaryState(const std::uint64_t order,
                        const std::uint64_t cycles,
                        const std::uint64_t source_order,
                        const std::uint64_t available_jet_order,
                        const std::uint64_t continuation_epoch,
                        const std::uint64_t singular_residual_generation,
                        const SessionId session,
                        const OriginKind origin_kind,
                        const BoundarySchema schema,
                        CyclicPresentation presentation) noexcept
        : order_(order), cycles_(cycles), source_order_(source_order),
          available_jet_order_(available_jet_order),
          continuation_epoch_(continuation_epoch),
          singular_residual_generation_(singular_residual_generation),
          session_(session), origin_kind_(origin_kind), schema_(schema),
          presentation_(std::move(presentation)) {}

    friend class ThreeGateBoundaryProtocol;
    friend class BoundaryAuditView;
};

[[nodiscard]] inline bool same_scoped_semantic_state(
    const CyclicBoundaryState& left,
    const CyclicBoundaryState& right) noexcept {
    return left.order() == right.order() &&
           left.cycles() == right.cycles() &&
           left.source_order() == right.source_order() &&
           left.available_jet_order() == right.available_jet_order() &&
           left.continuation_epoch() == right.continuation_epoch() &&
           left.singular_residual_generation() ==
               right.singular_residual_generation() &&
           left.session() == right.session() &&
           left.schema() == right.schema();
}

[[nodiscard]] inline bool presentation_equivalent(
    const CyclicBoundaryState& left,
    const CyclicBoundaryState& right) noexcept {
    return same_scoped_semantic_state(left, right) &&
           certify_presentation_transport(left.presentation(),
                                          right.presentation()).accepted;
}

struct NativeContinuationCapsule final {
private:
    std::uint64_t order_{};
    std::uint64_t cycles_{};
    std::uint64_t source_order_{};
    std::uint64_t available_jet_order_{};
    std::uint64_t continuation_epoch_{};
    std::uint64_t singular_residual_generation_{};
    SessionId session_{};
    BoundarySchema schema_{BoundarySchema::CyclicOrderV1};
    CyclicPresentation presentation_{};

    NativeContinuationCapsule(const CyclicBoundaryState& state) noexcept
        : order_(state.order()), cycles_(state.cycles()),
          source_order_(state.source_order()),
          available_jet_order_(state.available_jet_order()),
          continuation_epoch_(state.continuation_epoch()),
          singular_residual_generation_(state.singular_residual_generation()),
          session_(state.session()), schema_(state.schema()),
          presentation_(state.presentation()) {}

    friend class ThreeGateBoundaryProtocol;
};

struct RamifiedCompatibilityWitness final {
    std::uint64_t source_order{};
    std::uint64_t target_order{};
    std::uint64_t ramification_index{};
    bool exact_quotient_transport{};
    bool terminal_projector_collapses{};
    bool primitive_content_first_visible_at_ramification{};
    bool accepted{};
};

struct LeastRamificationWitness final {
    std::uint64_t order{};
    std::uint64_t fresh_cycles{};
    std::uint64_t compared_cycles{};
    bool fresh_is_initial_in_divisibility_fibre{};
    bool accepted{};
};

// ---------------------------------------------------------------------------
// Gate 3: observer-indexed closure and irreversible ordinary descent
// ---------------------------------------------------------------------------

enum class ObserverKind : std::uint8_t {
    OrderOnlyV1 = 1,
    PrimitiveClosureJetV1 = 2
};

enum class ContinuationLanguage : std::uint8_t {
    FrozenTerminalV1 = 1,
    SameFrameGermContinuationV1 = 2
};

enum class ClosureMode : std::uint8_t {
    NativeScoped = 1,
    ReferenceOnly = 2
};

struct ObservationContract final {
    ObserverKind observer{ObserverKind::OrderOnlyV1};
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminalV1};
    std::uint64_t language_version{1U};
    std::uint64_t declared_jet_horizon{};

    friend bool operator==(const ObservationContract&,
                           const ObservationContract&) = default;
};

struct OrdinaryOrderObservation final {
    std::uint64_t order{};
    std::uint64_t observer_version{1U};

    friend bool operator==(const OrdinaryOrderObservation&,
                           const OrdinaryOrderObservation&) = default;
};

struct PrimitiveClosureObservation final {
    std::uint64_t order{};
    std::uint64_t first_visible_jet{};
    std::vector<std::uint64_t> primitive_column{};
    std::uint64_t audit_modulus{};

    friend bool operator==(const PrimitiveClosureObservation&,
                           const PrimitiveClosureObservation&) = default;
};

using OrdinaryBoundaryObservation =
    std::variant<OrdinaryOrderObservation, PrimitiveClosureObservation>;

struct ObservationClosureCertificate final {
    ObserverKind observer{ObserverKind::OrderOnlyV1};
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminalV1};
    std::uint64_t language_version{};
    std::uint64_t order{};
    std::uint64_t cycles{};
    std::uint64_t required_jet_horizon{};
    std::uint64_t available_jet_horizon{};
    SessionId source_session{};
    ClosureMode mode{ClosureMode::NativeScoped};
    bool payload_already_bound{};
    bool no_deferred_execute{};
    bool scoped_not_global_future_equivalence{};
    bool accepted{};

    friend bool operator==(const ObservationClosureCertificate&,
                           const ObservationClosureCertificate&) = default;
};

class ObservationClosedState final {
public:
    ObservationClosedState() = delete;
    ObservationClosedState(const ObservationClosedState&) = default;

    [[nodiscard]] const CyclicBoundaryState& state() const noexcept {
        return state_;
    }
    [[nodiscard]] const OrdinaryBoundaryObservation& bound_observation() const noexcept {
        return bound_observation_;
    }
    [[nodiscard]] const ObservationClosureCertificate& certificate() const noexcept {
        return certificate_;
    }

private:
    CyclicBoundaryState state_;
    OrdinaryBoundaryObservation bound_observation_{};
    ObservationClosureCertificate certificate_{};

    ObservationClosedState(CyclicBoundaryState state,
                           OrdinaryBoundaryObservation observation,
                           ObservationClosureCertificate certificate)
        : state_(std::move(state)),
          bound_observation_(std::move(observation)),
          certificate_(certificate) {}

    friend class ThreeGateBoundaryProtocol;
};

struct CausalCutReceipt final {
    SessionId source_session{};
    OriginKind source_origin_kind{OriginKind::ExternalFreshUpload};
    ObserverKind observer{ObserverKind::OrderOnlyV1};
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminalV1};
    std::uint64_t discarded_cycle_multiplicity{};
    std::uint64_t discarded_continuation_epoch{};
    std::uint64_t discarded_singular_residual_generation{};
    bool no_resume_capability{};
    bool fresh_upload_required_for_reentry{};

    friend bool operator==(const CausalCutReceipt&,
                           const CausalCutReceipt&) = default;
};

struct DownloadPacket final {
    OrdinaryBoundaryObservation observation{};
    CausalCutReceipt cut{};

    friend bool operator==(const DownloadPacket&, const DownloadPacket&) = default;
};

// ---------------------------------------------------------------------------
// The scoped protocol
// ---------------------------------------------------------------------------

class ThreeGateBoundaryProtocol final {
public:
    [[nodiscard]] static std::optional<CyclicBoundaryState> upload(
        const CertifiedCyclicOrderSpec& specification,
        SessionAuthority& authority,
        const CyclicPresentation& presentation,
        BoundaryLedger* ledger = nullptr) {
        if (specification.schema() != BoundarySchema::CyclicOrderV1 ||
            !presentation.valid() ||
            presentation.order != specification.order())
            return std::nullopt;
        if (ledger) {
            ++ledger->representation_normalizations;
            ++ledger->fresh_origins_created;
        }
        const auto session = authority.issue_fresh();
        return CyclicBoundaryState{
            specification.order(), 1U, specification.order(), 0U, 0U, 0U,
            session, OriginKind::ExternalFreshUpload, specification.schema(),
            presentation};
    }

    [[nodiscard]] static std::optional<CyclicBoundaryState> upload(
        const EncodedCyclicOrderSpec& encoded,
        SessionAuthority& authority,
        BoundaryLedger* ledger = nullptr) {
        const auto specification = SpecificationGate::normalize(encoded);
        if (!specification) return std::nullopt;
        return upload(*specification, authority,
                      CyclicPresentation::standard(specification->order()),
                      ledger);
    }

    [[nodiscard]] static std::optional<CyclicBoundaryState> change_presentation(
        const CyclicBoundaryState& source,
        const CyclicPresentation& target,
        PresentationTransportCertificate* certificate = nullptr,
        BoundaryLedger* ledger = nullptr) noexcept {
        const auto cert = certify_presentation_transport(source.presentation(), target);
        if (!cert.accepted) return std::nullopt;
        if (certificate) *certificate = cert;
        if (ledger) ++ledger->presentation_transports;
        return CyclicBoundaryState{
            source.order(), source.cycles(), source.source_order(),
            source.available_jet_order(), source.continuation_epoch(),
            source.singular_residual_generation(), source.session(),
            source.origin_kind(), source.schema(), target};
    }

    [[nodiscard]] static std::optional<CyclicBoundaryState> native_quotient(
        const CyclicBoundaryState& source,
        const std::uint64_t target_order,
        BoundaryLedger* ledger = nullptr) noexcept {
        if (target_order < 2U || source.order() % target_order != 0U)
            return std::nullopt;
        const auto quotient_index = source.order() / target_order;
        if (source.cycles() >
            std::numeric_limits<std::uint64_t>::max() / quotient_index)
            return std::nullopt;
        const auto target_cycles = source.cycles() * quotient_index;
        CyclicPresentation target_presentation{
            target_order,
            source.presentation().exponent % target_order,
            source.presentation().inverse_exponent % target_order};
        if (!target_presentation.valid()) return std::nullopt;
        if (ledger) ++ledger->native_quotients;
        return CyclicBoundaryState{
            target_order, target_cycles, source.source_order(),
            source.available_jet_order(), source.continuation_epoch(),
            source.singular_residual_generation(), source.session(),
            OriginKind::NativeRamifiedQuotient, source.schema(),
            target_presentation};
    }

    [[nodiscard]] static CyclicBoundaryState native_same_frame_continue(
        const CyclicBoundaryState& source,
        const std::uint64_t new_available_jet_order,
        BoundaryLedger* ledger = nullptr) {
        if (new_available_jet_order < source.available_jet_order())
            throw std::invalid_argument("R63 cannot shrink the retained jet horizon");
        if (source.continuation_epoch() ==
                std::numeric_limits<std::uint64_t>::max() ||
            source.singular_residual_generation() ==
                std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("R63 continuation counter exhausted");
        if (ledger) ++ledger->native_same_frame_continuations;
        return CyclicBoundaryState{
            source.order(), source.cycles(), source.source_order(),
            new_available_jet_order, source.continuation_epoch() + 1U,
            source.singular_residual_generation() + 1U, source.session(),
            OriginKind::NativeSameFrameContinuation, source.schema(),
            source.presentation()};
    }

    [[nodiscard]] static NativeContinuationCapsule native_export(
        const CyclicBoundaryState& state,
        BoundaryLedger* ledger = nullptr) noexcept {
        if (ledger) ++ledger->native_exports;
        return NativeContinuationCapsule{state};
    }

    [[nodiscard]] static std::optional<CyclicBoundaryState> native_import(
        const NativeContinuationCapsule& capsule,
        BoundaryLedger* ledger = nullptr) noexcept {
        if (capsule.schema_ != BoundarySchema::CyclicOrderV1 ||
            capsule.order_ < 2U || capsule.cycles_ == 0U ||
            capsule.source_order_ < capsule.order_ ||
            !capsule.session_.valid() || !capsule.presentation_.valid() ||
            capsule.presentation_.order != capsule.order_ ||
            capsule.source_order_ % capsule.order_ != 0U ||
            capsule.source_order_ / capsule.order_ != capsule.cycles_)
            return std::nullopt;
        if (ledger) ++ledger->native_imports;
        return CyclicBoundaryState{
            capsule.order_, capsule.cycles_, capsule.source_order_,
            capsule.available_jet_order_, capsule.continuation_epoch_,
            capsule.singular_residual_generation_, capsule.session_,
            OriginKind::NativeContinuationImport, capsule.schema_,
            capsule.presentation_};
    }

    [[nodiscard]] static LeastRamificationWitness verify_least_ramification(
        const CyclicBoundaryState& fresh,
        const CyclicBoundaryState& compared) noexcept {
        LeastRamificationWitness witness{};
        witness.order = fresh.order();
        witness.fresh_cycles = fresh.cycles();
        witness.compared_cycles = compared.cycles();
        witness.fresh_is_initial_in_divisibility_fibre =
            fresh.order() == compared.order() && fresh.cycles() == 1U &&
            compared.cycles() >= 1U && compared.cycles() % fresh.cycles() == 0U;
        witness.accepted = witness.fresh_is_initial_in_divisibility_fibre;
        return witness;
    }

    [[nodiscard]] static std::optional<ObservationClosedState> close_order_observation(
        const CyclicBoundaryState& state,
        const ObservationContract& contract,
        BoundaryLedger* ledger = nullptr) {
        if (ledger) ++ledger->closure_checks;
        if (contract.observer != ObserverKind::OrderOnlyV1 ||
            contract.language_version == 0U)
            return std::nullopt;
        if (contract.language != ContinuationLanguage::FrozenTerminalV1 &&
            contract.language != ContinuationLanguage::SameFrameGermContinuationV1)
            return std::nullopt;

        ObservationClosureCertificate certificate{};
        certificate.observer = contract.observer;
        certificate.language = contract.language;
        certificate.language_version = contract.language_version;
        certificate.order = state.order();
        certificate.cycles = state.cycles();
        certificate.required_jet_horizon = 0U;
        certificate.available_jet_horizon = state.available_jet_order();
        certificate.source_session = state.session();
        certificate.mode = ClosureMode::NativeScoped;
        certificate.payload_already_bound = true;
        certificate.no_deferred_execute = true;
        certificate.scoped_not_global_future_equivalence = true;
        certificate.accepted = true;
        return ObservationClosedState{
            state, OrdinaryOrderObservation{state.order(), 1U}, certificate};
    }

    [[nodiscard]] static std::optional<ObservationClosedState>
    reference_close_primitive_observation(
        const CyclicBoundaryState& state,
        const ObservationContract& contract,
        const std::uint64_t audit_modulus = 998244353U,
        BoundaryLedger* ledger = nullptr) {
        if (ledger) ++ledger->closure_checks;
        if (contract.observer != ObserverKind::PrimitiveClosureJetV1 ||
            contract.language_version == 0U ||
            contract.language != ContinuationLanguage::FrozenTerminalV1)
            return std::nullopt;
        const auto required = state.cycles();
        if (contract.declared_jet_horizon < required ||
            state.available_jet_order() < required)
            return std::nullopt;

        angel::afac62::SelfClosureGermLedger parent_ledger{};
        const auto tensor = angel::afac62::materialize_self_closure_germ(
            state.order(), state.cycles(), required, audit_modulus,
            &parent_ledger);
        if (required >= tensor.size()) return std::nullopt;
        if (std::all_of(tensor[required].begin(), tensor[required].end(),
                        [](const auto value) { return value == 0U; }))
            return std::nullopt;
        for (std::uint64_t degree = 0U; degree < required; ++degree) {
            if (std::any_of(tensor[degree].begin(), tensor[degree].end(),
                            [](const auto value) { return value != 0U; }))
                return std::nullopt;
        }

        if (ledger) {
            ++ledger->reference_germ_materializations;
            ledger->dense_reference_coefficients +=
                static_cast<std::uint64_t>(tensor.size()) * state.order();
            ledger->dense_reference_updates +=
                parent_ledger.jet_cyclic_coefficient_updates;
        }

        ObservationClosureCertificate certificate{};
        certificate.observer = contract.observer;
        certificate.language = contract.language;
        certificate.language_version = contract.language_version;
        certificate.order = state.order();
        certificate.cycles = state.cycles();
        certificate.required_jet_horizon = required;
        certificate.available_jet_horizon = state.available_jet_order();
        certificate.source_session = state.session();
        certificate.mode = ClosureMode::ReferenceOnly;
        certificate.payload_already_bound = true;
        certificate.no_deferred_execute = true;
        certificate.scoped_not_global_future_equivalence = true;
        certificate.accepted = true;
        return ObservationClosedState{
            state,
            PrimitiveClosureObservation{state.order(), required,
                                        tensor[required], audit_modulus},
            certificate};
    }

    [[nodiscard]] static DownloadPacket download(
        const ObservationClosedState& closed,
        BoundaryLedger* ledger = nullptr) {
        const auto& certificate = closed.certificate();
        const auto& state = closed.state();
        if (!certificate.accepted || !certificate.payload_already_bound ||
            !certificate.no_deferred_execute ||
            !certificate.scoped_not_global_future_equivalence ||
            certificate.order != state.order() ||
            certificate.cycles != state.cycles() ||
            certificate.source_session != state.session())
            throw std::invalid_argument("R63 rejected invalid observation closure");
        if (ledger) ++ledger->downloads;

        CausalCutReceipt cut{};
        cut.source_session = state.session();
        cut.source_origin_kind = state.origin_kind();
        cut.observer = certificate.observer;
        cut.language = certificate.language;
        cut.discarded_cycle_multiplicity = state.cycles();
        cut.discarded_continuation_epoch = state.continuation_epoch();
        cut.discarded_singular_residual_generation =
            state.singular_residual_generation();
        cut.no_resume_capability = true;
        cut.fresh_upload_required_for_reentry = true;
        return DownloadPacket{closed.bound_observation(), cut};
    }

    [[nodiscard]] static std::optional<CyclicBoundaryState> reupload_as_fresh_origin(
        const OrdinaryOrderObservation& observation,
        SessionAuthority& authority,
        BoundaryLedger* ledger = nullptr) {
        return upload(SpecificationGate::encode_canonical(observation.order),
                      authority, ledger);
    }

    [[nodiscard]] static RamifiedCompatibilityWitness
    reference_verify_ramified_compatibility(
        const std::uint64_t source_order,
        const std::uint64_t target_order,
        const std::uint64_t audit_modulus = 998244353U,
        BoundaryLedger* ledger = nullptr) {
        RamifiedCompatibilityWitness witness{};
        witness.source_order = source_order;
        witness.target_order = target_order;
        if (target_order < 2U || source_order < target_order ||
            source_order % target_order != 0U)
            return witness;
        witness.ramification_index = source_order / target_order;
        const auto horizon = witness.ramification_index;

        angel::afac62::SelfClosureGermLedger large_ledger{};
        angel::afac62::SelfClosureGermLedger repeated_ledger{};
        const auto large = angel::afac62::materialize_self_closure_germ(
            source_order, 1U, horizon, audit_modulus, &large_ledger);
        const auto pushed = angel::afac62::quotient_pushforward_germ(
            large, target_order, audit_modulus);
        const auto repeated = angel::afac62::materialize_self_closure_germ(
            target_order, witness.ramification_index, horizon,
            audit_modulus, &repeated_ledger);
        witness.exact_quotient_transport = pushed == repeated;

        const auto terminal =
            angel::afac61::materialize_unnormalized_self_cyclic_column(
                source_order, audit_modulus);
        const auto pushed_terminal = angel::afac62::quotient_pushforward_column(
            terminal, target_order, audit_modulus);
        witness.terminal_projector_collapses = std::all_of(
            pushed_terminal.begin(), pushed_terminal.end(),
            [](const auto value) { return value == 0U; });

        witness.primitive_content_first_visible_at_ramification = true;
        for (std::uint64_t degree = 0U; degree < horizon; ++degree) {
            if (std::any_of(pushed[degree].begin(), pushed[degree].end(),
                            [](const auto value) { return value != 0U; })) {
                witness.primitive_content_first_visible_at_ramification = false;
                break;
            }
        }
        if (horizon >= pushed.size() ||
            std::all_of(pushed[horizon].begin(), pushed[horizon].end(),
                        [](const auto value) { return value == 0U; }))
            witness.primitive_content_first_visible_at_ramification = false;

        if (ledger) {
            ++ledger->reference_germ_materializations;
            ledger->dense_reference_coefficients +=
                static_cast<std::uint64_t>(large.size()) * source_order +
                static_cast<std::uint64_t>(repeated.size()) * target_order;
            ledger->dense_reference_updates +=
                large_ledger.jet_cyclic_coefficient_updates +
                repeated_ledger.jet_cyclic_coefficient_updates;
        }
        witness.accepted = witness.exact_quotient_transport &&
            witness.terminal_projector_collapses &&
            witness.primitive_content_first_visible_at_ramification;
        return witness;
    }
};

class BoundaryAuditView final {
public:
    [[nodiscard]] static bool exact_state_equal(
        const CyclicBoundaryState& left,
        const CyclicBoundaryState& right) noexcept {
        return same_scoped_semantic_state(left, right) &&
               left.presentation() == right.presentation() &&
               left.origin_kind() == right.origin_kind();
    }
};

// Explicitly forbidden boundary shortcuts.
CyclicBoundaryState restore_native_state_from_ordinary_observation(
    const OrdinaryBoundaryObservation&) = delete;
CyclicBoundaryState restore_native_state_from_cut_receipt(
    const CausalCutReceipt&) = delete;
DownloadPacket download_unclosed_state(const CyclicBoundaryState&) = delete;
OrdinaryBoundaryObservation treat_native_capsule_as_ordinary(
    const NativeContinuationCapsule&) = delete;
CyclicBoundaryState feed_ordinary_observation_into_native_execute(
    const OrdinaryBoundaryObservation&) = delete;

static_assert(!std::is_default_constructible_v<CyclicBoundaryState>);
static_assert(!std::is_default_constructible_v<CertifiedCyclicOrderSpec>);
static_assert(!std::is_default_constructible_v<ObservationClosedState>);
static_assert(!std::is_convertible_v<OrdinaryBoundaryObservation,
                                     CyclicBoundaryState>);
static_assert(!std::is_convertible_v<CausalCutReceipt,
                                     CyclicBoundaryState>);
static_assert(!std::is_convertible_v<NativeContinuationCapsule,
                                     OrdinaryBoundaryObservation>);

} // namespace angel::afac63
