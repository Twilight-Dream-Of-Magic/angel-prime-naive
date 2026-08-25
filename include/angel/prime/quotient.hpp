#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace angel::detail {
struct QuotientViewModel;
struct PrimeAccess;
}

namespace angel::prime {

struct CoordinateSummary final {
    std::uint64_t factor_count{};
    std::uint64_t block_width{};
    std::uint64_t full_blocks{};
    std::uint64_t tail_factors{};
    std::uint64_t polynomial_coefficients{};
    std::uint64_t evaluation_values{};
    std::uint64_t total_scalar_slots{};
    bool natural_square_root_coordinate{};
    bool polylogarithmic_claimed{};
    bool rejected_as_polylogarithmic{};
};

class QuotientView final {
public:
    QuotientView() = delete;
    QuotientView(const QuotientView&) noexcept = default;
    QuotientView(QuotientView&&) noexcept = default;
    QuotientView& operator=(const QuotientView&) noexcept = default;
    QuotientView& operator=(QuotientView&&) noexcept = default;
    ~QuotientView() = default;

    [[nodiscard]] std::uint64_t candidate() const noexcept;
    [[nodiscard]] CoordinateSummary coordinate() const noexcept;
    [[nodiscard]] bool preserves_complete_state_identity() const noexcept;

private:
    explicit QuotientView(std::shared_ptr<const detail::QuotientViewModel> model)
        : model_(std::move(model)) {}

    std::shared_ptr<const detail::QuotientViewModel> model_;
    friend struct detail::PrimeAccess;
};

} // namespace angel::prime
