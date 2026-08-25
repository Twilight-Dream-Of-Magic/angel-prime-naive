#pragma once

#include "angel/afac59/sublinear_wilson.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace angel::afac59 {

enum class PracticalWilsonBackend : std::uint8_t {
    R58ParallelLinear,
    R59CompositeSafeSublinear
};

struct PracticalWilsonPolicy final {
    std::uint64_t linear_cutover_candidate{1'000'000'000U};
    std::uint32_t linear_workers{8U};
    SublinearWilsonPolicy sublinear{
        SublinearWilsonPolicy::production()};
};

using PracticalWilsonExecution = std::variant<
    angel::afac58::WilsonApplicationResult,
    R59ApplicationResult>;

struct PracticalWilsonResult final {
    PracticalWilsonBackend backend{};
    PracticalWilsonExecution execution;

    [[nodiscard]] bool completed() const noexcept {
        if (const auto* linear = std::get_if<
                angel::afac58::WilsonApplicationResult>(&execution))
            return std::holds_alternative<angel::afac58::WilsonObservation>(
                linear->download);
        const auto& sublinear = std::get<R59ApplicationResult>(execution);
        return std::holds_alternative<SublinearWilsonObservation>(
            sublinear.download);
    }

    [[nodiscard]] std::optional<bool> prime_decision() const noexcept {
        if (const auto* linear = std::get_if<
                angel::afac58::WilsonApplicationResult>(&execution)) {
            if (const auto* observation =
                    std::get_if<angel::afac58::WilsonObservation>(
                        &linear->download))
                return observation->is_prime();
            return std::nullopt;
        }
        const auto& sublinear = std::get<R59ApplicationResult>(execution);
        if (const auto* observation =
                std::get_if<SublinearWilsonObservation>(&sublinear.download))
            return observation->is_prime();
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::uint64_t> residue() const noexcept {
        if (const auto* linear = std::get_if<
                angel::afac58::WilsonApplicationResult>(&execution)) {
            if (const auto* observation =
                    std::get_if<angel::afac58::WilsonObservation>(
                        &linear->download))
                return observation->residue();
            return std::nullopt;
        }
        const auto& sublinear = std::get<R59ApplicationResult>(execution);
        if (const auto* observation =
                std::get_if<SublinearWilsonObservation>(&sublinear.download))
            return observation->residue();
        return std::nullopt;
    }
};

[[nodiscard]] inline PracticalWilsonResult ANGEL_WILSON_PRIME_PRACTICAL(
    const std::uint64_t candidate,
    PracticalWilsonPolicy policy = {}) {
    if (candidate <= policy.linear_cutover_candidate) {
        auto execution = angel::afac58::ANGEL_WILSON_PRIME(
            candidate,
            angel::afac58::WilsonPolicy::strict_parallel(
                policy.linear_workers,
                policy.linear_cutover_candidate == 0U
                    ? 0U
                    : policy.linear_cutover_candidate - 1U));
        return PracticalWilsonResult{
            PracticalWilsonBackend::R58ParallelLinear,
            PracticalWilsonExecution{std::move(execution)}};
    }
    auto execution = ANGEL_WILSON_PRIME_SUBLINEAR(
        candidate, policy.sublinear);
    return PracticalWilsonResult{
        PracticalWilsonBackend::R59CompositeSafeSublinear,
        PracticalWilsonExecution{std::move(execution)}};
}

} // namespace angel::afac59
