#pragma once

#include "angel/prime/quotient.hpp"
#include "angel/state_integrity.hpp"

#include <cstdint>
#include <variant>

namespace angel::prime {

struct ObservationPolicy final {
    std::uint64_t maximum_block_width{100'000U};
    std::uint64_t block_width_override{};
    bool parallel_tree_branches{true};
    std::uint64_t parallel_tree_threshold{4096U};
};

struct ObservationLedger final {
    std::uint64_t input_bits{};
    std::uint64_t native_steps{};
    std::uint64_t state_payload_bytes{};
    std::uint64_t target_factor_count{};
    std::uint64_t block_width{};
    std::uint64_t full_blocks{};
    std::uint64_t tail_factors{};
    std::uint64_t factor_leaf_materializations{};
    std::uint64_t evaluation_points{};
    std::uint64_t block_value_multiplications{};
    std::uint64_t tail_multiplications{};
    std::uint64_t ordinary_elapsed_nanoseconds{};
    std::uint64_t polynomial_multiplications{};
    std::uint64_t maximum_polynomial_coefficients{};
    std::uint64_t maximum_transform_length{};
    bool ordinary_projection_started{};
    bool ordinary_projection_completed{};
    bool ordinary_feedback{};
};

struct WilsonObservation final {
    std::uint64_t candidate{};
    std::uint64_t factorial_residue{};
    bool prime{};
    ObservationLedger ledger{};
};

enum class LimitReason : std::uint8_t {
    BlockWidth = 1,
    PolynomialEngine = 2
};

struct ResourceLimit final {
    std::uint64_t candidate{};
    LimitReason reason{LimitReason::BlockWidth};
    std::uint64_t required_block_width{};
    std::uint64_t allowed_block_width{};
    ObservationLedger ledger{};
};

using OrdinaryObservation = std::variant<WilsonObservation, ResourceLimit>;

struct Download final {
    OrdinaryObservation observation{};
    CoordinateSummary coordinate{};
    StateIntegrity integrity{};
    bool verification_passed{};
};

} // namespace angel::prime
