#pragma once

// This is the only adapter that names the immutable historical implementation.
// Public headers and ordinary implementation modules depend on semantic aliases.
#include "angel/afac63/three_gate_boundary.hpp"

namespace angel::detail::frozen {
namespace hashing = ::angel::afac56;
namespace arithmetic = ::angel::afac57;
namespace factorial_boundary = ::angel::afac58;
namespace wilson = ::angel::afac59;
namespace quotient_view = ::angel::afac60;
namespace cyclic_action = ::angel::afac61;
namespace cyclic_boundary = ::angel::afac63;
} // namespace angel::detail::frozen
