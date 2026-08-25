#pragma once

#include "angel/afac58/wilson_boundary.hpp"
#include "angel/afac59/polynomial_ring.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace angel::afac59 {

struct SublinearWilsonPolicy final {
    std::uint64_t maximum_block_width{100'000U};
    std::uint64_t block_width_override{};
    PolynomialPolicy polynomial{};
    bool parallel_tree_branches{true};
    std::uint64_t parallel_tree_threshold{4096U};

    [[nodiscard]] static SublinearWilsonPolicy production(
        const std::uint64_t maximum_block_width = 100'000U) noexcept {
        SublinearWilsonPolicy policy{};
        policy.maximum_block_width = maximum_block_width;
        return policy;
    }
};

struct SublinearWilsonLedger final {
    std::uint64_t angel_input_bits{};
    std::uint64_t angel_native_steps{};
    std::uint64_t angel_state_payload_bytes{};
    std::uint64_t target_factor_count{};
    std::uint64_t block_width{};
    std::uint64_t full_blocks{};
    std::uint64_t covered_by_full_blocks{};
    std::uint64_t tail_factors{};
    std::uint64_t factor_leaf_materializations{};
    std::uint64_t evaluation_points{};
    std::uint64_t block_value_multiplications{};
    std::uint64_t tail_multiplications{};
    std::uint64_t ordinary_elapsed_nanoseconds{};
    std::uint64_t maximum_outer_tree_tasks{};
    PolynomialLedger polynomial{};
    bool ordinary_projection_started{};
    bool ordinary_projection_completed{};
    bool ordinary_result_fed_back_to_angel{};

    [[nodiscard]] std::uint64_t factor_scale_objects() const noexcept {
        return factor_leaf_materializations + evaluation_points + tail_factors;
    }

    friend bool operator==(const SublinearWilsonLedger&,
                           const SublinearWilsonLedger&) = default;
};

enum class SublinearLimitReason : std::uint8_t {
    BlockWidth,
    PolynomialEngine
};

class SublinearWilsonObservation final {
public:
    SublinearWilsonObservation() = delete;

    [[nodiscard]] std::uint64_t candidate() const noexcept { return candidate_; }
    [[nodiscard]] std::uint64_t residue() const noexcept { return residue_; }
    [[nodiscard]] bool is_prime() const noexcept { return prime_; }
    [[nodiscard]] const SublinearWilsonLedger& ledger() const noexcept {
        return ledger_;
    }
    [[nodiscard]] std::uint64_t seal() const noexcept { return seal_; }

private:
    std::uint64_t candidate_{};
    std::uint64_t residue_{};
    bool prime_{};
    SublinearWilsonLedger ledger_{};
    std::uint64_t request_binding_{};
    std::uint64_t seal_{};

    SublinearWilsonObservation(
        const std::uint64_t candidate, const std::uint64_t residue,
        const bool prime, SublinearWilsonLedger ledger,
        const std::uint64_t request_binding, const std::uint64_t seal)
        : candidate_(candidate), residue_(residue), prime_(prime),
          ledger_(std::move(ledger)), request_binding_(request_binding),
          seal_(seal) {}

    friend class SublinearWilsonObserver;
    friend class SublinearWilsonVerifier;
};

struct SublinearWilsonResourceLimit final {
    std::uint64_t candidate{};
    SublinearLimitReason reason{};
    std::uint64_t required_block_width{};
    std::uint64_t allowed_block_width{};
    std::uint64_t request_binding{};
    SublinearWilsonLedger ledger{};
    std::uint64_t seal{};

    friend bool operator==(const SublinearWilsonResourceLimit&,
                           const SublinearWilsonResourceLimit&) = default;
};

using SublinearWilsonDownload =
    std::variant<SublinearWilsonObservation, SublinearWilsonResourceLimit>;

[[nodiscard]] inline std::uint64_t polynomial_ledger_seal(
    const PolynomialLedger& ledger) noexcept {
    std::uint64_t state = 0x504f4c595235394cULL;
    const std::array<std::uint64_t, 19U> values{{
        ledger.polynomial_multiplications,
        ledger.schoolbook_convolutions,
        ledger.schoolbook_coefficient_products,
        ledger.ntt_convolutions,
        ledger.ntt_prime_transforms,
        ledger.ntt_butterflies,
        ledger.ntt_parallel_batches,
        ledger.maximum_ntt_workers,
        ledger.crt_output_coefficients,
        ledger.crt_mixed_radix_digits,
        ledger.crt_exact_bound_checks,
        ledger.maximum_crt_primes,
        ledger.monic_remainders,
        ledger.newton_inverse_rounds,
        ledger.horner_evaluations,
        ledger.horner_coefficient_steps,
        ledger.product_tree_internal_nodes,
        ledger.remainder_tree_internal_nodes,
        ledger.maximum_polynomial_coefficients ^
            std::rotl(ledger.maximum_ntt_length, 29),
    }};
    for (std::size_t index = 0U; index < values.size(); ++index)
        state = angel::afac56::mix64(
            state ^ std::rotl(values[index], static_cast<int>((index * 7U) & 63U)));
    return state;
}

[[nodiscard]] inline std::uint64_t sublinear_ledger_seal(
    const SublinearWilsonLedger& ledger) noexcept {
    std::uint64_t state = 0x5355424c494e3539ULL;
    const std::array<std::uint64_t, 18U> values{{
        ledger.angel_input_bits,
        ledger.angel_native_steps,
        ledger.angel_state_payload_bytes,
        ledger.target_factor_count,
        ledger.block_width,
        ledger.full_blocks,
        ledger.covered_by_full_blocks,
        ledger.tail_factors,
        ledger.factor_leaf_materializations,
        ledger.evaluation_points,
        ledger.block_value_multiplications,
        ledger.tail_multiplications,
        ledger.ordinary_elapsed_nanoseconds,
        ledger.maximum_outer_tree_tasks,
        polynomial_ledger_seal(ledger.polynomial),
        static_cast<std::uint64_t>(ledger.ordinary_projection_started),
        static_cast<std::uint64_t>(ledger.ordinary_projection_completed),
        static_cast<std::uint64_t>(ledger.ordinary_result_fed_back_to_angel),
    }};
    for (std::size_t index = 0U; index < values.size(); ++index)
        state = angel::afac56::mix64(
            state ^ std::rotl(values[index], static_cast<int>((index * 11U) & 63U)));
    return state;
}

[[nodiscard]] inline std::uint64_t sublinear_observation_seal(
    const std::uint64_t candidate, const std::uint64_t residue,
    const bool prime, const std::uint64_t request_binding,
    const SublinearWilsonLedger& ledger) noexcept {
    return angel::afac56::mix64(
        candidate ^ std::rotl(residue, 7) ^
        std::rotl(static_cast<std::uint64_t>(prime), 17) ^
        std::rotl(request_binding, 31) ^
        std::rotl(sublinear_ledger_seal(ledger), 43) ^
        0x4f4253523539534cULL);
}

[[nodiscard]] inline std::uint64_t sublinear_limit_seal(
    const SublinearWilsonResourceLimit& limit) noexcept {
    return angel::afac56::mix64(
        limit.candidate ^
        std::rotl(static_cast<std::uint64_t>(limit.reason), 9) ^
        std::rotl(limit.required_block_width, 19) ^
        std::rotl(limit.allowed_block_width, 29) ^
        std::rotl(limit.request_binding, 41) ^
        std::rotl(sublinear_ledger_seal(limit.ledger), 53) ^
        0x4c494d4954353953ULL);
}

[[nodiscard]] inline std::uint64_t ceil_square_root(
    const std::uint64_t value) noexcept {
    if (value <= 1U) return value;
    std::uint64_t low = 1U;
    std::uint64_t high = std::uint64_t{1U} << 32U;
    while (low < high) {
        const auto middle = low + ((high - low) >> 1U);
        if (static_cast<wide_uint>(middle) * middle >= value)
            high = middle;
        else
            low = middle + 1U;
    }
    return low;
}

namespace detail {

[[nodiscard]] inline Polynomial build_factor_polynomial(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const std::uint64_t begin, const std::uint64_t end) {
    if (end - begin == 1U)
        return Polynomial{(begin + 1U) % ring.modulus(), 1U};
    const auto middle = begin + ((end - begin) >> 1U);
    auto left = build_factor_polynomial(ring, ledger, begin, middle);
    auto right = build_factor_polynomial(ring, ledger, middle, end);
    ++ledger.product_tree_internal_nodes;
    return ring.multiply(left, right);
}

struct PolynomialBuildResult final {
    Polynomial polynomial{};
    PolynomialLedger ledger{};
};

[[nodiscard]] inline Polynomial build_factor_polynomial_outer_parallel(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const std::uint64_t modulus, const PolynomialPolicy polynomial_policy,
    const std::uint64_t width, const SublinearWilsonPolicy& policy,
    SublinearWilsonLedger& wilson_ledger) {
    if (!policy.parallel_tree_branches ||
        width < policy.parallel_tree_threshold || width < 2U)
        return build_factor_polynomial(ring, ledger, 0U, width);

    const auto middle = width >> 1U;
    wilson_ledger.maximum_outer_tree_tasks = std::max<std::uint64_t>(
        wilson_ledger.maximum_outer_tree_tasks, 2U);
    auto left_future = std::async(std::launch::async, [=] {
        PolynomialBuildResult result{};
        CompositeSafePolynomialRing local_ring{
            modulus, result.ledger, polynomial_policy};
        result.polynomial = build_factor_polynomial(
            local_ring, result.ledger, 0U, middle);
        return result;
    });
    PolynomialBuildResult right{};
    CompositeSafePolynomialRing right_ring{
        modulus, right.ledger, polynomial_policy};
    right.polynomial = build_factor_polynomial(
        right_ring, right.ledger, middle, width);
    auto left = left_future.get();
    merge_polynomial_ledger(ledger, left.ledger);
    merge_polynomial_ledger(ledger, right.ledger);
    ++ledger.product_tree_internal_nodes;
    return ring.multiply(left.polynomial, right.polynomial);
}

struct EvaluationNode final {
    std::size_t begin{};
    std::size_t end{};
    std::uint64_t point{};
    Polynomial product{};
    std::unique_ptr<EvaluationNode> left{};
    std::unique_ptr<EvaluationNode> right{};
};

[[nodiscard]] inline std::unique_ptr<EvaluationNode> build_evaluation_tree(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const std::vector<std::uint64_t>& points,
    const std::size_t begin, const std::size_t end) {
    auto node = std::make_unique<EvaluationNode>();
    node->begin = begin;
    node->end = end;
    if (end - begin == 1U) {
        node->point = points[begin];
        const auto constant = node->point == 0U
            ? 0U
            : ring.modulus() - node->point;
        node->product = Polynomial{constant, 1U};
        return node;
    }
    const auto middle = begin + ((end - begin) >> 1U);
    node->left = build_evaluation_tree(ring, ledger, points, begin, middle);
    node->right = build_evaluation_tree(ring, ledger, points, middle, end);
    ++ledger.product_tree_internal_nodes;
    node->product = ring.multiply(node->left->product, node->right->product);
    return node;
}

struct EvaluationBuildResult final {
    std::unique_ptr<EvaluationNode> node{};
    PolynomialLedger ledger{};
};

[[nodiscard]] inline std::unique_ptr<EvaluationNode>
build_evaluation_tree_outer_parallel(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const std::uint64_t modulus, const PolynomialPolicy polynomial_policy,
    const std::vector<std::uint64_t>& points,
    const SublinearWilsonPolicy& policy,
    SublinearWilsonLedger& wilson_ledger) {
    if (!policy.parallel_tree_branches ||
        points.size() < policy.parallel_tree_threshold || points.size() < 2U)
        return build_evaluation_tree(
            ring, ledger, points, 0U, points.size());

    const auto middle = points.size() >> 1U;
    wilson_ledger.maximum_outer_tree_tasks = std::max<std::uint64_t>(
        wilson_ledger.maximum_outer_tree_tasks, 2U);
    auto left_future = std::async(std::launch::async, [&points, modulus,
                                                       polynomial_policy,
                                                       middle] {
        EvaluationBuildResult result{};
        CompositeSafePolynomialRing local_ring{
            modulus, result.ledger, polynomial_policy};
        result.node = build_evaluation_tree(
            local_ring, result.ledger, points, 0U, middle);
        return result;
    });
    EvaluationBuildResult right{};
    CompositeSafePolynomialRing right_ring{
        modulus, right.ledger, polynomial_policy};
    right.node = build_evaluation_tree(
        right_ring, right.ledger, points, middle, points.size());
    auto left = left_future.get();

    auto root = std::make_unique<EvaluationNode>();
    root->begin = 0U;
    root->end = points.size();
    root->left = std::move(left.node);
    root->right = std::move(right.node);
    merge_polynomial_ledger(ledger, left.ledger);
    merge_polynomial_ledger(ledger, right.ledger);
    ++ledger.product_tree_internal_nodes;
    root->product = ring.multiply(root->left->product, root->right->product);
    return root;
}

inline void descend_remainder_tree(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const EvaluationNode& node, const Polynomial& polynomial,
    std::vector<std::uint64_t>& values) {
    if (node.end - node.begin == 1U) {
        values[node.begin] = ring.evaluate(polynomial, node.point);
        return;
    }
    ++ledger.remainder_tree_internal_nodes;
    auto left_remainder = ring.monic_remainder(polynomial, node.left->product);
    auto right_remainder = ring.monic_remainder(polynomial, node.right->product);
    descend_remainder_tree(
        ring, ledger, *node.left, left_remainder, values);
    descend_remainder_tree(
        ring, ledger, *node.right, right_remainder, values);
}

inline void descend_remainder_tree_outer_parallel(
    CompositeSafePolynomialRing& ring, PolynomialLedger& ledger,
    const std::uint64_t modulus, const PolynomialPolicy polynomial_policy,
    const EvaluationNode& node, const Polynomial& polynomial,
    std::vector<std::uint64_t>& values,
    const SublinearWilsonPolicy& policy,
    SublinearWilsonLedger& wilson_ledger) {
    if (!policy.parallel_tree_branches ||
        node.end - node.begin < policy.parallel_tree_threshold ||
        node.end - node.begin < 2U) {
        descend_remainder_tree(ring, ledger, node, polynomial, values);
        return;
    }

    ++ledger.remainder_tree_internal_nodes;
    wilson_ledger.maximum_outer_tree_tasks = std::max<std::uint64_t>(
        wilson_ledger.maximum_outer_tree_tasks, 2U);
    const auto branch = [&](const EvaluationNode& child) {
        PolynomialLedger local_ledger{};
        CompositeSafePolynomialRing local_ring{
            modulus, local_ledger, polynomial_policy};
        auto remainder = local_ring.monic_remainder(polynomial, child.product);
        descend_remainder_tree(
            local_ring, local_ledger, child, remainder, values);
        return local_ledger;
    };
    auto left_future = std::async(
        std::launch::async, [&] { return branch(*node.left); });
    auto right_future = std::async(
        std::launch::async, [&] { return branch(*node.right); });
    const auto left_ledger = left_future.get();
    const auto right_ledger = right_future.get();
    merge_polynomial_ledger(ledger, left_ledger);
    merge_polynomial_ledger(ledger, right_ledger);
}

[[nodiscard]] inline std::uint64_t factorial_mod_sublinear(
    const std::uint64_t factor_count, const std::uint64_t modulus,
    const SublinearWilsonPolicy& policy, SublinearWilsonLedger& ledger) {
    const auto natural_width = ceil_square_root(factor_count);
    const auto width = policy.block_width_override == 0U
        ? natural_width
        : policy.block_width_override;
    if (width == 0U || width > factor_count)
        throw std::invalid_argument("block width must lie in [1,factor_count]");
    if (width > policy.maximum_block_width)
        throw PolynomialResourceError("block width exceeds policy limit");

    ledger.block_width = width;
    ledger.full_blocks = factor_count / width;
    ledger.covered_by_full_blocks = ledger.full_blocks * width;
    ledger.tail_factors = factor_count - ledger.covered_by_full_blocks;
    ledger.factor_leaf_materializations = width;
    ledger.evaluation_points = ledger.full_blocks;

    CompositeSafePolynomialRing ring{
        modulus, ledger.polynomial, policy.polynomial};
    auto block_polynomial = build_factor_polynomial_outer_parallel(
        ring, ledger.polynomial, modulus, policy.polynomial, width,
        policy, ledger);

    std::vector<std::uint64_t> points(ledger.full_blocks, 0U);
    for (std::uint64_t block = 0U; block < ledger.full_blocks; ++block)
        points[block] = static_cast<std::uint64_t>(
            (static_cast<wide_uint>(block) * width) % modulus);

    std::vector<std::uint64_t> block_values(ledger.full_blocks, 0U);
    auto tree = build_evaluation_tree_outer_parallel(
        ring, ledger.polynomial, modulus, policy.polynomial, points,
        policy, ledger);
    descend_remainder_tree_outer_parallel(
        ring, ledger.polynomial, modulus, policy.polynomial, *tree,
        block_polynomial, block_values, policy, ledger);

    std::uint64_t residue = 1U % modulus;
    for (const auto value : block_values) {
        residue = multiply_mod(residue, value, modulus);
        ++ledger.block_value_multiplications;
    }
    for (std::uint64_t factor = ledger.covered_by_full_blocks + 1U;
         factor <= factor_count; ++factor) {
        residue = multiply_mod(residue, factor % modulus, modulus);
        ++ledger.tail_multiplications;
    }
    return residue;
}

} // namespace detail

class SublinearWilsonObserver final {
public:
    [[nodiscard]] SublinearWilsonDownload download(
        const angel::afac58::CertifiedWilsonRequest& request,
        const SublinearWilsonPolicy policy =
            SublinearWilsonPolicy::production()) const {
        if (!angel::afac58::WilsonRequestVerifier::verify(request).accepted)
            throw std::invalid_argument("R59 request failed R58 binding replay");

        SublinearWilsonLedger ledger{};
        const auto& execution = request.factorial_execution();
        ledger.angel_input_bits = execution.principal_jet.valuation().bit_length();
        ledger.angel_native_steps =
            execution.r56_factorial.ledger.total_steps() +
            execution.fusion_ledger.total_native_steps();
        ledger.angel_state_payload_bytes =
            execution.principal_jet.payload_bytes();
        ledger.target_factor_count = request.candidate() - 1U;
        ledger.ordinary_result_fed_back_to_angel = false;

        const auto required_width = ceil_square_root(ledger.target_factor_count);
        if (policy.block_width_override == 0U &&
            required_width > policy.maximum_block_width)
            return make_limit(
                request, SublinearLimitReason::BlockWidth, required_width,
                policy.maximum_block_width, std::move(ledger));
        if (policy.block_width_override > policy.maximum_block_width)
            return make_limit(
                request, SublinearLimitReason::BlockWidth,
                policy.block_width_override, policy.maximum_block_width,
                std::move(ledger));

        ledger.ordinary_projection_started = true;
        const auto start = std::chrono::steady_clock::now();
        try {
            const auto residue = detail::factorial_mod_sublinear(
                ledger.target_factor_count, request.candidate(), policy, ledger);
            const auto stop = std::chrono::steady_clock::now();
            ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    stop - start).count());
            ledger.ordinary_projection_completed = true;
            const bool prime = residue == request.candidate() - 1U;
            const auto seal = sublinear_observation_seal(
                request.candidate(), residue, prime, request.binding_seal(), ledger);
            return SublinearWilsonObservation{
                request.candidate(), residue, prime, std::move(ledger),
                request.binding_seal(), seal};
        } catch (const PolynomialResourceError&) {
            const auto stop = std::chrono::steady_clock::now();
            ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    stop - start).count());
            return make_limit(
                request, SublinearLimitReason::PolynomialEngine,
                ledger.block_width == 0U ? required_width : ledger.block_width,
                policy.maximum_block_width, std::move(ledger));
        }
    }

private:
    [[nodiscard]] static SublinearWilsonResourceLimit make_limit(
        const angel::afac58::CertifiedWilsonRequest& request,
        const SublinearLimitReason reason,
        const std::uint64_t required_width,
        const std::uint64_t allowed_width,
        SublinearWilsonLedger ledger) {
        SublinearWilsonResourceLimit limit{};
        limit.candidate = request.candidate();
        limit.reason = reason;
        limit.required_block_width = required_width;
        limit.allowed_block_width = allowed_width;
        limit.request_binding = request.binding_seal();
        limit.ledger = std::move(ledger);
        limit.seal = sublinear_limit_seal(limit);
        return limit;
    }
};

struct SublinearWilsonVerification final {
    bool request_valid{};
    bool binding_valid{};
    bool coverage_valid{};
    bool boundary_valid{};
    bool decision_matches_wilson{};
    bool seal_valid{};
    bool accepted{};
};

class SublinearWilsonVerifier final {
public:
    [[nodiscard]] static SublinearWilsonVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const SublinearWilsonObservation& observation) noexcept {
        SublinearWilsonVerification out{};
        out.request_valid =
            angel::afac58::WilsonRequestVerifier::verify(request).accepted;
        out.binding_valid = observation.candidate_ == request.candidate() &&
                            observation.request_binding_ == request.binding_seal();
        const auto& ledger = observation.ledger_;
        const auto covered = static_cast<wide_uint>(ledger.block_width) *
                             ledger.full_blocks + ledger.tail_factors;
        out.coverage_valid =
            covered == ledger.target_factor_count &&
            ledger.target_factor_count == request.candidate() - 1U &&
            ledger.covered_by_full_blocks ==
                ledger.block_width * ledger.full_blocks &&
            ledger.factor_leaf_materializations == ledger.block_width &&
            ledger.evaluation_points == ledger.full_blocks &&
            ledger.block_value_multiplications == ledger.full_blocks &&
            ledger.tail_multiplications == ledger.tail_factors;
        out.boundary_valid = ledger.ordinary_projection_started &&
                             ledger.ordinary_projection_completed &&
                             !ledger.ordinary_result_fed_back_to_angel;
        out.decision_matches_wilson =
            observation.residue_ < observation.candidate_ &&
            observation.prime_ ==
                (observation.residue_ == observation.candidate_ - 1U);
        out.seal_valid = observation.seal_ == sublinear_observation_seal(
            observation.candidate_, observation.residue_, observation.prime_,
            observation.request_binding_, observation.ledger_);
        out.accepted = out.request_valid && out.binding_valid &&
                       out.coverage_valid && out.boundary_valid &&
                       out.decision_matches_wilson && out.seal_valid;
        return out;
    }

    [[nodiscard]] static bool verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const SublinearWilsonResourceLimit& limit) noexcept {
        const bool reason_valid =
            (limit.reason == SublinearLimitReason::BlockWidth &&
             !limit.ledger.ordinary_projection_started &&
             limit.required_block_width > limit.allowed_block_width) ||
            (limit.reason == SublinearLimitReason::PolynomialEngine &&
             limit.ledger.ordinary_projection_started);
        return angel::afac58::WilsonRequestVerifier::verify(request).accepted &&
               limit.candidate == request.candidate() &&
               limit.request_binding == request.binding_seal() &&
               !limit.ledger.ordinary_projection_completed &&
               !limit.ledger.ordinary_result_fed_back_to_angel &&
               reason_valid &&
               limit.seal == sublinear_limit_seal(limit);
    }
};

struct R59ApplicationResult final {
    angel::afac58::CertifiedWilsonRequest request;
    SublinearWilsonDownload download;
};

[[nodiscard]] inline R59ApplicationResult ANGEL_WILSON_PRIME_SUBLINEAR(
    const std::uint64_t candidate,
    const SublinearWilsonPolicy policy =
        SublinearWilsonPolicy::production()) {
    angel::afac58::WilsonBoundary boundary;
    SublinearWilsonObserver observer;
    auto request = boundary.bind(candidate);
    auto download = observer.download(request, policy);
    return R59ApplicationResult{std::move(request), std::move(download)};
}

struct AngelRunningState final {};
AngelRunningState feed_sublinear_wilson_back(
    const SublinearWilsonObservation&) = delete;
SublinearWilsonObservation implicit_sublinear_download(
    const angel::afac58::CertifiedWilsonRequest&) = delete;

static_assert(!std::is_default_constructible_v<SublinearWilsonObservation>);
static_assert(!std::is_convertible_v<SublinearWilsonObservation,
                                     AngelRunningState>);

} // namespace angel::afac59
