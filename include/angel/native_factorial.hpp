#pragma once

#include "angel/prime.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace angel::detail {
struct NativeFactorialViewModel;
struct ExactFactorialValueModel;
struct NativeFactorialAccess;
}

namespace angel::prime {

struct NativeFactorialSummary final {
    std::uint64_t candidate{};
    std::uint64_t factorial_argument{};
    std::uint64_t coefficient{};
    std::uint64_t request_binding{};
    std::uint64_t state_seal{};
    std::uint64_t program_seal{};
    std::uint64_t native_result_seal{};
    std::uint64_t native_certificate_seal{};
    std::uint64_t view_seal{};
    bool native_coordinate_verified{};
    bool denotation_materialized{};
};

class NativeFactorialView final {
public:
    NativeFactorialView() = delete;
    NativeFactorialView(const NativeFactorialView&) noexcept = default;
    NativeFactorialView(NativeFactorialView&&) noexcept = default;
    NativeFactorialView& operator=(const NativeFactorialView&) noexcept = default;
    NativeFactorialView& operator=(NativeFactorialView&&) noexcept = default;
    ~NativeFactorialView() = default;

    [[nodiscard]] NativeFactorialSummary summary() const noexcept;
    [[nodiscard]] bool preserves_complete_state_identity() const noexcept;

private:
    explicit NativeFactorialView(
        std::shared_ptr<const detail::NativeFactorialViewModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::NativeFactorialViewModel> model_;
    friend struct detail::NativeFactorialAccess;
};

struct ExactFactorialPolicy final {
    std::uint64_t maximum_factorial_argument{4096U};
};

struct ExactFactorialLedger final {
    std::uint64_t sequential_small_multiplications{};
    std::uint64_t sequential_limb_updates{};
    std::uint64_t product_tree_multiplications{};
    std::uint64_t product_tree_limb_products{};
    std::uint64_t decimal_divisions{};
    std::uint64_t decimal_limb_updates{};
    std::uint64_t result_limbs{};
    std::uint64_t result_bits{};
};

struct ExactFactorialSummary final {
    std::uint64_t factorial_argument{};
    std::uint64_t bit_length{};
    std::uint64_t limb_count{};
    std::uint64_t value_hash{};
    std::uint64_t exact_value_seal{};
    bool native_coordinate_consumed{};
    bool sequential_product_tree_equal{};
};

class ExactFactorialValue final {
public:
    ExactFactorialValue() = delete;
    ExactFactorialValue(const ExactFactorialValue&) noexcept = default;
    ExactFactorialValue(ExactFactorialValue&&) noexcept = default;
    ExactFactorialValue& operator=(const ExactFactorialValue&) noexcept = default;
    ExactFactorialValue& operator=(ExactFactorialValue&&) noexcept = default;
    ~ExactFactorialValue() = default;

    [[nodiscard]] ExactFactorialSummary summary() const noexcept;

private:
    explicit ExactFactorialValue(
        std::shared_ptr<const detail::ExactFactorialValueModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::ExactFactorialValueModel> model_;
    friend struct detail::NativeFactorialAccess;
};

struct ExactFactorialDownload final {
    std::uint64_t factorial_argument{};
    std::string decimal{};
    std::string hexadecimal{};
    ExactFactorialLedger ledger{};
    StateIntegrity integrity{};
    bool native_coordinate_consumed{};
    bool independent_product_tree_verified{};
    bool value_seal_verified{};
    bool verification_passed{};
};

enum class WilsonConsumptionMode : std::uint8_t {
    NativeCoordinateModularProjection = 1,
    ExactBigIntegerRemainder = 2
};

struct NativeWilsonEvidence final {
    WilsonConsumptionMode mode{
        WilsonConsumptionMode::NativeCoordinateModularProjection};
    std::uint64_t factorial_argument{};
    std::uint64_t native_coefficient{};
    std::uint64_t native_result_seal{};
    std::uint64_t native_certificate_seal{};
    std::uint64_t exact_value_seal{};
    bool native_coordinate_verified{};
    bool factor_count_loaded_from_native_coordinate{};
    bool candidate_used_only_as_modulus{};
    bool exact_big_integer_consumed{};
    bool modular_result_matches_exact_big_integer{};
};

struct NativeWilsonDownload final {
    OrdinaryObservation observation{};
    CoordinateSummary coordinate{};
    StateIntegrity integrity{};
    NativeWilsonEvidence evidence{};
    bool verification_passed{};

    [[nodiscard]] bool verified() const noexcept {
        return verification_passed && integrity.preserved() &&
               evidence.native_coordinate_verified &&
               evidence.factor_count_loaded_from_native_coordinate;
    }
};

struct BindNativeFactorial final {};
struct DeriveExactFactorial final {
    ExactFactorialPolicy policy{};
};
struct DownloadExactFactorial final {};
struct ProjectWilsonFromNative final {
    ObservationPolicy policy{};
};
struct ObserveWilsonFromExact final {};

[[nodiscard]] constexpr BindNativeFactorial bind_native_factorial() noexcept {
    return {};
}

[[nodiscard]] constexpr DeriveExactFactorial derive_exact_factorial(
    const ExactFactorialPolicy policy = {}) noexcept {
    return DeriveExactFactorial{policy};
}

[[nodiscard]] constexpr DownloadExactFactorial
download_exact_factorial() noexcept {
    return {};
}

[[nodiscard]] constexpr ProjectWilsonFromNative project_wilson_from_native(
    const ObservationPolicy policy = {}) noexcept {
    return ProjectWilsonFromNative{policy};
}

[[nodiscard]] constexpr ObserveWilsonFromExact
observe_wilson_from_exact() noexcept {
    return {};
}

[[nodiscard]] NativeFactorialView operator|(
    const FactorialState& state, BindNativeFactorial operation);
[[nodiscard]] ExactFactorialValue operator|(
    const NativeFactorialView& view, DeriveExactFactorial operation);
[[nodiscard]] ExactFactorialDownload operator|(
    const ExactFactorialValue& value, DownloadExactFactorial operation);
[[nodiscard]] NativeWilsonDownload operator|(
    const NativeFactorialView& view, ProjectWilsonFromNative operation);
[[nodiscard]] NativeWilsonDownload operator|(
    const ExactFactorialValue& value, ObserveWilsonFromExact operation);

} // namespace angel::prime
