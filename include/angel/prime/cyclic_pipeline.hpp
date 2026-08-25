#pragma once

#include "angel/prime/cyclic_action.hpp"

namespace angel::prime {

struct BindCyclicAction final {};

struct EvaluateCyclicAction final {
    CyclicActionPolicy policy{};
};

struct DownloadCyclicStructure final {};

[[nodiscard]] constexpr BindCyclicAction bind_cyclic_action() noexcept {
    return {};
}

[[nodiscard]] constexpr EvaluateCyclicAction evaluate_cyclic_action(
    const CyclicActionPolicy policy = {}) noexcept {
    return EvaluateCyclicAction{policy};
}

[[nodiscard]] constexpr DownloadCyclicStructure
download_cyclic_structure() noexcept {
    return {};
}

[[nodiscard]] CyclicActionView operator|(
    const FactorialState& state, BindCyclicAction operation);

[[nodiscard]] ClosedCyclicAction operator|(
    const CyclicActionView& view, EvaluateCyclicAction operation);

[[nodiscard]] CyclicStructureDownload operator|(
    const ClosedCyclicAction& closed, DownloadCyclicStructure operation);

} // namespace angel::prime
