#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace angel::boundary {

enum class ByteOrder : std::uint8_t {
    LittleEndian = 1,
    BigEndian = 2
};

struct EncodedOrder final {
    std::array<std::uint8_t, 8> bytes{};
    std::uint8_t byte_count{1U};
    ByteOrder byte_order{ByteOrder::LittleEndian};

    [[nodiscard]] static EncodedOrder canonical(std::uint64_t order);
};

struct NormalizationEvidence final {
    std::uint64_t canonical_order{};
    std::uint8_t canonical_byte_count{};
    std::uint8_t removed_high_zero_bytes{};
    bool endian_reordered{};
    bool exact_value_preserved{};
};

[[nodiscard]] std::optional<NormalizationEvidence> normalize(
    const EncodedOrder& encoded) noexcept;

} // namespace angel::boundary
