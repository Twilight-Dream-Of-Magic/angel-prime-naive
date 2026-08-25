#pragma once

#include <cstddef>

namespace angel::diagnostics {

struct FrozenLayout final {
    std::size_t arithmetic_state_bytes{};
    std::size_t cyclic_boundary_state_bytes{};
    std::size_t public_factorial_handle_bytes{};
    std::size_t public_boundary_handle_bytes{};
    bool exact_arithmetic_state_held{};
    bool exact_boundary_state_held{};
    bool wrapper_compresses_state{};
};

[[nodiscard]] FrozenLayout frozen_layout() noexcept;

} // namespace angel::diagnostics
