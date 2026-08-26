#pragma once

#include "angel/boundary.hpp"

namespace angel::boundary {

// Ordinary observations are derived, non-authoritative records.  Validation
// always compares them with the immutable observation already bound to the
// canonical closed Angel state; it never writes the ordinary payload back.
struct ObservationValidation final {
    bool canonical_state_bound{};
    bool observation_matches{};
    bool causal_cut_matches{};
    bool terminal_only{};

    [[nodiscard]] bool accepted() const noexcept {
        return canonical_state_bound && observation_matches &&
               causal_cut_matches && terminal_only;
    }
};

[[nodiscard]] ObservationValidation validate_download_packet(
    const ClosedObservation& canonical,
    const DownloadPacket& derived) noexcept;

} // namespace angel::boundary
