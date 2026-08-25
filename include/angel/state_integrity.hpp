#pragma once

#include <cstdint>

namespace angel {

struct StateIntegrity final {
    std::uint64_t request_binding_before{};
    std::uint64_t request_binding_after{};
    std::uint64_t state_seal_before{};
    std::uint64_t state_seal_after{};
    std::uint64_t program_seal_before{};
    std::uint64_t program_seal_after{};
    std::uint64_t certificate_seal_before{};
    std::uint64_t certificate_seal_after{};
    std::uint64_t payload_bytes_before{};
    std::uint64_t payload_bytes_after{};
    std::uint64_t nodes_rewritten{};
    std::uint64_t nodes_merged{};
    bool state_compressed{};
    bool ordinary_feedback{};
    bool verification_passed{};

    [[nodiscard]] bool preserved() const noexcept {
        return verification_passed &&
               request_binding_before == request_binding_after &&
               state_seal_before == state_seal_after &&
               program_seal_before == program_seal_after &&
               certificate_seal_before == certificate_seal_after &&
               payload_bytes_before == payload_bytes_after &&
               nodes_rewritten == 0U && nodes_merged == 0U &&
               !state_compressed && !ordinary_feedback;
    }
};

} // namespace angel
