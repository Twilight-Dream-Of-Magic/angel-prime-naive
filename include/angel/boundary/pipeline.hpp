#pragma once

#include "angel/boundary/encoding.hpp"
#include "angel/boundary/evidence.hpp"
#include "angel/boundary/observation.hpp"

#include <cstdint>

namespace angel::boundary {

struct UploadOperation final {
    SessionAuthority* authority{};
    BoundaryLedger* ledger{};
};

struct ChangePresentationOperation final {
    Presentation target{};
    PresentationEvidence* evidence{};
    BoundaryLedger* ledger{};
};

struct QuotientOperation final {
    std::uint64_t target_order{};
    BoundaryLedger* ledger{};
};

struct ContinueOperation final {
    std::uint64_t available_jet_order{};
    BoundaryLedger* ledger{};
};

struct ExportCheckpointOperation final {
    BoundaryLedger* ledger{};
};

struct ImportCheckpointOperation final {
    BoundaryLedger* ledger{};
};

struct ObserveOrderOperation final {
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminal};
    std::uint64_t language_version{1U};
    BoundaryLedger* ledger{};
};

struct ObservePrimitiveOperation final {
    std::uint64_t declared_jet_horizon{};
    std::uint64_t audit_modulus{998244353U};
    std::uint64_t language_version{1U};
    BoundaryLedger* ledger{};
};

struct DownloadOperation final {
    BoundaryLedger* ledger{};
};

[[nodiscard]] inline UploadOperation upload(
    SessionAuthority& authority, BoundaryLedger* ledger = nullptr) noexcept {
    return UploadOperation{&authority, ledger};
}

[[nodiscard]] inline ChangePresentationOperation change_presentation(
    Presentation target,
    PresentationEvidence* evidence = nullptr,
    BoundaryLedger* ledger = nullptr) noexcept {
    return ChangePresentationOperation{target, evidence, ledger};
}

[[nodiscard]] inline QuotientOperation quotient_to(
    const std::uint64_t target_order,
    BoundaryLedger* ledger = nullptr) noexcept {
    return QuotientOperation{target_order, ledger};
}

[[nodiscard]] inline ContinueOperation continue_to(
    const std::uint64_t available_jet_order,
    BoundaryLedger* ledger = nullptr) noexcept {
    return ContinueOperation{available_jet_order, ledger};
}

[[nodiscard]] inline ExportCheckpointOperation export_checkpoint(
    BoundaryLedger* ledger = nullptr) noexcept {
    return ExportCheckpointOperation{ledger};
}

[[nodiscard]] inline ImportCheckpointOperation import_checkpoint(
    BoundaryLedger* ledger = nullptr) noexcept {
    return ImportCheckpointOperation{ledger};
}

[[nodiscard]] inline ObserveOrderOperation observe_order(
    const ContinuationLanguage language = ContinuationLanguage::FrozenTerminal,
    const std::uint64_t language_version = 1U,
    BoundaryLedger* ledger = nullptr) noexcept {
    return ObserveOrderOperation{language, language_version, ledger};
}

[[nodiscard]] inline ObservePrimitiveOperation observe_primitive(
    const std::uint64_t declared_jet_horizon,
    const std::uint64_t audit_modulus = 998244353U,
    const std::uint64_t language_version = 1U,
    BoundaryLedger* ledger = nullptr) noexcept {
    return ObservePrimitiveOperation{
        declared_jet_horizon, audit_modulus, language_version, ledger};
}

[[nodiscard]] inline DownloadOperation download(
    BoundaryLedger* ledger = nullptr) noexcept {
    return DownloadOperation{ledger};
}

[[nodiscard]] State operator|(const EncodedOrder& input, UploadOperation operation);
[[nodiscard]] State operator|(const State& state, ChangePresentationOperation operation);
[[nodiscard]] State operator|(const State& state, QuotientOperation operation);
[[nodiscard]] State operator|(const State& state, ContinueOperation operation);
[[nodiscard]] NativeCheckpoint operator|(
    const State& state, ExportCheckpointOperation operation);
[[nodiscard]] State operator|(
    const NativeCheckpoint& checkpoint, ImportCheckpointOperation operation);
[[nodiscard]] ClosedObservation operator|(
    const State& state, ObserveOrderOperation operation);
[[nodiscard]] ClosedObservation operator|(
    const State& state, ObservePrimitiveOperation operation);
[[nodiscard]] DownloadPacket operator|(
    const ClosedObservation& closed, DownloadOperation operation);

} // namespace angel::boundary
