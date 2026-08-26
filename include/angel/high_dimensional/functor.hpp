#pragma once

#include "angel/high_dimensional/arithmetic.hpp"

#include <cstdint>
#include <variant>
#include <vector>

namespace angel::high {

struct MazeAddress final {
    std::uint64_t room{1U};
    std::uint64_t road{1U};
    std::uint64_t frame{1U};
    std::uint64_t causal_epoch{};
    std::uint64_t history_depth{};
    std::uint64_t holonomy{};
    std::uint64_t singular_generation{};
    std::uint64_t higher_cell{};

    friend bool operator==(const MazeAddress&, const MazeAddress&) = default;
};

struct CausalEvent final {
    ArithmeticOpcode opcode{ArithmeticOpcode::HAdd};
    std::uint64_t left_parent_seal{};
    std::uint64_t right_parent_seal{};
    std::uint64_t event_seal{};

    friend bool operator==(const CausalEvent&, const CausalEvent&) = default;
};

class MazeState final {
public:
    MazeState() = delete;
    MazeState(const MazeState&) = default;
    MazeState(MazeState&&) noexcept = default;
    MazeState& operator=(const MazeState&) = default;
    MazeState& operator=(MazeState&&) noexcept = default;

    [[nodiscard]] const TriClassValue& value() const noexcept { return value_; }
    [[nodiscard]] const MazeAddress& address() const noexcept { return address_; }
    [[nodiscard]] const std::vector<CausalEvent>& history() const noexcept {
        return history_;
    }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }
    [[nodiscard]] bool exactly_equal(const MazeState& other) const noexcept;

private:
    TriClassValue value_;
    MazeAddress address_;
    std::vector<CausalEvent> history_;
    std::uint64_t seal_{};

    MazeState(
        TriClassValue value,
        MazeAddress address,
        std::vector<CausalEvent> history);

    friend MazeState upload_complete(TriClassValue, MazeAddress);
    friend class NativeFunctor;
    friend std::variant<MazeState, ArithmeticContinuation> hadd(
        const MazeState&, const MazeState&);
    friend std::variant<MazeState, ArithmeticContinuation> hsub(
        const MazeState&, const MazeState&);
    friend std::variant<MazeState, ArithmeticContinuation> hmul(
        const MazeState&, const MazeState&);
    friend std::variant<MazeState, ArithmeticContinuation> detail_lift_binary(
        const MazeState&,
        const MazeState&,
        ArithmeticOpcode,
        const ArithmeticResult&);
};

using StateResult = std::variant<MazeState, ArithmeticContinuation>;

struct OrdinarySpecification final {
    Coefficient value{};
    std::uint64_t schema_version{1U};
    MazeAddress initial_address{};
};

[[nodiscard]] MazeState upload_complete(
    TriClassValue value,
    MazeAddress address = {});

class UploadFunctor final {
public:
    [[nodiscard]] MazeState operator()(const OrdinarySpecification& input) const;
};

[[nodiscard]] StateResult hadd(const MazeState& left, const MazeState& right);
[[nodiscard]] StateResult hsub(const MazeState& left, const MazeState& right);
[[nodiscard]] StateResult hmul(const MazeState& left, const MazeState& right);

struct NativeInstruction final {
    ArithmeticOpcode opcode{ArithmeticOpcode::HAdd};
    TriClassValue operand;

    friend bool operator==(const NativeInstruction&, const NativeInstruction&) = default;
};

class NativeFunctor final {
public:
    NativeFunctor() = default;
    explicit NativeFunctor(std::vector<NativeInstruction> instructions);

    [[nodiscard]] static NativeFunctor identity();
    [[nodiscard]] static NativeFunctor add(TriClassValue operand);
    [[nodiscard]] static NativeFunctor subtract(TriClassValue operand);
    [[nodiscard]] static NativeFunctor multiply(TriClassValue operand);
    [[nodiscard]] static NativeFunctor divide_exact(TriClassValue operand);

    [[nodiscard]] const std::vector<NativeInstruction>& instructions() const noexcept {
        return instructions_;
    }
    [[nodiscard]] StateResult operator()(const MazeState& state) const;

private:
    std::vector<NativeInstruction> instructions_;
};

// compose(after, before) is after o before.
[[nodiscard]] NativeFunctor compose(
    const NativeFunctor& after,
    const NativeFunctor& before);

struct DerivedObservation final {
    Coefficient ordinary_coordinate{};
    std::uint64_t state_seal{};
    std::uint64_t history_depth{};
    std::uint64_t schema_version{1U};
    bool derived_non_authoritative{};

    friend bool operator==(const DerivedObservation&, const DerivedObservation&) = default;
};

class DownloadFunctor final {
public:
    [[nodiscard]] DerivedObservation operator()(const MazeState& state) const noexcept;
};

[[nodiscard]] bool validate_observation(
    const MazeState& canonical,
    const DerivedObservation& derived) noexcept;

MazeState promote_observation(const DerivedObservation&) = delete;

} // namespace angel::high
