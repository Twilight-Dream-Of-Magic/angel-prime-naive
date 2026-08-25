#pragma once

#include "angel/boundary/observation.hpp"
#include "angel/native_factorial.hpp"
#include "angel/prime/cyclic_action.hpp"
#include "angel/prime/quotient.hpp"
#include "angel/prime/state.hpp"
#include "internal/frozen_types.hpp"
#include "internal/big_unsigned.hpp"

#include <memory>
#include <utility>

namespace angel::detail {

struct FactorialStateModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;

    explicit FactorialStateModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input)
        : request(std::move(input)) {}
};

struct QuotientViewModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;
    frozen::quotient_view::CertifiedExternalQuotientView view;

    QuotientViewModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input_request,
        frozen::quotient_view::CertifiedExternalQuotientView input_view)
        : request(std::move(input_request)), view(std::move(input_view)) {}
};

struct CyclicActionViewModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;
    frozen::cyclic_action::CertifiedSelfCyclicActionView view;

    CyclicActionViewModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input_request,
        frozen::cyclic_action::CertifiedSelfCyclicActionView input_view)
        : request(std::move(input_request)), view(std::move(input_view)) {}
};

struct ClosedCyclicActionModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;
    frozen::cyclic_action::CertifiedSelfCyclicActionView view;
    frozen::cyclic_action::SelfCyclicProbeDownload download;
    prime::CyclicActionPolicy policy;

    ClosedCyclicActionModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input_request,
        frozen::cyclic_action::CertifiedSelfCyclicActionView input_view,
        frozen::cyclic_action::SelfCyclicProbeDownload input_download,
        const prime::CyclicActionPolicy input_policy)
        : request(std::move(input_request)), view(std::move(input_view)),
          download(std::move(input_download)), policy(input_policy) {}
};

struct NativeFactorialViewModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;
    std::uint64_t view_seal{};

    NativeFactorialViewModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input_request,
        const std::uint64_t input_view_seal)
        : request(std::move(input_request)), view_seal(input_view_seal) {}
};

struct ExactFactorialValueModel final {
    frozen::factorial_boundary::CertifiedWilsonRequest request;
    std::uint64_t native_view_seal{};
    BigUnsigned value;
    BigUnsignedLedger sequential_ledger;
    BigUnsignedLedger product_tree_ledger;
    std::uint64_t exact_value_seal{};
    bool independent_equal{};

    ExactFactorialValueModel(
        frozen::factorial_boundary::CertifiedWilsonRequest input_request,
        const std::uint64_t input_view_seal,
        BigUnsigned input_value,
        const BigUnsignedLedger input_sequential_ledger,
        const BigUnsignedLedger input_product_tree_ledger,
        const std::uint64_t input_exact_value_seal,
        const bool input_independent_equal)
        : request(std::move(input_request)),
          native_view_seal(input_view_seal), value(std::move(input_value)),
          sequential_ledger(input_sequential_ledger),
          product_tree_ledger(input_product_tree_ledger),
          exact_value_seal(input_exact_value_seal),
          independent_equal(input_independent_equal) {}
};

struct PrimeAccess final {
    [[nodiscard]] static prime::FactorialState make_factorial(
        frozen::factorial_boundary::CertifiedWilsonRequest request) {
        return prime::FactorialState{
            std::make_shared<FactorialStateModel>(std::move(request))};
    }

    [[nodiscard]] static const FactorialStateModel& model(
        const prime::FactorialState& state) noexcept {
        return *state.model_;
    }

    [[nodiscard]] static prime::QuotientView make_view(
        frozen::factorial_boundary::CertifiedWilsonRequest request,
        frozen::quotient_view::CertifiedExternalQuotientView view) {
        return prime::QuotientView{std::make_shared<QuotientViewModel>(
            std::move(request), std::move(view))};
    }

    [[nodiscard]] static const QuotientViewModel& model(
        const prime::QuotientView& view) noexcept {
        return *view.model_;
    }
};

struct CyclicStructureAccess final {
    [[nodiscard]] static prime::CyclicActionView make_view(
        frozen::factorial_boundary::CertifiedWilsonRequest request,
        frozen::cyclic_action::CertifiedSelfCyclicActionView view) {
        return prime::CyclicActionView{
            std::make_shared<CyclicActionViewModel>(
                std::move(request), std::move(view))};
    }

    [[nodiscard]] static const CyclicActionViewModel& model(
        const prime::CyclicActionView& view) noexcept {
        return *view.model_;
    }

    [[nodiscard]] static prime::ClosedCyclicAction make_closed(
        frozen::factorial_boundary::CertifiedWilsonRequest request,
        frozen::cyclic_action::CertifiedSelfCyclicActionView view,
        frozen::cyclic_action::SelfCyclicProbeDownload download,
        const prime::CyclicActionPolicy policy) {
        return prime::ClosedCyclicAction{
            std::make_shared<ClosedCyclicActionModel>(
                std::move(request), std::move(view), std::move(download),
                policy)};
    }

    [[nodiscard]] static const ClosedCyclicActionModel& model(
        const prime::ClosedCyclicAction& closed) noexcept {
        return *closed.model_;
    }
};

struct NativeFactorialAccess final {
    [[nodiscard]] static prime::NativeFactorialView make_view(
        frozen::factorial_boundary::CertifiedWilsonRequest request,
        const std::uint64_t view_seal) {
        return prime::NativeFactorialView{
            std::make_shared<NativeFactorialViewModel>(
                std::move(request), view_seal)};
    }

    [[nodiscard]] static const NativeFactorialViewModel& model(
        const prime::NativeFactorialView& view) noexcept {
        return *view.model_;
    }

    [[nodiscard]] static prime::ExactFactorialValue make_exact(
        frozen::factorial_boundary::CertifiedWilsonRequest request,
        const std::uint64_t native_view_seal,
        BigUnsigned value,
        const BigUnsignedLedger sequential_ledger,
        const BigUnsignedLedger product_tree_ledger,
        const std::uint64_t exact_value_seal,
        const bool independent_equal) {
        return prime::ExactFactorialValue{
            std::make_shared<ExactFactorialValueModel>(
                std::move(request), native_view_seal, std::move(value),
                sequential_ledger, product_tree_ledger, exact_value_seal,
                independent_equal)};
    }

    [[nodiscard]] static const ExactFactorialValueModel& model(
        const prime::ExactFactorialValue& value) noexcept {
        return *value.model_;
    }
};

struct AuthorityModel final {
    frozen::cyclic_boundary::SessionAuthority authority;

    explicit AuthorityModel(const std::uint64_t value) : authority(value) {}
};

struct BoundaryStateModel final {
    frozen::cyclic_boundary::CyclicBoundaryState state;

    explicit BoundaryStateModel(
        frozen::cyclic_boundary::CyclicBoundaryState input)
        : state(std::move(input)) {}
};

struct CheckpointModel final {
    frozen::cyclic_boundary::NativeContinuationCapsule checkpoint;

    explicit CheckpointModel(
        frozen::cyclic_boundary::NativeContinuationCapsule input)
        : checkpoint(std::move(input)) {}
};

struct ClosedObservationModel final {
    frozen::cyclic_boundary::ObservationClosedState closed;

    explicit ClosedObservationModel(
        frozen::cyclic_boundary::ObservationClosedState input)
        : closed(std::move(input)) {}
};

struct BoundaryAccess final {
    [[nodiscard]] static AuthorityModel& model(
        boundary::SessionAuthority& authority) noexcept {
        return *authority.model_;
    }

    [[nodiscard]] static boundary::State make_state(
        frozen::cyclic_boundary::CyclicBoundaryState state) {
        return boundary::State{
            std::make_shared<BoundaryStateModel>(std::move(state))};
    }

    [[nodiscard]] static const BoundaryStateModel& model(
        const boundary::State& state) noexcept {
        return *state.model_;
    }

    [[nodiscard]] static boundary::NativeCheckpoint make_checkpoint(
        frozen::cyclic_boundary::NativeContinuationCapsule checkpoint) {
        return boundary::NativeCheckpoint{
            std::make_shared<CheckpointModel>(std::move(checkpoint))};
    }

    [[nodiscard]] static const CheckpointModel& model(
        const boundary::NativeCheckpoint& checkpoint) noexcept {
        return *checkpoint.model_;
    }

    [[nodiscard]] static boundary::ClosedObservation make_closed(
        frozen::cyclic_boundary::ObservationClosedState closed) {
        return boundary::ClosedObservation{
            std::make_shared<ClosedObservationModel>(std::move(closed))};
    }

    [[nodiscard]] static const ClosedObservationModel& model(
        const boundary::ClosedObservation& closed) noexcept {
        return *closed.model_;
    }
};

} // namespace angel::detail
