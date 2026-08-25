#pragma once

#include "angel/prime/state.hpp"
#include "angel/state_integrity.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace angel::detail {
struct CyclicActionViewModel;
struct ClosedCyclicActionModel;
struct CyclicStructureAccess;
}

namespace angel::prime {

enum class PeriodStructure : std::uint8_t {
    PrimitiveOnly = 1,
    ProperPeriodKernel = 2
};

struct CyclicActionPolicy final {
    std::uint64_t maximum_order{4096U};
    std::uint64_t audit_modulus{1'000'000'007U};
    bool include_normalized_action{};
    bool compute_kernel_dimension{};
};

struct CyclicActionViewSummary final {
    std::uint64_t order{};
    std::uint64_t represented_factor_count{};
    std::uint64_t request_binding{};
    std::uint64_t state_seal{};
    std::uint64_t program_seal{};
    std::uint64_t valuation_hash{};
    std::uint64_t view_seal{};
    std::uint64_t descriptor_words{};
    bool contains_arithmetic_state{};
    bool compresses_arithmetic_state{};
    bool can_feed_back_to_arithmetic{};
};

class CyclicActionView final {
public:
    CyclicActionView() = delete;
    CyclicActionView(const CyclicActionView&) noexcept = default;
    CyclicActionView(CyclicActionView&&) noexcept = default;
    CyclicActionView& operator=(const CyclicActionView&) noexcept = default;
    CyclicActionView& operator=(CyclicActionView&&) noexcept = default;
    ~CyclicActionView() = default;

    [[nodiscard]] CyclicActionViewSummary summary() const noexcept;
    [[nodiscard]] bool preserves_complete_state_identity() const noexcept;

private:
    explicit CyclicActionView(
        std::shared_ptr<const detail::CyclicActionViewModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::CyclicActionViewModel> model_;
    friend struct detail::CyclicStructureAccess;
};

struct CyclicActionLedger final {
    std::uint64_t factors_applied{};
    std::uint64_t cyclic_coefficient_updates{};
    std::uint64_t normalized_window_updates{};
    std::uint64_t polynomial_division_steps{};
    std::uint64_t independent_number_theory_steps{};
    std::uint64_t independent_relation_updates{};
    std::uint64_t maximum_live_coefficients{};
    std::uint64_t nodes_rewritten{};
    std::uint64_t nodes_merged{};
    bool ordinary_feedback{};
};

struct CyclicObservationCertificate final {
    std::uint64_t order{};
    std::uint64_t view_seal{};
    std::uint64_t response_seal{};
    std::uint64_t maximum_order{};
    bool observation_closed{};
    bool resource_limited{};
    bool no_resume_capability{};
    bool no_feedback_channel{};
};

class ClosedCyclicAction final {
public:
    ClosedCyclicAction() = delete;
    ClosedCyclicAction(const ClosedCyclicAction&) noexcept = default;
    ClosedCyclicAction(ClosedCyclicAction&&) noexcept = default;
    ClosedCyclicAction& operator=(const ClosedCyclicAction&) noexcept = default;
    ClosedCyclicAction& operator=(ClosedCyclicAction&&) noexcept = default;
    ~ClosedCyclicAction() = default;

    [[nodiscard]] CyclicObservationCertificate certificate() const noexcept;

private:
    explicit ClosedCyclicAction(
        std::shared_ptr<const detail::ClosedCyclicActionModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::ClosedCyclicActionModel> model_;
    friend struct detail::CyclicStructureAccess;
};

struct CyclicStructureObservation final {
    std::uint64_t order{};
    std::uint64_t audit_modulus{};
    std::vector<std::uint64_t> primitive_period_response{};
    std::vector<std::uint64_t> normalized_action_response{};
    std::optional<std::uint64_t> proper_period_kernel_dimension{};
    PeriodStructure structure{PeriodStructure::ProperPeriodKernel};
};

struct CyclicStructureResourceLimit final {
    std::uint64_t requested_order{};
    std::uint64_t maximum_order{};
};

using CyclicOrdinaryObservation =
    std::variant<CyclicStructureObservation, CyclicStructureResourceLimit>;

struct CyclicStructureVerification final {
    bool frozen_binding_valid{};
    bool response_integrity_valid{};
    bool dimensions_valid{};
    bool response_seal_valid{};
    bool response_materialized{};
    bool ramanujan_replay_checked{};
    bool ramanujan_replay_valid{};
    bool normalized_action_checked{};
    bool valuation_reattachment_valid{};
    bool constant_mode_factorial_valid{};
    bool kernel_dimension_checked{};
    bool kernel_dimension_valid{};
    bool period_classification_valid{};
    bool no_feedback{};
    bool accepted{};
};

struct CyclicStructureDownload final {
    CyclicOrdinaryObservation observation{};
    CyclicActionLedger ledger{};
    StateIntegrity integrity{};
    CyclicObservationCertificate certificate{};
    CyclicStructureVerification verification{};

    [[nodiscard]] bool verified() const noexcept {
        return verification.accepted && integrity.preserved();
    }
};

} // namespace angel::prime
