#pragma once

#include "angel/boundary.hpp"
#include "angel/prime.hpp"
#include "internal/models.hpp"

namespace angel::detail {

[[nodiscard]] inline frozen::cyclic_boundary::EncodedCyclicOrderSpec
to_frozen(const boundary::EncodedOrder& encoded) noexcept {
    frozen::cyclic_boundary::EncodedCyclicOrderSpec out{};
    out.bytes = encoded.bytes;
    out.byte_count = encoded.byte_count;
    out.byte_order = encoded.byte_order == boundary::ByteOrder::LittleEndian
        ? frozen::cyclic_boundary::ByteOrder::LittleEndian
        : frozen::cyclic_boundary::ByteOrder::BigEndian;
    return out;
}

[[nodiscard]] inline frozen::cyclic_boundary::BoundaryLedger to_frozen(
    const boundary::BoundaryLedger& ledger) noexcept {
    frozen::cyclic_boundary::BoundaryLedger out{};
    out.representation_normalizations = ledger.representation_normalizations;
    out.fresh_origins_created = ledger.fresh_origins_created;
    out.presentation_transports = ledger.presentation_transports;
    out.native_quotients = ledger.native_quotients;
    out.native_same_frame_continuations =
        ledger.native_same_frame_continuations;
    out.native_exports = ledger.native_exports;
    out.native_imports = ledger.native_imports;
    out.closure_checks = ledger.closure_checks;
    out.reference_germ_materializations = ledger.reference_germ_materializations;
    out.dense_reference_coefficients = ledger.dense_reference_coefficients;
    out.dense_reference_updates = ledger.dense_reference_updates;
    out.downloads = ledger.downloads;
    out.angel_nodes_rewritten = ledger.nodes_rewritten;
    out.angel_nodes_merged = ledger.nodes_merged;
    out.ordinary_observation_fed_back_to_native = ledger.ordinary_feedback;
    return out;
}

inline void from_frozen(
    const frozen::cyclic_boundary::BoundaryLedger& input,
    boundary::BoundaryLedger& ledger) noexcept {
    ledger.representation_normalizations = input.representation_normalizations;
    ledger.fresh_origins_created = input.fresh_origins_created;
    ledger.presentation_transports = input.presentation_transports;
    ledger.native_quotients = input.native_quotients;
    ledger.native_same_frame_continuations =
        input.native_same_frame_continuations;
    ledger.native_exports = input.native_exports;
    ledger.native_imports = input.native_imports;
    ledger.closure_checks = input.closure_checks;
    ledger.reference_germ_materializations = input.reference_germ_materializations;
    ledger.dense_reference_coefficients = input.dense_reference_coefficients;
    ledger.dense_reference_updates = input.dense_reference_updates;
    ledger.downloads = input.downloads;
    ledger.nodes_rewritten = input.angel_nodes_rewritten;
    ledger.nodes_merged = input.angel_nodes_merged;
    ledger.ordinary_feedback = input.ordinary_observation_fed_back_to_native;
}

class LedgerBridge final {
public:
    explicit LedgerBridge(boundary::BoundaryLedger* public_ledger) noexcept
        : public_(public_ledger), frozen_(public_ledger
              ? to_frozen(*public_ledger)
              : frozen::cyclic_boundary::BoundaryLedger{}) {}

    ~LedgerBridge() {
        if (public_) from_frozen(frozen_, *public_);
    }

    [[nodiscard]] frozen::cyclic_boundary::BoundaryLedger* pointer() noexcept {
        return public_ ? &frozen_ : nullptr;
    }

private:
    boundary::BoundaryLedger* public_{};
    frozen::cyclic_boundary::BoundaryLedger frozen_{};
};

[[nodiscard]] inline boundary::SessionId from_frozen(
    const frozen::cyclic_boundary::SessionId session) noexcept {
    return boundary::SessionId{session.authority, session.sequence};
}

[[nodiscard]] inline boundary::Origin from_frozen(
    const frozen::cyclic_boundary::OriginKind origin) noexcept {
    switch (origin) {
        case frozen::cyclic_boundary::OriginKind::ExternalFreshUpload:
            return boundary::Origin::FreshUpload;
        case frozen::cyclic_boundary::OriginKind::NativeRamifiedQuotient:
            return boundary::Origin::RamifiedQuotient;
        case frozen::cyclic_boundary::OriginKind::NativeContinuationImport:
            return boundary::Origin::CheckpointImport;
        case frozen::cyclic_boundary::OriginKind::NativeSameFrameContinuation:
            return boundary::Origin::SameFrameContinuation;
    }
    return boundary::Origin::FreshUpload;
}

[[nodiscard]] inline boundary::Presentation from_frozen(
    const frozen::cyclic_boundary::CyclicPresentation& value) noexcept {
    return boundary::Presentation{
        value.order, value.exponent, value.inverse_exponent};
}

[[nodiscard]] inline frozen::cyclic_boundary::CyclicPresentation to_frozen(
    const boundary::Presentation& value) noexcept {
    return frozen::cyclic_boundary::CyclicPresentation{
        value.order, value.exponent, value.inverse_exponent};
}

[[nodiscard]] inline boundary::Observer from_frozen(
    const frozen::cyclic_boundary::ObserverKind observer) noexcept {
    return observer == frozen::cyclic_boundary::ObserverKind::OrderOnlyV1
        ? boundary::Observer::Order
        : boundary::Observer::PrimitiveClosureJet;
}

[[nodiscard]] inline boundary::ContinuationLanguage from_frozen(
    const frozen::cyclic_boundary::ContinuationLanguage language) noexcept {
    return language ==
            frozen::cyclic_boundary::ContinuationLanguage::FrozenTerminalV1
        ? boundary::ContinuationLanguage::FrozenTerminal
        : boundary::ContinuationLanguage::SameFrameGerm;
}

[[nodiscard]] inline frozen::cyclic_boundary::ContinuationLanguage to_frozen(
    const boundary::ContinuationLanguage language) noexcept {
    return language == boundary::ContinuationLanguage::FrozenTerminal
        ? frozen::cyclic_boundary::ContinuationLanguage::FrozenTerminalV1
        : frozen::cyclic_boundary::ContinuationLanguage::SameFrameGermContinuationV1;
}

[[nodiscard]] inline boundary::ClosureMode from_frozen(
    const frozen::cyclic_boundary::ClosureMode mode) noexcept {
    return mode == frozen::cyclic_boundary::ClosureMode::NativeScoped
        ? boundary::ClosureMode::NativeScoped
        : boundary::ClosureMode::ReferenceOnly;
}

[[nodiscard]] inline prime::CoordinateSummary from_frozen(
    const frozen::quotient_view::ExternalCoordinateDimension& coordinate) noexcept {
    return prime::CoordinateSummary{
        coordinate.factor_count,
        coordinate.block_width,
        coordinate.full_blocks,
        coordinate.tail_factors,
        coordinate.polynomial_coefficients,
        coordinate.evaluation_values,
        coordinate.total_scalar_slots,
        coordinate.natural_sqrt_coordinate,
        coordinate.polylogarithmic_summary_claimed,
        coordinate.tombstoned_as_polylog_candidate};
}

[[nodiscard]] inline prime::ObservationLedger from_frozen(
    const frozen::wilson::SublinearWilsonLedger& ledger) noexcept {
    return prime::ObservationLedger{
        ledger.angel_input_bits,
        ledger.angel_native_steps,
        ledger.angel_state_payload_bytes,
        ledger.target_factor_count,
        ledger.block_width,
        ledger.full_blocks,
        ledger.tail_factors,
        ledger.factor_leaf_materializations,
        ledger.evaluation_points,
        ledger.block_value_multiplications,
        ledger.tail_multiplications,
        ledger.ordinary_elapsed_nanoseconds,
        ledger.polynomial.polynomial_multiplications,
        ledger.polynomial.maximum_polynomial_coefficients,
        ledger.polynomial.maximum_ntt_length,
        ledger.ordinary_projection_started,
        ledger.ordinary_projection_completed,
        ledger.ordinary_result_fed_back_to_angel};
}

[[nodiscard]] inline frozen::wilson::SublinearWilsonPolicy to_frozen(
    const prime::ObservationPolicy& policy) noexcept {
    auto out = frozen::wilson::SublinearWilsonPolicy::production(
        policy.maximum_block_width);
    out.block_width_override = policy.block_width_override;
    out.parallel_tree_branches = policy.parallel_tree_branches;
    out.parallel_tree_threshold = policy.parallel_tree_threshold;
    return out;
}

} // namespace angel::detail
