#include "angel/prime/pipeline.hpp"
#include "internal/conversions.hpp"
#include "internal/native_factorial_runtime.hpp"

#include <stdexcept>
#include <utility>

namespace angel::prime {

FactorialState operator|(
    const Candidate input, const UploadFactorialState) {
    detail::frozen::factorial_boundary::WilsonBoundary boundary;
    return detail::PrimeAccess::make_factorial(boundary.bind(input.value));
}

QuotientView operator|(
    const FactorialState& state, const BindQuotientView) {
    const auto& request = detail::PrimeAccess::model(state).request;
    detail::frozen::quotient_view::ExternalQuotientBoundary boundary;
    auto view = boundary.bind(request);
    return detail::PrimeAccess::make_view(request, std::move(view));
}

Download operator|(
    const QuotientView& public_view,
    const DownloadWilsonObservation operation) {
    const auto& model = detail::PrimeAccess::model(public_view);
    const auto view_verification = detail::frozen::quotient_view::
        ExternalQuotientViewVerifier::verify(model.request, model.view);
    if (!view_verification.accepted)
        throw std::invalid_argument("external quotient view binding rejected");
    auto native = detail::execute_wilson_from_native_coordinate(
        model.request, operation.policy);
    return Download{
        std::move(native.observation),
        native.coordinate,
        native.integrity,
        native.verification_passed};
}

FactorialStateSummary FactorialState::summary() const noexcept {
    const auto& request = detail::PrimeAccess::model(*this).request;
    const auto& execution = request.factorial_execution();
    const auto& state = execution.principal_jet;
    return FactorialStateSummary{
        request.candidate(),
        request.candidate() - 1U,
        request.binding_seal(),
        state.seal(),
        state.source_program().program().seal(),
        state.valuation().bit_length(),
        execution.r56_factorial.ledger.total_steps() +
            execution.fusion_ledger.total_native_steps(),
        state.payload_bytes()};
}

std::uint64_t QuotientView::candidate() const noexcept {
    return detail::PrimeAccess::model(*this).view.candidate();
}

CoordinateSummary QuotientView::coordinate() const noexcept {
    return detail::from_frozen(
        detail::PrimeAccess::model(*this).view.coordinate());
}

bool QuotientView::preserves_complete_state_identity() const noexcept {
    const auto& model = detail::PrimeAccess::model(*this);
    return detail::frozen::quotient_view::ExternalQuotientViewVerifier::verify(
        model.request, model.view).accepted;
}

} // namespace angel::prime
