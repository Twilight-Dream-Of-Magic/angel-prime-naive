#pragma once

#include "angel/high_dimensional/functor.hpp"

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

namespace angel::high {

struct WeightedFunctor final {
    Coefficient weight{1};
    std::uint32_t branch_label{};
    NativeFunctor functor;
};

struct WeightedHistory final {
    Coefficient weight{};
    std::uint32_t branch_label{};
    std::vector<std::uint32_t> path;
    MazeState endpoint;
};

struct HistorySuperposition final {
    std::vector<WeightedHistory> branches;
};

using SuperpositionResult =
    std::variant<HistorySuperposition, ArithmeticContinuation>;

class ClassQuantumFunctor final {
public:
    ClassQuantumFunctor() = default;
    explicit ClassQuantumFunctor(std::vector<WeightedFunctor> branches);

    [[nodiscard]] const std::vector<WeightedFunctor>& branches() const noexcept {
        return branches_;
    }
    [[nodiscard]] SuperpositionResult operator()(const MazeState& input) const;

private:
    std::vector<WeightedFunctor> branches_;
};

[[nodiscard]] ClassQuantumFunctor superpose(
    const ClassQuantumFunctor& left,
    const ClassQuantumFunctor& right);

struct InterferenceResult final {
    HistorySuperposition survivors;
    std::uint64_t input_branches{};
    std::uint64_t canceled_branches{};
    std::uint64_t canceled_endpoints{};
    bool structural_key_only{};
};

// Interference is legal only after exact structural endpoints agree. Equal
// ordinary projections with different complete states are never canceled.
[[nodiscard]] std::variant<InterferenceResult, ArithmeticContinuation>
interfere_structurally(const HistorySuperposition& input);

struct TensorContinuation final {
    bool coefficient_overflow{};
    bool no_false_entanglement_claim{};
};

class TensorHistory final {
public:
    TensorHistory() = delete;

    [[nodiscard]] static TensorHistory exact(
        std::size_t left_dimension,
        std::size_t right_dimension,
        std::vector<Coefficient> coefficients);

    [[nodiscard]] std::size_t left_dimension() const noexcept {
        return left_dimension_;
    }
    [[nodiscard]] std::size_t right_dimension() const noexcept {
        return right_dimension_;
    }
    [[nodiscard]] const std::vector<Coefficient>& coefficients() const noexcept {
        return coefficients_;
    }
    [[nodiscard]] Coefficient at(std::size_t left, std::size_t right) const;

private:
    std::size_t left_dimension_{};
    std::size_t right_dimension_{};
    std::vector<Coefficient> coefficients_;

    TensorHistory(
        std::size_t left_dimension,
        std::size_t right_dimension,
        std::vector<Coefficient> coefficients);
};

using TensorBuildResult = std::variant<TensorHistory, TensorContinuation>;

[[nodiscard]] TensorBuildResult tensor_product(
    const std::vector<Coefficient>& left,
    const std::vector<Coefficient>& right);

struct EntanglementCertificate final {
    bool exact{};
    bool entangled{};
    std::size_t left_row_a{};
    std::size_t left_row_b{};
    std::size_t right_column_a{};
    std::size_t right_column_b{};
    Coefficient nonzero_minor{};
};

using EntanglementResult =
    std::variant<EntanglementCertificate, TensorContinuation>;

// A finite integer-weight tensor is called class-quantum entangled exactly
// when its coefficient matrix has rank greater than one over Q. A nonzero
// 2x2 minor is an exact certificate of nonseparability.
[[nodiscard]] EntanglementResult analyze_entanglement(
    const TensorHistory& tensor);

} // namespace angel::high
