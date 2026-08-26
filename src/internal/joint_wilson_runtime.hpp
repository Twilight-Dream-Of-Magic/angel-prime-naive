#pragma once

#include "angel/joint_wilson.hpp"
#include "internal/frozen_types.hpp"

#include <cstdint>

namespace angel::detail {

struct JointWilsonExecution final {
    std::uint64_t residue{};
    prime::JointWilsonCoordinate coordinate{};
    prime::JointWilsonLedger ledger{};
};

struct WilsonParetoAudit final {
    std::uint64_t factor_count{};
    std::uint64_t modulus{};
    std::uint64_t legacy_residue{};
    std::uint64_t optimized_residue{};
    std::uint64_t exact_residue{};
    std::uint64_t legacy_work_units{};
    std::uint64_t optimized_work_units{};
    std::uint64_t legacy_peak_live_coefficients{};
    std::uint64_t optimized_peak_live_coefficients{};
    std::uint64_t legacy_materialized_coordinate_count{};
    std::uint64_t optimized_materialized_coordinate_count{};
    prime::JointWilsonCoordinate legacy_coordinate{};
    prime::JointWilsonCoordinate optimized_coordinate{};
    prime::JointWilsonLedger legacy_ledger{};
    prime::JointWilsonLedger optimized_ledger{};
    bool residues_equal{};
    bool time_work_strictly_lower{};
    bool peak_space_strictly_lower{};
    bool coordinate_strictly_lower{};
    bool ring_additions_strictly_lower{};
    bool ring_multiplications_strictly_lower{};
    bool modular_reductions_strictly_lower{};
    bool coefficient_updates_strictly_lower{};
    bool peak_limbs_strictly_lower{};
};

[[nodiscard]] JointWilsonExecution execute_joint_wilson_projection(
    std::uint64_t factor_count,
    std::uint64_t modulus,
    const prime::JointWilsonPolicy& policy);

[[nodiscard]] WilsonParetoAudit audit_joint_wilson_projection(
    std::uint64_t factor_count,
    std::uint64_t modulus,
    const prime::JointWilsonPolicy& policy);

} // namespace angel::detail
