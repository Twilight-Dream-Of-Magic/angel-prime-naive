#pragma once

#include "angel/boundary/session.hpp"

#include <cstdint>
#include <memory>
#include <utility>

namespace angel::detail {
struct BoundaryStateModel;
struct CheckpointModel;
struct BoundaryAccess;
}

namespace angel::boundary {

enum class Origin : std::uint8_t {
    FreshUpload = 1,
    RamifiedQuotient = 2,
    CheckpointImport = 3,
    SameFrameContinuation = 4
};

struct Presentation final {
    std::uint64_t order{};
    std::uint64_t exponent{1U};
    std::uint64_t inverse_exponent{1U};
};

struct StateSummary final {
    std::uint64_t order{};
    std::uint64_t cycles{};
    std::uint64_t source_order{};
    std::uint64_t available_jet_order{};
    std::uint64_t continuation_epoch{};
    std::uint64_t residual_generation{};
    SessionId session{};
    Origin origin{Origin::FreshUpload};
    Presentation presentation{};
};

struct BoundaryLedger final {
    std::uint64_t representation_normalizations{};
    std::uint64_t fresh_origins_created{};
    std::uint64_t presentation_transports{};
    std::uint64_t native_quotients{};
    std::uint64_t native_same_frame_continuations{};
    std::uint64_t native_exports{};
    std::uint64_t native_imports{};
    std::uint64_t closure_checks{};
    std::uint64_t reference_germ_materializations{};
    std::uint64_t dense_reference_coefficients{};
    std::uint64_t dense_reference_updates{};
    std::uint64_t downloads{};
    std::uint64_t nodes_rewritten{};
    std::uint64_t nodes_merged{};
    bool ordinary_feedback{};
};

class State final {
public:
    State() = delete;
    State(const State&) noexcept = default;
    State(State&&) noexcept = default;
    State& operator=(const State&) noexcept = default;
    State& operator=(State&&) noexcept = default;
    ~State() = default;

    [[nodiscard]] StateSummary summary() const noexcept;
    [[nodiscard]] bool exactly_equal(const State& other) const noexcept;
    [[nodiscard]] bool presentation_equivalent(const State& other) const noexcept;

private:
    explicit State(std::shared_ptr<const detail::BoundaryStateModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::BoundaryStateModel> model_;
    friend struct detail::BoundaryAccess;
};

class NativeCheckpoint final {
public:
    NativeCheckpoint() = delete;
    NativeCheckpoint(const NativeCheckpoint&) noexcept = default;
    NativeCheckpoint(NativeCheckpoint&&) noexcept = default;
    NativeCheckpoint& operator=(const NativeCheckpoint&) noexcept = default;
    NativeCheckpoint& operator=(NativeCheckpoint&&) noexcept = default;
    ~NativeCheckpoint() = default;

private:
    explicit NativeCheckpoint(std::shared_ptr<const detail::CheckpointModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::CheckpointModel> model_;
    friend struct detail::BoundaryAccess;
};

} // namespace angel::boundary
