#pragma once

#include "angel/native_factorial.hpp"
#include "internal/frozen_types.hpp"

namespace angel::detail {

[[nodiscard]] prime::NativeWilsonDownload execute_wilson_from_native_coordinate(
    const frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const prime::ObservationPolicy& policy);

} // namespace angel::detail
