#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <queue>
#include <stdexcept>
#include <vector>

namespace angel::afac63::behavioral {

// Exact finite reference for the future-sufficiency problem. This is audit
// mathematics, not a Prime/selector primitive and not part of native Execute.

template<std::size_t StateCount, std::size_t AlphabetSize>
struct FiniteMooreMachine final {
    static_assert(StateCount > 0U && StateCount <= 64U);
    static_assert(AlphabetSize > 0U);

    std::array<std::array<std::uint16_t, AlphabetSize>, StateCount> transition{};
    std::array<std::uint64_t, StateCount> observation{};
    std::uint64_t semantic_version{1U};
    std::uint64_t observer_contract{1U};

    [[nodiscard]] bool valid() const noexcept {
        if (semantic_version == 0U || observer_contract == 0U) return false;
        for (const auto& row : transition)
            for (const auto next : row)
                if (next >= StateCount) return false;
        return true;
    }
};

template<std::size_t StateCount>
struct BehavioralPartition final {
    std::array<std::uint16_t, StateCount> class_of{};
    std::uint16_t class_count{};
};

template<std::size_t AlphabetSize>
struct RefinementSignature final {
    std::uint64_t observation{};
    std::array<std::uint16_t, AlphabetSize> target_class{};
    bool operator==(const RefinementSignature&) const = default;
};

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline BehavioralPartition<StateCount> minimal_behavioral_partition(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine) {
    if (!machine.valid()) throw std::invalid_argument("invalid finite causal machine");

    BehavioralPartition<StateCount> partition{};
    std::vector<std::uint64_t> observations;
    observations.reserve(StateCount);
    for (std::size_t state = 0U; state < StateCount; ++state) {
        const auto it = std::find(observations.begin(), observations.end(),
                                  machine.observation[state]);
        if (it == observations.end()) {
            observations.push_back(machine.observation[state]);
            partition.class_of[state] = static_cast<std::uint16_t>(observations.size() - 1U);
        } else {
            partition.class_of[state] = static_cast<std::uint16_t>(
                std::distance(observations.begin(), it));
        }
    }
    partition.class_count = static_cast<std::uint16_t>(observations.size());

    for (;;) {
        std::array<std::uint16_t, StateCount> next{};
        std::vector<RefinementSignature<AlphabetSize>> signatures;
        signatures.reserve(StateCount);
        for (std::size_t state = 0U; state < StateCount; ++state) {
            RefinementSignature<AlphabetSize> signature{};
            signature.observation = machine.observation[state];
            for (std::size_t symbol = 0U; symbol < AlphabetSize; ++symbol)
                signature.target_class[symbol] =
                    partition.class_of[machine.transition[state][symbol]];
            const auto it = std::find(signatures.begin(), signatures.end(), signature);
            if (it == signatures.end()) {
                signatures.push_back(signature);
                next[state] = static_cast<std::uint16_t>(signatures.size() - 1U);
            } else {
                next[state] = static_cast<std::uint16_t>(
                    std::distance(signatures.begin(), it));
            }
        }
        if (next == partition.class_of) {
            partition.class_count = static_cast<std::uint16_t>(signatures.size());
            return partition;
        }
        partition.class_of = next;
        partition.class_count = static_cast<std::uint16_t>(signatures.size());
    }
}

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline bool future_equivalent(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine,
    const std::uint16_t left, const std::uint16_t right) {
    if (left >= StateCount || right >= StateCount)
        throw std::out_of_range("state index out of range");
    const auto partition = minimal_behavioral_partition(machine);
    return partition.class_of[left] == partition.class_of[right];
}

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline bool observation_is_resumable_without_complement(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine) {
    const auto partition = minimal_behavioral_partition(machine);
    for (std::size_t i = 0U; i < StateCount; ++i)
        for (std::size_t j = i + 1U; j < StateCount; ++j)
            if (machine.observation[i] == machine.observation[j] &&
                partition.class_of[i] != partition.class_of[j])
                return false;
    return true;
}

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline std::uint32_t resume_complement_lower_bound_bits(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine) {
    const auto partition = minimal_behavioral_partition(machine);
    std::uint16_t maximum = 0U;
    for (std::size_t i = 0U; i < StateCount; ++i) {
        std::array<bool, StateCount> seen{};
        std::uint16_t count = 0U;
        for (std::size_t j = 0U; j < StateCount; ++j) {
            if (machine.observation[j] != machine.observation[i]) continue;
            const auto cls = partition.class_of[j];
            if (!seen[cls]) {
                seen[cls] = true;
                ++count;
            }
        }
        maximum = std::max(maximum, count);
    }
    std::uint32_t bits = 0U;
    std::uint32_t capacity = 1U;
    while (capacity < maximum) {
        capacity <<= 1U;
        ++bits;
    }
    return bits;
}

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline std::uint64_t reachable_mask(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine,
    const std::uint16_t start) {
    if (!machine.valid() || start >= StateCount)
        throw std::invalid_argument("invalid machine or start state");
    std::uint64_t mask = std::uint64_t{1U} << start;
    std::queue<std::uint16_t> work;
    work.push(start);
    while (!work.empty()) {
        const auto state = work.front();
        work.pop();
        for (const auto next : machine.transition[state]) {
            const auto bit = std::uint64_t{1U} << next;
            if ((mask & bit) == 0U) {
                mask |= bit;
                work.push(next);
            }
        }
    }
    return mask;
}

struct ObservationStabilityCertificate final {
    std::uint16_t start_state{};
    std::uint64_t reachable_states{};
    std::uint64_t stable_observation{};
    std::uint64_t semantic_version{};
    std::uint64_t observer_contract{};
    bool all_legal_futures_stable{};
};

template<std::size_t StateCount, std::size_t AlphabetSize>
[[nodiscard]] inline std::optional<ObservationStabilityCertificate>
certify_observation_stability(
    const FiniteMooreMachine<StateCount, AlphabetSize>& machine,
    const std::uint16_t start) {
    const auto mask = reachable_mask(machine, start);
    const auto observed = machine.observation[start];
    for (std::size_t state = 0U; state < StateCount; ++state)
        if ((mask & (std::uint64_t{1U} << state)) != 0U &&
            machine.observation[state] != observed)
            return std::nullopt;
    return ObservationStabilityCertificate{
        start, mask, observed, machine.semantic_version,
        machine.observer_contract, true};
}

} // namespace angel::afac63::behavioral
