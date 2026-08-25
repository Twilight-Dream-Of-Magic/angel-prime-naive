#include "angel/boundary/encoding.hpp"
#include "internal/conversions.hpp"

#include <stdexcept>

namespace angel::boundary {

EncodedOrder EncodedOrder::canonical(const std::uint64_t order) {
    const auto frozen = detail::frozen::cyclic_boundary::SpecificationGate::
        encode_canonical(order);
    EncodedOrder out{};
    out.bytes = frozen.bytes;
    out.byte_count = frozen.byte_count;
    out.byte_order = ByteOrder::LittleEndian;
    return out;
}

std::optional<NormalizationEvidence> normalize(
    const EncodedOrder& encoded) noexcept {
    const auto normalized = detail::frozen::cyclic_boundary::SpecificationGate::
        normalize(detail::to_frozen(encoded));
    if (!normalized) return std::nullopt;
    const auto& witness = normalized->witness();
    return NormalizationEvidence{
        witness.canonical_order,
        witness.canonical_byte_count,
        witness.removed_high_zero_bytes,
        witness.endian_reordered,
        witness.exact_discrete_value_preserved};
}

} // namespace angel::boundary
