#include "angel/diagnostics.hpp"
#include "angel/boundary/state.hpp"
#include "angel/prime/state.hpp"
#include "internal/models.hpp"

namespace angel::diagnostics {

FrozenLayout frozen_layout() noexcept {
    return FrozenLayout{
        sizeof(detail::frozen::arithmetic::CertifiedPrincipalJetState),
        sizeof(detail::frozen::cyclic_boundary::CyclicBoundaryState),
        sizeof(prime::FactorialState),
        sizeof(boundary::State),
        true,
        true,
        false};
}

} // namespace angel::diagnostics
