#pragma once

#include "angel/boundary/state.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace angel::detail {
struct ClosedObservationModel;
struct BoundaryAccess;
}

namespace angel::boundary {

enum class Observer : std::uint8_t {
    Order = 1,
    PrimitiveClosureJet = 2
};

enum class ContinuationLanguage : std::uint8_t {
    FrozenTerminal = 1,
    SameFrameGerm = 2
};

enum class ClosureMode : std::uint8_t {
    NativeScoped = 1,
    ReferenceOnly = 2
};

struct OrderObservation final {
    std::uint64_t order{};
    std::uint64_t observer_version{1U};
    friend bool operator==(const OrderObservation&, const OrderObservation&) = default;
};

struct PrimitiveObservation final {
    std::uint64_t order{};
    std::uint64_t first_visible_jet{};
    std::vector<std::uint64_t> primitive_column{};
    std::uint64_t audit_modulus{};
    friend bool operator==(const PrimitiveObservation&, const PrimitiveObservation&) = default;
};

using OrdinaryObservation = std::variant<OrderObservation, PrimitiveObservation>;

struct ClosureCertificate final {
    Observer observer{Observer::Order};
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminal};
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
};

class ClosedObservation final {
public:
    ClosedObservation() = delete;
    ClosedObservation(const ClosedObservation&) noexcept = default;
    ClosedObservation(ClosedObservation&&) noexcept = default;
    ClosedObservation& operator=(const ClosedObservation&) noexcept = default;
    ClosedObservation& operator=(ClosedObservation&&) noexcept = default;
    ~ClosedObservation() = default;

    [[nodiscard]] ClosureCertificate certificate() const;

private:
    explicit ClosedObservation(
        std::shared_ptr<const detail::ClosedObservationModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::ClosedObservationModel> model_;
    friend struct detail::BoundaryAccess;
};

struct CausalCut final {
    SessionId source_session{};
    Origin source_origin{Origin::FreshUpload};
    Observer observer{Observer::Order};
    ContinuationLanguage language{ContinuationLanguage::FrozenTerminal};
    std::uint64_t discarded_cycle_multiplicity{};
    std::uint64_t discarded_continuation_epoch{};
    std::uint64_t discarded_residual_generation{};
    bool no_resume_capability{};
    bool fresh_upload_required_for_reentry{};
};

struct DownloadPacket final {
    OrdinaryObservation observation{};
    CausalCut cut{};
};

} // namespace angel::boundary
