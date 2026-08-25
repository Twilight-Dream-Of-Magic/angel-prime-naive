#pragma once

#define ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_MAJOR 1
#define ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_MINOR 1
#define ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_PATCH 0

namespace angel {

inline constexpr int sdk_version_major =
    ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_MAJOR;
inline constexpr int sdk_version_minor =
    ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_MINOR;
inline constexpr int sdk_version_patch =
    ANGEL_CAUSAL_BOUNDARY_SDK_VERSION_PATCH;
inline constexpr const char* sdk_version = "1.1.0";

} // namespace angel
