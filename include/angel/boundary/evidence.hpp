#pragma once

#include "angel/boundary/state.hpp"

#include <cstdint>

namespace angel::boundary {

struct PresentationEvidence final {
    Presentation source{};
    Presentation target{};
    std::uint64_t forward_exponent{};
    std::uint64_t backward_exponent{};
    bool exact_inverse{};
    bool accepted{};
};

struct LeastCommitmentEvidence final {
    std::uint64_t order{};
    std::uint64_t fresh_cycles{};
    std::uint64_t compared_cycles{};
    bool fresh_is_initial{};
    bool accepted{};
};

struct RamifiedTransportEvidence final {
    std::uint64_t source_order{};
    std::uint64_t target_order{};
    std::uint64_t ramification_index{};
    bool exact_quotient_transport{};
    bool terminal_projector_collapses{};
    bool primitive_content_first_visible_at_ramification{};
    bool accepted{};
};

[[nodiscard]] LeastCommitmentEvidence verify_least_commitment(
    const State& fresh, const State& compared) noexcept;

[[nodiscard]] RamifiedTransportEvidence verify_ramified_transport(
    std::uint64_t source_order,
    std::uint64_t target_order,
    std::uint64_t audit_modulus = 998244353U,
    BoundaryLedger* ledger = nullptr);

} // namespace angel::boundary
