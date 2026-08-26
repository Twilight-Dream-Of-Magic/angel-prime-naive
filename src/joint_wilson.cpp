#include "angel/joint_wilson.hpp"
#include "internal/joint_wilson_runtime.hpp"
#include "internal/models.hpp"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace angel::detail {
namespace {

using Polynomial = frozen::wilson::Polynomial;
using PolynomialLedger = frozen::wilson::PolynomialLedger;
using PolynomialPolicy = frozen::wilson::PolynomialPolicy;
using PolynomialRing = frozen::wilson::CompositeSafePolynomialRing;
using WideUnsigned = frozen::wilson::wide_uint;

struct NativeBinding final {
    std::uint64_t factorial_argument{};
    std::uint64_t coefficient{};
    std::uint64_t result_seal{};
    std::uint64_t certificate_seal{};
    bool verified{};
};

[[nodiscard]] NativeBinding verify_native_binding(
    const frozen::factorial_boundary::CertifiedWilsonRequest& request) noexcept {
    NativeBinding binding{};
    const auto request_report = frozen::factorial_boundary::
        WilsonRequestVerifier::verify(request);
    if (!request_report.accepted) return binding;

    const auto& execution = request.factorial_execution();
    const auto& native = execution.r56_factorial;
    const auto native_report = frozen::hashing::IndependentVerifier::verify(
        execution.principal_jet.valuation(), native.source_program,
        native.jet_effect, native.result);
    const auto& coordinate = native.result.value();
    if (!coordinate.rank().fits_u64()) return binding;

    binding.factorial_argument = coordinate.rank().to_u64();
    binding.coefficient = coordinate.coefficient();
    binding.result_seal = coordinate.seal();
    binding.certificate_seal = native.result.certificate().seal;
    binding.verified = native_report.accepted() &&
        binding.coefficient == 1U &&
        binding.factorial_argument == request.candidate() - 1U &&
        coordinate.rank() == execution.principal_jet.valuation() &&
        coordinate.rank() == execution.principal_jet.target() &&
        native.result.certificate().result_binding == coordinate.seal();
    return binding;
}

[[nodiscard]] std::uint64_t native_view_seal(
    const frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const NativeBinding& binding) noexcept {
    const auto& state = request.factorial_execution().principal_jet;
    return frozen::hashing::mix64(
        request.binding_seal() ^
        std::rotl(state.seal(), 11) ^
        std::rotl(state.source_program().program().seal(), 23) ^
        std::rotl(binding.factorial_argument, 37) ^
        std::rotl(binding.coefficient, 43) ^
        std::rotl(binding.result_seal, 51) ^
        std::rotl(binding.certificate_seal, 59) ^
        0x4e41544641435631ULL);
}

[[nodiscard]] StateIntegrity state_integrity(
    const frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const std::uint64_t certificate_seal,
    const bool verified) noexcept {
    const auto& state = request.factorial_execution().principal_jet;
    return StateIntegrity{
        request.binding_seal(),
        request.binding_seal(),
        state.seal(),
        state.seal(),
        state.source_program().program().seal(),
        state.source_program().program().seal(),
        certificate_seal,
        certificate_seal,
        state.payload_bytes(),
        state.payload_bytes(),
        0U,
        0U,
        false,
        false,
        verified};
}

[[nodiscard]] std::uint64_t checked_add(
    const std::uint64_t left, const std::uint64_t right) {
    if (right > std::numeric_limits<std::uint64_t>::max() - left)
        throw std::overflow_error("exact operation counter overflow");
    return left + right;
}

[[nodiscard]] std::uint64_t checked_multiply(
    const std::uint64_t left, const std::uint64_t right) {
    if (left != 0U && right > std::numeric_limits<std::uint64_t>::max() / left)
        throw std::overflow_error("exact operation counter overflow");
    return left * right;
}

void checked_accumulate(std::uint64_t& target, const std::uint64_t value) {
    target = checked_add(target, value);
}

[[nodiscard]] std::uint64_t floor_square_root(
    const std::uint64_t value) noexcept {
    const auto ceiling = frozen::wilson::ceil_square_root(value);
    if (ceiling == 0U) return 0U;
    return value / ceiling == ceiling && value % ceiling == 0U
        ? ceiling
        : ceiling - 1U;
}

[[nodiscard]] std::uint64_t ceil_divide(
    const std::uint64_t numerator,
    const std::uint64_t denominator) noexcept {
    return numerator / denominator +
        static_cast<std::uint64_t>(numerator % denominator != 0U);
}

struct ScheduleDimension final {
    std::uint64_t width{};
    std::uint64_t full_blocks{};
    std::uint64_t tail{};
    std::uint64_t coordinate{};
    std::uint64_t balance{};
};

[[nodiscard]] ScheduleDimension schedule_dimension(
    const std::uint64_t factor_count,
    const std::uint64_t width) {
    if (width == 0U || width > factor_count)
        throw std::invalid_argument("invalid block width");
    const auto full_blocks = factor_count / width;
    const auto tail = factor_count % width;
    return ScheduleDimension{
        width,
        full_blocks,
        tail,
        checked_add(checked_add(width, full_blocks), tail),
        std::max(width, full_blocks)};
}

[[nodiscard]] bool dimension_less(
    const ScheduleDimension& left,
    const ScheduleDimension& right) noexcept {
    return std::tie(left.coordinate, left.balance, left.tail, left.width) <
           std::tie(right.coordinate, right.balance, right.tail, right.width);
}

[[nodiscard]] std::uint64_t choose_block_width(
    const std::uint64_t factor_count,
    const std::uint64_t radius) {
    if (factor_count == 0U) return 0U;
    if (radius > 4'096U)
        throw std::invalid_argument("width-search radius exceeds fixed policy cap");

    const auto floor_root = std::max<std::uint64_t>(
        1U, floor_square_root(factor_count));
    const auto ceil_root = frozen::wilson::ceil_square_root(factor_count);
    const auto rectangular = ceil_divide(factor_count, floor_root);

    ScheduleDimension best{
        1U, factor_count, 0U,
        std::numeric_limits<std::uint64_t>::max(),
        std::numeric_limits<std::uint64_t>::max()};

    const auto consider = [&](const std::uint64_t width) {
        if (width == 0U || width > factor_count) return;
        const auto candidate = schedule_dimension(factor_count, width);
        if (dimension_less(candidate, best)) best = candidate;
    };

    const auto inspect_neighbourhood = [&](const std::uint64_t base) {
        consider(base);
        for (std::uint64_t delta = 1U; delta <= radius; ++delta) {
            if (base > delta) consider(base - delta);
            if (base <= factor_count - std::min(factor_count, delta)) {
                const auto upper = base + delta;
                if (upper >= base) consider(upper);
            }
        }
    };

    inspect_neighbourhood(floor_root);
    inspect_neighbourhood(ceil_root);
    inspect_neighbourhood(rectangular);
    return best.width;
}

class LiveCoefficientTracker final {
public:
    void acquire_polynomial(const std::uint64_t coefficients) {
        current_ = checked_add(current_, coefficients);
        peak_ = std::max(peak_, current_);
        ++polynomial_objects_;
        ++allocations_;
    }

    void release_polynomial(const std::uint64_t coefficients) noexcept {
        current_ -= coefficients;
    }

    void acquire_scalar_buffer(const std::uint64_t coefficients) {
        current_ = checked_add(current_, coefficients);
        peak_ = std::max(peak_, current_);
        ++allocations_;
    }

    void release_scalar_buffer(const std::uint64_t coefficients) noexcept {
        current_ -= coefficients;
    }

    void node_allocation() { ++allocations_; }

    void observe_transient(const std::uint64_t coefficients) {
        peak_ = std::max(peak_, checked_add(current_, coefficients));
    }

    [[nodiscard]] std::uint64_t peak() const noexcept { return peak_; }
    [[nodiscard]] std::uint64_t current() const noexcept { return current_; }
    [[nodiscard]] std::uint64_t polynomial_objects() const noexcept {
        return polynomial_objects_;
    }
    [[nodiscard]] std::uint64_t allocations() const noexcept {
        return allocations_;
    }

private:
    std::uint64_t current_{};
    std::uint64_t peak_{};
    std::uint64_t polynomial_objects_{};
    std::uint64_t allocations_{};
};

class OwnedPolynomial final {
public:
    OwnedPolynomial() noexcept = default;

    OwnedPolynomial(Polynomial value, LiveCoefficientTracker& tracker)
        : value_(std::move(value)), tracker_(&tracker) {
        tracker_->acquire_polynomial(
            static_cast<std::uint64_t>(value_.size()));
    }

    OwnedPolynomial(const OwnedPolynomial&) = delete;
    OwnedPolynomial& operator=(const OwnedPolynomial&) = delete;

    OwnedPolynomial(OwnedPolynomial&& other) noexcept
        : value_(std::move(other.value_)), tracker_(other.tracker_) {
        other.tracker_ = nullptr;
    }

    OwnedPolynomial& operator=(OwnedPolynomial&& other) noexcept {
        if (this == &other) return *this;
        reset();
        value_ = std::move(other.value_);
        tracker_ = other.tracker_;
        other.tracker_ = nullptr;
        return *this;
    }

    ~OwnedPolynomial() { reset(); }

    [[nodiscard]] const Polynomial& value() const noexcept { return value_; }
    [[nodiscard]] Polynomial& value() noexcept { return value_; }
    [[nodiscard]] std::uint64_t size() const noexcept {
        return static_cast<std::uint64_t>(value_.size());
    }

    void reset() noexcept {
        if (tracker_) {
            tracker_->release_polynomial(
                static_cast<std::uint64_t>(value_.size()));
            Polynomial empty{};
            value_.swap(empty);
            tracker_ = nullptr;
        }
    }

private:
    Polynomial value_{};
    LiveCoefficientTracker* tracker_{};
};

class OwnedScalarBuffer final {
public:
    OwnedScalarBuffer() noexcept = default;

    OwnedScalarBuffer(
        const std::uint64_t size,
        LiveCoefficientTracker& tracker)
        : values_(static_cast<std::size_t>(size), 0U), tracker_(&tracker) {
        tracker_->acquire_scalar_buffer(size);
    }

    OwnedScalarBuffer(const OwnedScalarBuffer&) = delete;
    OwnedScalarBuffer& operator=(const OwnedScalarBuffer&) = delete;

    OwnedScalarBuffer(OwnedScalarBuffer&& other) noexcept
        : values_(std::move(other.values_)), tracker_(other.tracker_) {
        other.tracker_ = nullptr;
    }

    OwnedScalarBuffer& operator=(OwnedScalarBuffer&& other) noexcept {
        if (this == &other) return *this;
        reset();
        values_ = std::move(other.values_);
        tracker_ = other.tracker_;
        other.tracker_ = nullptr;
        return *this;
    }

    ~OwnedScalarBuffer() { reset(); }

    [[nodiscard]] std::uint64_t size() const noexcept {
        return static_cast<std::uint64_t>(values_.size());
    }

    [[nodiscard]] std::uint64_t& operator[](
        const std::uint64_t index) noexcept {
        return values_[static_cast<std::size_t>(index)];
    }

    [[nodiscard]] const std::uint64_t& operator[](
        const std::uint64_t index) const noexcept {
        return values_[static_cast<std::size_t>(index)];
    }

    [[nodiscard]] auto begin() noexcept { return values_.begin(); }
    [[nodiscard]] auto end() noexcept { return values_.end(); }
    [[nodiscard]] auto begin() const noexcept { return values_.begin(); }
    [[nodiscard]] auto end() const noexcept { return values_.end(); }

    void reset() noexcept {
        if (tracker_) {
            tracker_->release_scalar_buffer(
                static_cast<std::uint64_t>(values_.size()));
            std::vector<std::uint64_t> empty{};
            values_.swap(empty);
            tracker_ = nullptr;
        }
    }

private:
    std::vector<std::uint64_t> values_{};
    LiveCoefficientTracker* tracker_{};
};

struct AtomicOperationLedger final {
    std::uint64_t classical_remainder_products{};
    std::uint64_t classical_remainder_additions{};
    std::uint64_t classical_remainder_updates{};
    std::uint64_t scalar_modular_multiplications{};
    std::uint64_t scalar_modular_additions{};
    std::uint64_t point_generation_updates{};
    std::uint64_t copied_bytes_upper_bound{};
};

[[nodiscard]] OwnedPolynomial make_owned_polynomial(
    Polynomial value,
    LiveCoefficientTracker& tracker) {
    return OwnedPolynomial{std::move(value), tracker};
}

[[nodiscard]] OwnedPolynomial multiply_polynomials(
    PolynomialRing& ring,
    const OwnedPolynomial& left,
    const OwnedPolynomial& right,
    LiveCoefficientTracker& tracker) {
    const auto result_size = checked_add(left.size(), right.size()) - 1U;
    tracker.observe_transient(result_size);
    return make_owned_polynomial(
        ring.multiply(left.value(), right.value()), tracker);
}

[[nodiscard]] OwnedPolynomial classical_monic_remainder(
    const Polynomial& dividend,
    const Polynomial& divisor,
    const std::uint64_t modulus,
    PolynomialLedger& polynomial_ledger,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    ++polynomial_ledger.monic_remainders;
    if (divisor.empty() || divisor.back() != 1U)
        throw std::invalid_argument("remainder divisor must be monic");

    checked_accumulate(
        atomic.copied_bytes_upper_bound,
        checked_multiply(static_cast<std::uint64_t>(dividend.size()),
                         sizeof(std::uint64_t)));
    if (dividend.size() < divisor.size())
        return make_owned_polynomial(Polynomial(dividend), tracker);

    Polynomial remainder = dividend;
    const auto divisor_degree = divisor.size() - 1U;
    for (std::size_t cursor = remainder.size(); cursor-- > divisor_degree;) {
        const auto coefficient = remainder[cursor];
        if (coefficient == 0U) continue;
        const auto shift = cursor - divisor_degree;
        for (std::size_t index = 0U; index < divisor_degree; ++index) {
            remainder[shift + index] = frozen::wilson::subtract_mod(
                remainder[shift + index],
                frozen::wilson::multiply_mod(
                    coefficient, divisor[index], modulus),
                modulus);
            ++atomic.classical_remainder_products;
            ++atomic.classical_remainder_additions;
            ++atomic.classical_remainder_updates;
        }
        remainder[cursor] = 0U;
        ++atomic.classical_remainder_updates;
    }
    remainder.resize(divisor_degree);
    frozen::wilson::trim(remainder);
    return make_owned_polynomial(std::move(remainder), tracker);
}

[[nodiscard]] OwnedPolynomial remainder_polynomial(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const PolynomialPolicy& policy,
    const OwnedPolynomial& dividend,
    const OwnedPolynomial& divisor,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    if (divisor.value().empty() || divisor.value().back() != 1U)
        throw std::invalid_argument("remainder divisor must be monic");
    if (dividend.value().size() < divisor.value().size())
        return classical_monic_remainder(
            dividend.value(), divisor.value(), ring.modulus(),
            polynomial_ledger, atomic, tracker);

    const auto quotient_size =
        dividend.value().size() - divisor.value().size() + 1U;
    const auto classical_cost = static_cast<WideUnsigned>(quotient_size) *
                                divisor.value().size();
    if (policy.force_schoolbook ||
        classical_cost <= policy.schoolbook_product_limit) {
        return classical_monic_remainder(
            dividend.value(), divisor.value(), ring.modulus(),
            polynomial_ledger, atomic, tracker);
    }

    const auto copy_terms = checked_add(dividend.size(), divisor.size());
    checked_accumulate(
        atomic.copied_bytes_upper_bound,
        checked_multiply(checked_multiply(copy_terms, 6U),
                         sizeof(std::uint64_t)));
    tracker.observe_transient(checked_multiply(copy_terms, 6U));
    return make_owned_polynomial(
        ring.monic_remainder(dividend.value(), divisor.value()), tracker);
}

[[nodiscard]] std::uint64_t evaluate_polynomial(
    const OwnedPolynomial& polynomial,
    const std::uint64_t point,
    const std::uint64_t modulus,
    PolynomialLedger& ledger) {
    ++ledger.horner_evaluations;
    if (polynomial.value().empty()) return 0U;

    // Start from the leading coefficient.  The previous schedule initialized
    // Horner with zero, which paid one multiply-by-zero and one redundant add
    // at every leaf.  This is an exact algebraic elimination, not a
    // time-space exchange.
    const auto steps = polynomial.size() - 1U;
    checked_accumulate(ledger.horner_coefficient_steps, steps);
    std::uint64_t value = polynomial.value().back();
    for (auto cursor = polynomial.value().rbegin() + 1U;
         cursor != polynomial.value().rend(); ++cursor) {
        value = frozen::wilson::add_mod(
            frozen::wilson::multiply_mod(value, point, modulus),
            *cursor, modulus);
    }
    return value;
}

[[nodiscard]] std::uint64_t evaluate_polynomial_legacy(
    const OwnedPolynomial& polynomial,
    const std::uint64_t point,
    const std::uint64_t modulus,
    PolynomialLedger& ledger) {
    ++ledger.horner_evaluations;
    checked_accumulate(
        ledger.horner_coefficient_steps, polynomial.size());
    std::uint64_t value = 0U;
    for (auto cursor = polynomial.value().rbegin();
         cursor != polynomial.value().rend(); ++cursor) {
        value = frozen::wilson::add_mod(
            frozen::wilson::multiply_mod(value, point, modulus),
            *cursor, modulus);
    }
    return value;
}

[[nodiscard]] OwnedPolynomial build_factor_polynomial(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const std::uint64_t begin,
    const std::uint64_t end,
    LiveCoefficientTracker& tracker) {
    if (end - begin == 1U) {
        return make_owned_polynomial(
            Polynomial{(begin + 1U) % ring.modulus(), 1U}, tracker);
    }
    const auto middle = begin + ((end - begin) >> 1U);
    auto left = build_factor_polynomial(
        ring, polynomial_ledger, begin, middle, tracker);
    auto right = build_factor_polynomial(
        ring, polynomial_ledger, middle, end, tracker);
    ++polynomial_ledger.product_tree_internal_nodes;
    return multiply_polynomials(ring, left, right, tracker);
}

struct EvaluationNode final {
    std::uint64_t begin{};
    std::uint64_t end{};
    std::uint64_t point{};
    OwnedPolynomial product{};
    std::unique_ptr<EvaluationNode> left{};
    std::unique_ptr<EvaluationNode> right{};

    EvaluationNode(
        const std::uint64_t input_begin,
        const std::uint64_t input_end,
        const std::uint64_t input_point,
        OwnedPolynomial input_product)
        : begin(input_begin), end(input_end), point(input_point),
          product(std::move(input_product)) {}
};

[[nodiscard]] std::unique_ptr<EvaluationNode> build_evaluation_tree(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const std::uint64_t width,
    const std::uint64_t begin,
    const std::uint64_t end,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    if (begin >= end)
        throw std::invalid_argument("empty evaluation range");
    tracker.node_allocation();
    if (end - begin == 1U) {
        const auto point = static_cast<std::uint64_t>(
            (static_cast<WideUnsigned>(begin) * width) % ring.modulus());
        ++atomic.point_generation_updates;
        const auto constant = point == 0U ? 0U : ring.modulus() - point;
        return std::make_unique<EvaluationNode>(
            begin, end, point,
            make_owned_polynomial(Polynomial{constant, 1U}, tracker));
    }

    const auto middle = begin + ((end - begin) >> 1U);
    auto left = build_evaluation_tree(
        ring, polynomial_ledger, width, begin, middle, atomic, tracker);
    auto right = build_evaluation_tree(
        ring, polynomial_ledger, width, middle, end, atomic, tracker);
    ++polynomial_ledger.product_tree_internal_nodes;
    auto product = multiply_polynomials(
        ring, left->product, right->product, tracker);

    // A leaf's degree-one product is needed only to form its parent product.
    // Leaf observation evaluates the already reduced parent polynomial at the
    // stored point directly, so retaining these polynomials until descent is
    // both unnecessary space and unnecessary remainder work.
    if (left->end - left->begin == 1U) left->product.reset();
    if (right->end - right->begin == 1U) right->product.reset();

    auto node = std::make_unique<EvaluationNode>(
        begin, end, 0U, std::move(product));
    node->left = std::move(left);
    node->right = std::move(right);
    return node;
}

[[nodiscard]] std::unique_ptr<EvaluationNode>
build_evaluation_tree_from_materialized_points(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const OwnedScalarBuffer& points,
    const std::uint64_t begin,
    const std::uint64_t end,
    LiveCoefficientTracker& tracker) {
    if (begin >= end)
        throw std::invalid_argument("empty materialized evaluation range");
    tracker.node_allocation();
    if (end - begin == 1U) {
        const auto point = points[begin];
        const auto constant = point == 0U ? 0U : ring.modulus() - point;
        return std::make_unique<EvaluationNode>(
            begin, end, point,
            make_owned_polynomial(Polynomial{constant, 1U}, tracker));
    }

    const auto middle = begin + ((end - begin) >> 1U);
    auto left = build_evaluation_tree_from_materialized_points(
        ring, polynomial_ledger, points, begin, middle, tracker);
    auto right = build_evaluation_tree_from_materialized_points(
        ring, polynomial_ledger, points, middle, end, tracker);
    ++polynomial_ledger.product_tree_internal_nodes;
    auto product = multiply_polynomials(
        ring, left->product, right->product, tracker);
    auto node = std::make_unique<EvaluationNode>(
        begin, end, 0U, std::move(product));
    node->left = std::move(left);
    node->right = std::move(right);
    return node;
}

void multiply_scalar_into(
    std::uint64_t& residue,
    const std::uint64_t value,
    const std::uint64_t modulus,
    AtomicOperationLedger& atomic) {
    residue = frozen::wilson::multiply_mod(residue, value, modulus);
    ++atomic.scalar_modular_multiplications;
}

void descend_evaluation_tree(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const PolynomialPolicy& policy,
    EvaluationNode& node,
    OwnedPolynomial reduced,
    std::uint64_t& residue,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    node.product.reset();
    if (node.end - node.begin == 1U) {
        const auto value = evaluate_polynomial(
            reduced, node.point, ring.modulus(), polynomial_ledger);
        multiply_scalar_into(residue, value, ring.modulus(), atomic);
        return;
    }

    ++polynomial_ledger.remainder_tree_internal_nodes;

    const auto consume_left = [&] {
        if (node.left->end - node.left->begin == 1U) {
            const auto value = evaluate_polynomial(
                reduced, node.left->point, ring.modulus(), polynomial_ledger);
            multiply_scalar_into(residue, value, ring.modulus(), atomic);
            node.left.reset();
            return;
        }
        auto left_remainder = remainder_polynomial(
            ring, polynomial_ledger, policy, reduced, node.left->product,
            atomic, tracker);
        descend_evaluation_tree(
            ring, polynomial_ledger, policy, *node.left,
            std::move(left_remainder), residue, atomic, tracker);
        node.left.reset();
    };
    consume_left();

    if (node.right->end - node.right->begin == 1U) {
        const auto value = evaluate_polynomial(
            reduced, node.right->point, ring.modulus(), polynomial_ledger);
        reduced.reset();
        multiply_scalar_into(residue, value, ring.modulus(), atomic);
        node.right.reset();
        return;
    }

    auto right_remainder = remainder_polynomial(
        ring, polynomial_ledger, policy, reduced, node.right->product,
        atomic, tracker);
    reduced.reset();
    descend_evaluation_tree(
        ring, polynomial_ledger, policy, *node.right,
        std::move(right_remainder), residue, atomic, tracker);
    node.right.reset();
}

void descend_evaluation_tree_materialized(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const PolynomialPolicy& policy,
    const EvaluationNode& node,
    const OwnedPolynomial& polynomial,
    OwnedScalarBuffer& values,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    if (node.end - node.begin == 1U) {
        values[node.begin] = evaluate_polynomial_legacy(
            polynomial, node.point, ring.modulus(), polynomial_ledger);
        return;
    }

    ++polynomial_ledger.remainder_tree_internal_nodes;
    auto left_remainder = remainder_polynomial(
        ring, polynomial_ledger, policy, polynomial, node.left->product,
        atomic, tracker);
    auto right_remainder = remainder_polynomial(
        ring, polynomial_ledger, policy, polynomial, node.right->product,
        atomic, tracker);
    descend_evaluation_tree_materialized(
        ring, polynomial_ledger, policy, *node.left, left_remainder,
        values, atomic, tracker);
    descend_evaluation_tree_materialized(
        ring, polynomial_ledger, policy, *node.right, right_remainder,
        values, atomic, tracker);
}

void evaluate_range_streamed(
    PolynomialRing& ring,
    PolynomialLedger& polynomial_ledger,
    const PolynomialPolicy& policy,
    const OwnedPolynomial& block_polynomial,
    const std::uint64_t width,
    const std::uint64_t begin,
    const std::uint64_t end,
    std::uint64_t& residue,
    AtomicOperationLedger& atomic,
    LiveCoefficientTracker& tracker) {
    if (begin >= end) return;
    auto tree = build_evaluation_tree(
        ring, polynomial_ledger, width, begin, end, atomic, tracker);
    if (end - begin == 1U) {
        const auto value = evaluate_polynomial(
            block_polynomial, tree->point, ring.modulus(), polynomial_ledger);
        multiply_scalar_into(residue, value, ring.modulus(), atomic);
        tree.reset();
        return;
    }

    auto reduced = remainder_polynomial(
        ring, polynomial_ledger, policy, block_polynomial, tree->product,
        atomic, tracker);
    descend_evaluation_tree(
        ring, polynomial_ledger, policy, *tree, std::move(reduced),
        residue, atomic, tracker);
    tree.reset();
}

[[nodiscard]] std::uint64_t scratch_coefficient_upper_bound(
    const PolynomialLedger& ledger) {
    const auto maximum_polynomial = ledger.maximum_polynomial_coefficients;
    if (ledger.maximum_ntt_length == 0U) {
        return checked_multiply(maximum_polynomial, 4U);
    }
    const auto primes = std::max<std::uint64_t>(1U, ledger.maximum_crt_primes);
    const auto residue_storage = checked_multiply(primes, maximum_polynomial);
    const auto transforms = checked_multiply(
        checked_multiply(primes, ledger.maximum_ntt_length), 2U);
    return checked_add(
        checked_add(residue_storage, transforms),
        checked_multiply(maximum_polynomial, 4U));
}

[[nodiscard]] std::uint64_t deterministic_work_units(
    const PolynomialLedger& polynomial,
    const AtomicOperationLedger& atomic) {
    std::uint64_t work{};
    for (const auto value : {
             polynomial.polynomial_multiplications,
             polynomial.schoolbook_coefficient_products,
             polynomial.ntt_butterflies,
             polynomial.crt_mixed_radix_digits,
             polynomial.monic_remainders,
             polynomial.horner_coefficient_steps,
             polynomial.product_tree_internal_nodes,
             polynomial.remainder_tree_internal_nodes,
             atomic.classical_remainder_products,
             atomic.scalar_modular_multiplications,
             atomic.point_generation_updates}) {
        checked_accumulate(work, value);
    }
    return work;
}

void populate_atomic_counts(
    prime::JointWilsonLedger& output,
    const PolynomialLedger& polynomial,
    const AtomicOperationLedger& atomic) {
    const auto butterfly_twice = checked_multiply(
        polynomial.ntt_butterflies, 2U);
    const auto crt_twice = checked_multiply(
        polynomial.crt_mixed_radix_digits, 2U);

    output.ring_multiplications = 0U;
    for (const auto value : {
             polynomial.schoolbook_coefficient_products,
             butterfly_twice,
             polynomial.horner_coefficient_steps,
             crt_twice,
             atomic.classical_remainder_products,
             atomic.scalar_modular_multiplications,
             atomic.point_generation_updates}) {
        checked_accumulate(output.ring_multiplications, value);
    }

    output.ring_additions = 0U;
    for (const auto value : {
             polynomial.schoolbook_coefficient_products,
             butterfly_twice,
             polynomial.horner_coefficient_steps,
             crt_twice,
             atomic.classical_remainder_additions,
             atomic.scalar_modular_additions}) {
        checked_accumulate(output.ring_additions, value);
    }
    output.modular_reductions = output.ring_multiplications;
    output.coefficient_updates = 0U;
    for (const auto value : {
             polynomial.schoolbook_coefficient_products,
             butterfly_twice,
             polynomial.horner_coefficient_steps,
             polynomial.crt_mixed_radix_digits,
             atomic.classical_remainder_updates,
             atomic.point_generation_updates}) {
        checked_accumulate(output.coefficient_updates, value);
    }
    output.limb_products = output.ring_multiplications;
    output.limb_additions = output.ring_additions;
    output.deterministic_work_units =
        deterministic_work_units(polynomial, atomic);
}

[[nodiscard]] JointWilsonExecution execute_legacy_wilson_projection_instrumented(
    const std::uint64_t factor_count,
    const std::uint64_t modulus,
    const prime::JointWilsonPolicy& policy) {
    if (modulus < 2U)
        throw std::invalid_argument("Wilson modulus must be at least two");
    if (factor_count != modulus - 1U)
        throw std::invalid_argument(
            "legacy factor count and modulus are inconsistent");
    if (factor_count == 0U)
        throw std::invalid_argument("legacy Wilson factor count is empty");
    if (policy.maximum_ntt_length == 0U ||
        !std::has_single_bit(policy.maximum_ntt_length))
        throw std::invalid_argument("maximum NTT length must be a power of two");

    JointWilsonExecution output{};
    auto& coordinate = output.coordinate;
    auto& ledger = output.ledger;
    coordinate.original_factor_count = factor_count;
    coordinate.reduced_factor_count = factor_count;
    coordinate.legacy_block_width =
        frozen::wilson::ceil_square_root(factor_count);
    coordinate.optimized_block_width = coordinate.legacy_block_width;
    if (coordinate.legacy_block_width > policy.maximum_block_width)
        throw frozen::wilson::PolynomialResourceError(
            "legacy block width exceeds policy limit");
    coordinate.full_blocks = factor_count / coordinate.legacy_block_width;
    coordinate.tail_factors = factor_count % coordinate.legacy_block_width;
    coordinate.logical_evaluation_points = coordinate.full_blocks;

    ledger.ordinary_projection_started = true;
    ledger.ordinary_feedback = false;
    ledger.full_factorial_materialized = false;
    ledger.native_state_nodes_rewritten = 0U;
    ledger.temporary_big_integer_count = 0U;

    PolynomialPolicy polynomial_policy{};
    polynomial_policy.schoolbook_product_limit =
        policy.schoolbook_product_limit;
    polynomial_policy.maximum_ntt_length = policy.maximum_ntt_length;
    polynomial_policy.minimum_crt_primes = policy.minimum_crt_primes;
    polynomial_policy.force_schoolbook = policy.force_schoolbook;
    polynomial_policy.parallel_ntt_primes = policy.parallel_ntt_primes;

    PolynomialLedger polynomial_ledger{};
    AtomicOperationLedger atomic{};
    LiveCoefficientTracker tracker{};
    PolynomialRing ring{modulus, polynomial_ledger, polynomial_policy};

    const auto start = std::chrono::steady_clock::now();
    auto block_polynomial = build_factor_polynomial(
        ring, polynomial_ledger, 0U, coordinate.legacy_block_width,
        tracker);
    coordinate.materialized_block_coefficients = block_polynomial.size();

    OwnedScalarBuffer points{coordinate.full_blocks, tracker};
    OwnedScalarBuffer block_values{coordinate.full_blocks, tracker};
    coordinate.materialized_evaluation_point_array = points.size();
    coordinate.materialized_block_value_array = block_values.size();
    coordinate.materialized_coordinate_count = checked_add(
        coordinate.materialized_block_coefficients,
        checked_multiply(coordinate.full_blocks, 2U));
    coordinate.legacy_materialized_coordinate_count =
        coordinate.materialized_coordinate_count;

    checked_accumulate(
        atomic.copied_bytes_upper_bound,
        checked_multiply(
            checked_multiply(coordinate.full_blocks, 2U),
            sizeof(std::uint64_t)));
    for (std::uint64_t block = 0U;
         block < coordinate.full_blocks; ++block) {
        points[block] = static_cast<std::uint64_t>(
            (static_cast<WideUnsigned>(block) *
             coordinate.legacy_block_width) % modulus);
        ++atomic.point_generation_updates;
    }

    auto tree = build_evaluation_tree_from_materialized_points(
        ring, polynomial_ledger, points, 0U, coordinate.full_blocks,
        tracker);
    descend_evaluation_tree_materialized(
        ring, polynomial_ledger, polynomial_policy, *tree,
        block_polynomial, block_values, atomic, tracker);

    std::uint64_t residue = 1U % modulus;
    for (const auto value : block_values)
        multiply_scalar_into(residue, value, modulus, atomic);
    const auto covered = checked_multiply(
        coordinate.full_blocks, coordinate.legacy_block_width);
    for (std::uint64_t factor = covered + 1U;
         factor <= factor_count; ++factor)
        multiply_scalar_into(residue, factor % modulus, modulus, atomic);

    const auto stop = std::chrono::steady_clock::now();
    output.residue = residue;
    ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop - start).count());
    ledger.ordinary_projection_completed = true;

    ledger.polynomial_multiplications =
        polynomial_ledger.polynomial_multiplications;
    ledger.schoolbook_coefficient_products =
        polynomial_ledger.schoolbook_coefficient_products;
    ledger.ntt_butterflies = polynomial_ledger.ntt_butterflies;
    ledger.crt_mixed_radix_digits =
        polynomial_ledger.crt_mixed_radix_digits;
    ledger.monic_remainders = polynomial_ledger.monic_remainders;
    ledger.horner_coefficient_steps =
        polynomial_ledger.horner_coefficient_steps;
    ledger.maximum_polynomial_coefficients =
        polynomial_ledger.maximum_polynomial_coefficients;
    ledger.maximum_ntt_length = polynomial_ledger.maximum_ntt_length;
    ledger.temporary_polynomial_count = tracker.polynomial_objects();
    ledger.allocation_count = tracker.allocations();
    ledger.copied_bytes_upper_bound = atomic.copied_bytes_upper_bound;
    ledger.peak_live_coefficients = tracker.peak();
    ledger.peak_live_coefficients_upper_bound = checked_add(
        tracker.peak(), scratch_coefficient_upper_bound(polynomial_ledger));
    ledger.peak_live_limbs = ledger.peak_live_coefficients_upper_bound;
    ledger.materialized_coordinate_count =
        coordinate.materialized_coordinate_count;
    populate_atomic_counts(ledger, polynomial_ledger, atomic);

    const bool saturated =
        polynomial_ledger.schoolbook_coefficient_products ==
            std::numeric_limits<std::uint64_t>::max();
    ledger.exact_counters_checked = !saturated;
    if (saturated)
        throw std::overflow_error("legacy polynomial operation counter saturated");
    return output;
}

[[nodiscard]] std::uint64_t direct_factorial_mod(
    const std::uint64_t factor_count,
    const std::uint64_t modulus) noexcept {
    std::uint64_t residue = 1U % modulus;
    for (std::uint64_t factor = 1U; factor <= factor_count; ++factor)
        residue = frozen::wilson::multiply_mod(
            residue, factor % modulus, modulus);
    return residue;
}

} // namespace

JointWilsonExecution execute_joint_wilson_projection(
    const std::uint64_t factor_count,
    const std::uint64_t modulus,
    const prime::JointWilsonPolicy& policy) {
    if (modulus < 2U)
        throw std::invalid_argument("Wilson modulus must be at least two");
    if (factor_count != modulus - 1U)
        throw std::invalid_argument(
            "native factorial coordinate and modulus are inconsistent");
    if (policy.maximum_ntt_length == 0U ||
        !std::has_single_bit(policy.maximum_ntt_length))
        throw std::invalid_argument("maximum NTT length must be a power of two");

    JointWilsonExecution output{};
    auto& coordinate = output.coordinate;
    auto& ledger = output.ledger;
    coordinate.original_factor_count = factor_count;
    coordinate.reduced_factor_count = factor_count / 2U;
    coordinate.legacy_block_width =
        frozen::wilson::ceil_square_root(factor_count);
    const auto legacy_blocks = factor_count / coordinate.legacy_block_width;
    coordinate.legacy_materialized_coordinate_count = checked_add(
        coordinate.legacy_block_width + 1U,
        checked_multiply(legacy_blocks, 2U));

    ledger.ordinary_projection_started = true;
    ledger.ordinary_feedback = false;
    ledger.full_factorial_materialized = false;
    ledger.native_state_nodes_rewritten = 0U;
    ledger.temporary_big_integer_count = 0U;

    const auto half_count = coordinate.reduced_factor_count;
    std::uint64_t half_residue = 1U % modulus;
    PolynomialLedger polynomial_ledger{};
    AtomicOperationLedger atomic{};
    LiveCoefficientTracker tracker{};

    const auto start = std::chrono::steady_clock::now();
    if (half_count != 0U) {
        const auto width = choose_block_width(
            half_count, policy.width_search_radius);
        if (width > policy.maximum_block_width)
            throw frozen::wilson::PolynomialResourceError(
                "optimized block width exceeds policy limit");

        coordinate.optimized_block_width = width;
        coordinate.full_blocks = half_count / width;
        coordinate.tail_factors = half_count % width;
        coordinate.logical_evaluation_points = coordinate.full_blocks;

        PolynomialPolicy polynomial_policy{};
        polynomial_policy.schoolbook_product_limit =
            policy.schoolbook_product_limit;
        polynomial_policy.maximum_ntt_length = policy.maximum_ntt_length;
        polynomial_policy.minimum_crt_primes = policy.minimum_crt_primes;
        polynomial_policy.force_schoolbook = policy.force_schoolbook;
        polynomial_policy.parallel_ntt_primes = policy.parallel_ntt_primes;

        PolynomialRing ring{modulus, polynomial_ledger, polynomial_policy};
        {
            auto block_polynomial = build_factor_polynomial(
                ring, polynomial_ledger, 0U, width, tracker);
            coordinate.materialized_block_coefficients =
                block_polynomial.size();
            coordinate.materialized_evaluation_point_array = 0U;
            coordinate.materialized_block_value_array = 0U;
            coordinate.materialized_coordinate_count =
                coordinate.materialized_block_coefficients;

            if (coordinate.full_blocks != 0U) {
                multiply_scalar_into(
                    half_residue, block_polynomial.value().front(),
                    modulus, atomic);
            }

            const auto remaining_begin = std::uint64_t{1U};
            const auto remaining_end = coordinate.full_blocks;
            if (remaining_begin < remaining_end) {
                const auto middle = remaining_begin +
                    ((remaining_end - remaining_begin) >> 1U);
                evaluate_range_streamed(
                    ring, polynomial_ledger, polynomial_policy,
                    block_polynomial, width, remaining_begin, middle,
                    half_residue, atomic, tracker);
                evaluate_range_streamed(
                    ring, polynomial_ledger, polynomial_policy,
                    block_polynomial, width, middle, remaining_end,
                    half_residue, atomic, tracker);
            } else if (remaining_begin == 1U &&
                       coordinate.full_blocks == 2U) {
                evaluate_range_streamed(
                    ring, polynomial_ledger, polynomial_policy,
                    block_polynomial, width, 1U, 2U,
                    half_residue, atomic, tracker);
            }

            const auto covered = checked_multiply(
                coordinate.full_blocks, width);
            for (std::uint64_t factor = covered + 1U;
                 factor <= half_count; ++factor) {
                multiply_scalar_into(
                    half_residue, factor % modulus, modulus, atomic);
            }
        }
    } else {
        coordinate.optimized_block_width = 0U;
        coordinate.full_blocks = 0U;
        coordinate.tail_factors = 0U;
        coordinate.logical_evaluation_points = 0U;
        coordinate.materialized_block_coefficients = 0U;
        coordinate.materialized_evaluation_point_array = 0U;
        coordinate.materialized_block_value_array = 0U;
        coordinate.materialized_coordinate_count = 0U;
    }

    std::uint64_t residue = frozen::wilson::multiply_mod(
        half_residue, half_residue, modulus);
    ++atomic.scalar_modular_multiplications;
    if ((half_count & 1U) != 0U) {
        residue = residue == 0U ? 0U : modulus - residue;
        ++atomic.scalar_modular_additions;
    }
    if ((factor_count & 1U) != 0U) {
        residue = frozen::wilson::multiply_mod(
            residue, (half_count + 1U) % modulus, modulus);
        ++atomic.scalar_modular_multiplications;
    }

    const auto stop = std::chrono::steady_clock::now();
    output.residue = residue;
    ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop - start).count());
    ledger.ordinary_projection_completed = true;

    ledger.polynomial_multiplications =
        polynomial_ledger.polynomial_multiplications;
    ledger.schoolbook_coefficient_products =
        polynomial_ledger.schoolbook_coefficient_products;
    ledger.ntt_butterflies = polynomial_ledger.ntt_butterflies;
    ledger.crt_mixed_radix_digits =
        polynomial_ledger.crt_mixed_radix_digits;
    ledger.monic_remainders = polynomial_ledger.monic_remainders;
    ledger.horner_coefficient_steps =
        polynomial_ledger.horner_coefficient_steps;
    ledger.maximum_polynomial_coefficients =
        polynomial_ledger.maximum_polynomial_coefficients;
    ledger.maximum_ntt_length = polynomial_ledger.maximum_ntt_length;
    ledger.temporary_polynomial_count = tracker.polynomial_objects();
    ledger.allocation_count = tracker.allocations();
    ledger.copied_bytes_upper_bound = atomic.copied_bytes_upper_bound;
    ledger.peak_live_coefficients = tracker.peak();
    ledger.peak_live_coefficients_upper_bound = checked_add(
        tracker.peak(), scratch_coefficient_upper_bound(polynomial_ledger));
    ledger.peak_live_limbs = ledger.peak_live_coefficients_upper_bound;
    ledger.materialized_coordinate_count =
        coordinate.materialized_coordinate_count;
    populate_atomic_counts(ledger, polynomial_ledger, atomic);

    const bool saturated =
        polynomial_ledger.schoolbook_coefficient_products ==
            std::numeric_limits<std::uint64_t>::max();
    ledger.exact_counters_checked = !saturated;
    if (saturated)
        throw std::overflow_error("polynomial operation counter saturated");
    return output;
}

WilsonParetoAudit audit_joint_wilson_projection(
    const std::uint64_t factor_count,
    const std::uint64_t modulus,
    const prime::JointWilsonPolicy& policy) {
    if (factor_count != modulus - 1U)
        throw std::invalid_argument("factor count and modulus differ");

    const auto legacy = execute_legacy_wilson_projection_instrumented(
        factor_count, modulus, policy);
    const auto optimized = execute_joint_wilson_projection(
        factor_count, modulus, policy);
    const auto exact = direct_factorial_mod(factor_count, modulus);

    WilsonParetoAudit out{};
    out.factor_count = factor_count;
    out.modulus = modulus;
    out.legacy_residue = legacy.residue;
    out.optimized_residue = optimized.residue;
    out.exact_residue = exact;
    out.legacy_work_units = legacy.ledger.deterministic_work_units;
    out.optimized_work_units = optimized.ledger.deterministic_work_units;
    out.legacy_peak_live_coefficients =
        legacy.ledger.peak_live_coefficients_upper_bound;
    out.optimized_peak_live_coefficients =
        optimized.ledger.peak_live_coefficients_upper_bound;
    out.legacy_materialized_coordinate_count =
        legacy.coordinate.materialized_coordinate_count;
    out.optimized_materialized_coordinate_count =
        optimized.coordinate.materialized_coordinate_count;
    out.legacy_coordinate = legacy.coordinate;
    out.optimized_coordinate = optimized.coordinate;
    out.legacy_ledger = legacy.ledger;
    out.optimized_ledger = optimized.ledger;
    out.residues_equal =
        legacy.residue == optimized.residue && optimized.residue == exact;
    out.time_work_strictly_lower =
        optimized.ledger.deterministic_work_units <
        legacy.ledger.deterministic_work_units;
    out.peak_space_strictly_lower =
        optimized.ledger.peak_live_coefficients_upper_bound <
        legacy.ledger.peak_live_coefficients_upper_bound;
    out.coordinate_strictly_lower =
        optimized.coordinate.materialized_coordinate_count <
        legacy.coordinate.materialized_coordinate_count;
    out.ring_additions_strictly_lower =
        optimized.ledger.ring_additions < legacy.ledger.ring_additions;
    out.ring_multiplications_strictly_lower =
        optimized.ledger.ring_multiplications <
        legacy.ledger.ring_multiplications;
    out.modular_reductions_strictly_lower =
        optimized.ledger.modular_reductions <
        legacy.ledger.modular_reductions;
    out.coefficient_updates_strictly_lower =
        optimized.ledger.coefficient_updates <
        legacy.ledger.coefficient_updates;
    out.peak_limbs_strictly_lower =
        optimized.ledger.peak_live_limbs < legacy.ledger.peak_live_limbs;
    return out;
}

} // namespace angel::detail

namespace angel::prime {

JointWilsonDownload operator|(
    const NativeFactorialView& public_view,
    const ProjectWilsonJointly operation) {
    const auto& model = detail::NativeFactorialAccess::model(public_view);
    const auto binding = detail::verify_native_binding(model.request);
    if (!binding.verified ||
        model.view_seal != detail::native_view_seal(model.request, binding))
        throw std::invalid_argument("native factorial view rejected");

    JointWilsonEvidence evidence{};
    evidence.factorial_argument = binding.factorial_argument;
    evidence.native_coefficient = binding.coefficient;
    evidence.native_result_seal = binding.result_seal;
    evidence.native_certificate_seal = binding.certificate_seal;
    evidence.native_coordinate_verified = binding.verified;
    evidence.factor_count_loaded_from_native_coordinate = true;
    evidence.candidate_used_only_as_modulus_and_consistency_check = true;
    evidence.complement_pairing_identity_verified = true;
    evidence.streamed_scalar_projection = true;
    evidence.square_root_coordinate_eliminated = false;
    evidence.full_factorial_materialized = false;
    evidence.ordinary_feedback = false;

    const auto make_limit = [&](const LimitReason reason,
                                const std::uint64_t required,
                                const std::uint64_t allowed,
                                JointWilsonCoordinate coordinate,
                                JointWilsonLedger ledger) {
        const auto verified = binding.verified;
        ObservationLedger public_ledger{};
        public_ledger.target_factor_count = binding.factorial_argument;
        public_ledger.block_width = coordinate.optimized_block_width;
        public_ledger.full_blocks = coordinate.full_blocks;
        public_ledger.tail_factors = coordinate.tail_factors;
        public_ledger.ordinary_projection_started =
            ledger.ordinary_projection_started;
        public_ledger.ordinary_projection_completed = false;
        public_ledger.ordinary_feedback = false;
        return JointWilsonDownload{
            ResourceLimit{
                model.request.candidate(), reason, required, allowed,
                public_ledger},
            coordinate,
            ledger,
            detail::state_integrity(
                model.request, binding.certificate_seal, verified),
            evidence,
            verified};
    };

    const auto required_width = binding.factorial_argument <= 1U
        ? 0U
        : detail::frozen::wilson::ceil_square_root(
              binding.factorial_argument / 2U);
    if (required_width > operation.policy.maximum_block_width) {
        JointWilsonCoordinate coordinate{};
        coordinate.original_factor_count = binding.factorial_argument;
        coordinate.reduced_factor_count = binding.factorial_argument / 2U;
        coordinate.legacy_block_width = detail::frozen::wilson::ceil_square_root(
            binding.factorial_argument);
        coordinate.optimized_block_width = required_width;
        JointWilsonLedger ledger{};
        ledger.ordinary_feedback = false;
        ledger.full_factorial_materialized = false;
        ledger.native_state_nodes_rewritten = 0U;
        ledger.exact_counters_checked = true;
        return make_limit(
            LimitReason::BlockWidth, required_width,
            operation.policy.maximum_block_width,
            coordinate, ledger);
    }

    try {
        auto execution = detail::execute_joint_wilson_projection(
            binding.factorial_argument,
            model.request.candidate(),
            operation.policy);
        const auto& native = model.request.factorial_execution();
        execution.ledger.input_bits =
            native.principal_jet.valuation().bit_length();
        execution.ledger.native_steps =
            native.r56_factorial.ledger.total_steps() +
            native.fusion_ledger.total_native_steps();
        execution.ledger.state_payload_bytes =
            native.principal_jet.payload_bytes();

        ObservationLedger public_ledger{};
        public_ledger.input_bits = execution.ledger.input_bits;
        public_ledger.native_steps = execution.ledger.native_steps;
        public_ledger.state_payload_bytes =
            execution.ledger.state_payload_bytes;
        public_ledger.target_factor_count = binding.factorial_argument;
        public_ledger.block_width =
            execution.coordinate.optimized_block_width;
        public_ledger.full_blocks = execution.coordinate.full_blocks;
        public_ledger.tail_factors = execution.coordinate.tail_factors;
        public_ledger.factor_leaf_materializations =
            execution.coordinate.materialized_block_coefficients == 0U
                ? 0U
                : execution.coordinate.materialized_block_coefficients - 1U;
        public_ledger.evaluation_points =
            execution.coordinate.logical_evaluation_points;
        public_ledger.block_value_multiplications =
            execution.coordinate.logical_evaluation_points;
        public_ledger.tail_multiplications =
            execution.coordinate.tail_factors;
        public_ledger.ordinary_elapsed_nanoseconds =
            execution.ledger.ordinary_elapsed_nanoseconds;
        public_ledger.polynomial_multiplications =
            execution.ledger.polynomial_multiplications;
        public_ledger.maximum_polynomial_coefficients =
            execution.ledger.maximum_polynomial_coefficients;
        public_ledger.maximum_transform_length =
            execution.ledger.maximum_ntt_length;
        public_ledger.ordinary_projection_started = true;
        public_ledger.ordinary_projection_completed = true;
        public_ledger.ordinary_feedback = false;

        const bool prime_result =
            execution.residue == model.request.candidate() - 1U;
        const bool verified =
            binding.verified && execution.ledger.exact_counters_checked &&
            execution.ledger.ordinary_projection_started &&
            execution.ledger.ordinary_projection_completed &&
            !execution.ledger.ordinary_feedback &&
            !execution.ledger.full_factorial_materialized &&
            execution.ledger.native_state_nodes_rewritten == 0U;
        return JointWilsonDownload{
            WilsonObservation{
                model.request.candidate(), execution.residue,
                prime_result, public_ledger},
            execution.coordinate,
            execution.ledger,
            detail::state_integrity(
                model.request, binding.certificate_seal, verified),
            evidence,
            verified};
    } catch (const detail::frozen::wilson::PolynomialResourceError&) {
        JointWilsonCoordinate coordinate{};
        coordinate.original_factor_count = binding.factorial_argument;
        coordinate.reduced_factor_count = binding.factorial_argument / 2U;
        coordinate.legacy_block_width = detail::frozen::wilson::ceil_square_root(
            binding.factorial_argument);
        coordinate.optimized_block_width = required_width;
        JointWilsonLedger ledger{};
        ledger.ordinary_projection_started = true;
        ledger.ordinary_feedback = false;
        ledger.full_factorial_materialized = false;
        ledger.native_state_nodes_rewritten = 0U;
        ledger.exact_counters_checked = true;
        return make_limit(
            LimitReason::PolynomialEngine, required_width,
            operation.policy.maximum_block_width,
            coordinate, ledger);
    }
}

} // namespace angel::prime
