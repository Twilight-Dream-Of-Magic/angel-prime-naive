#pragma once

#include "angel/prime/input.hpp"
#include "angel/prime/observation.hpp"
#include "angel/prime/state.hpp"

namespace angel::prime {

struct UploadFactorialState final {};
struct BindQuotientView final {};
struct DownloadWilsonObservation final {
    ObservationPolicy policy{};
};

[[nodiscard]] constexpr UploadFactorialState upload_factorial_state() noexcept {
    return {};
}

[[nodiscard]] constexpr BindQuotientView bind_quotient_view() noexcept {
    return {};
}

[[nodiscard]] constexpr DownloadWilsonObservation download_wilson(
    const ObservationPolicy policy = {}) noexcept {
    return DownloadWilsonObservation{policy};
}

[[nodiscard]] FactorialState operator|(
    Candidate input, UploadFactorialState operation);
[[nodiscard]] QuotientView operator|(
    const FactorialState& state, BindQuotientView operation);
[[nodiscard]] Download operator|(
    const QuotientView& view, DownloadWilsonObservation operation);

} // namespace angel::prime
