#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace angel::detail {
struct FactorialStateModel;
struct PrimeAccess;
}

namespace angel::prime {

struct FactorialStateSummary final {
    std::uint64_t candidate{};
    std::uint64_t factorial_argument{};
    std::uint64_t request_binding{};
    std::uint64_t state_seal{};
    std::uint64_t program_seal{};
    std::uint64_t input_bits{};
    std::uint64_t native_steps{};
    std::uint64_t payload_bytes{};
};

class FactorialState final {
public:
    FactorialState() = delete;
    FactorialState(const FactorialState&) noexcept = default;
    FactorialState(FactorialState&&) noexcept = default;
    FactorialState& operator=(const FactorialState&) noexcept = default;
    FactorialState& operator=(FactorialState&&) noexcept = default;
    ~FactorialState() = default;

    [[nodiscard]] FactorialStateSummary summary() const noexcept;

private:
    explicit FactorialState(std::shared_ptr<const detail::FactorialStateModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::FactorialStateModel> model_;
    friend struct detail::PrimeAccess;
};

} // namespace angel::prime
