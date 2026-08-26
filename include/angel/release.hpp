#pragma once

#include "angel/version.hpp"

namespace angel {

// The original public version header remains byte-for-byte compatible with
// SDK 1.1.0.  Additive releases expose their package identity here instead of
// rewriting the published compatibility header.
inline constexpr int additive_release_major = 1;
inline constexpr int additive_release_minor = 2;
inline constexpr int additive_release_patch = 0;
inline constexpr const char* additive_release = "1.2.0";
inline constexpr const char* additive_feature =
    "joint-time-space-wilson-observation";

} // namespace angel
