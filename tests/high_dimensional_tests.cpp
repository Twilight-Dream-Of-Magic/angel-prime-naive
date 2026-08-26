#include "angel/high_dimensional.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

namespace {

using angel::high::ArithmeticContinuation;
using angel::high::ArithmeticResult;
using angel::high::Coefficient;
using angel::high::DivisionPacket;
using angel::high::DivisionResult;
using angel::high::MazeState;
using angel::high::SparseAxis;
using angel::high::StateResult;
using angel::high::TriClassValue;

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

[[nodiscard]] const TriClassValue& value(const ArithmeticResult& result) {
    require(std::holds_alternative<TriClassValue>(result),
            "unexpected arithmetic continuation");
    return std::get<TriClassValue>(result);
}

[[nodiscard]] const MazeState& state(const StateResult& result) {
    require(std::holds_alternative<MazeState>(result),
            "unexpected state continuation");
    return std::get<MazeState>(result);
}

[[nodiscard]] TriClassValue sample_a() {
    return TriClassValue::exact(
        2,
        SparseAxis{{angel::high::HistoryWord{1U}, 3}},
        SparseAxis{{angel::high::HistoryWord{9U}, 1}});
}

[[nodiscard]] TriClassValue sample_b() {
    return TriClassValue::exact(
        -1,
        SparseAxis{{angel::high::HistoryWord{2U}, 2}},
        SparseAxis{{angel::high::HistoryWord{8U}, -2}});
}

[[nodiscard]] TriClassValue sample_c() {
    return TriClassValue::exact(
        4,
        SparseAxis{{angel::high::HistoryWord{3U}, -1}},
        {});
}

void test_tri_class_axioms() {
    const auto a = sample_a();
    const auto b = sample_b();
    const auto c = sample_c();
    const auto zero = TriClassValue::ordinary(0);
    const auto one = TriClassValue::ordinary(1);

    require(value(angel::high::hadd(a, zero)) == a,
            "HADD identity failed");
    require(value(angel::high::hsub(a, a)).is_zero(),
            "HSUB inverse failed");
    require(value(angel::high::hmul(a, one)) == a,
            "HMUL identity failed");

    const auto ab = value(angel::high::hmul(a, b));
    const auto bc = value(angel::high::hmul(b, c));
    require(value(angel::high::hmul(ab, c)) ==
                value(angel::high::hmul(a, bc)),
            "HMUL associativity failed");

    const auto b_plus_c = value(angel::high::hadd(b, c));
    const auto left_distributive = value(angel::high::hmul(a, b_plus_c));
    const auto right_distributive = value(angel::high::hadd(
        value(angel::high::hmul(a, b)),
        value(angel::high::hmul(a, c))));
    require(left_distributive == right_distributive,
            "left distributivity failed");

    require(value(angel::high::hadd(a, b)).ordinary_coordinate() == 1,
            "ordinary HADD projection failed");
    require(value(angel::high::hsub(a, b)).ordinary_coordinate() == 3,
            "ordinary HSUB projection failed");
    require(value(angel::high::hmul(a, b)).ordinary_coordinate() == -2,
            "ordinary HMUL projection failed");

    const auto history_x = TriClassValue::history(11U);
    const auto history_y = TriClassValue::history(12U);
    require(value(angel::high::hmul(history_x, history_y)) !=
                value(angel::high::hmul(history_y, history_x)),
            "ordered histories were incorrectly commuted");

    const auto singular = TriClassValue::singular(21U);
    const auto hs = value(angel::high::hmul(history_x, singular));
    const auto sh = value(angel::high::hmul(singular, history_x));
    require(hs.history_coordinate().empty() &&
                !hs.singular_coordinate().empty() &&
                sh.history_coordinate().empty() &&
                !sh.singular_coordinate().empty(),
            "singular sector is not a two-sided ideal");

    const auto overflow = angel::high::hadd(
        TriClassValue::ordinary(std::numeric_limits<Coefficient>::max()),
        TriClassValue::ordinary(1));
    require(std::holds_alternative<ArithmeticContinuation>(overflow) &&
                angel::high::verify(
                    std::get<ArithmeticContinuation>(overflow)),
            "coefficient overflow did not become a verified continuation");
}

void test_totalized_division() {
    const auto numerator = TriClassValue::exact(
        17,
        SparseAxis{{angel::high::HistoryWord{4U}, 9}},
        SparseAxis{{angel::high::HistoryWord{7U}, -5}});
    const auto denominator = TriClassValue::ordinary(4);
    const DivisionResult divided = angel::high::hdiv(numerator, denominator);
    require(std::holds_alternative<DivisionPacket>(divided),
            "central scalar HDIV did not return a packet");
    const auto& packet = std::get<DivisionPacket>(divided);
    require(packet.reconstruction_verified && angel::high::verify(packet),
            "HDIV reconstruction certificate failed");
    require(packet.quotient.ordinary_coordinate() == 4 &&
                packet.residual.unresolved.ordinary_coordinate() == 1,
            "HDIV scalar quotient/remainder is wrong");

    const auto by_zero = angel::high::hdiv(
        numerator, TriClassValue::ordinary(0));
    require(std::holds_alternative<DivisionPacket>(by_zero),
            "division by zero did not totalize");
    const auto& zero_packet = std::get<DivisionPacket>(by_zero);
    require(zero_packet.quotient.is_zero() &&
                zero_packet.residual.unresolved == numerator &&
                zero_packet.reconstruction_verified,
            "division-by-zero payload did not preserve the numerator");

    const auto noncentral = angel::high::hdiv(
        numerator, TriClassValue::history(99U));
    require(std::holds_alternative<ArithmeticContinuation>(noncentral) &&
                angel::high::verify(
                    std::get<ArithmeticContinuation>(noncentral)),
            "noncentral division did not retain a continuation");
}

void test_maze_state_and_functor_laws() {
    const angel::high::UploadFunctor upload;
    const auto initial = upload(angel::high::OrdinarySpecification{6});

    const auto identity_result = angel::high::NativeFunctor::identity()(initial);
    require(state(identity_result).exactly_equal(initial),
            "identity functor law failed");

    const auto add_history = angel::high::NativeFunctor::add(
        TriClassValue::history(31U, 2));
    const auto multiply = angel::high::NativeFunctor::multiply(
        TriClassValue::ordinary(3));
    const auto composed = angel::high::compose(multiply, add_history);
    const auto composed_result = composed(initial);
    const auto first_result = add_history(initial);
    const auto sequential_result = multiply(state(first_result));
    require(state(composed_result).exactly_equal(state(sequential_result)),
            "functor composition law failed");

    const auto direct_sum = angel::high::hadd(initial, initial);
    require(state(direct_sum).value().ordinary_coordinate() == 12 &&
                state(direct_sum).address().history_depth == 1U,
            "maze-state HADD transition failed");

    auto mismatched_address = initial.address();
    mismatched_address.frame = 2U;
    const auto mismatched = angel::high::upload_complete(
        TriClassValue::ordinary(6), mismatched_address);
    const auto frame_result = angel::high::hmul(initial, mismatched);
    require(std::holds_alternative<ArithmeticContinuation>(frame_result),
            "frame mismatch did not become a continuation");

    const angel::high::DownloadFunctor download;
    const auto observation = download(state(composed_result));
    require(angel::high::validate_observation(
                state(composed_result), observation),
            "canonical derived observation was rejected");
    auto tampered = observation;
    ++tampered.history_depth;
    require(!angel::high::validate_observation(
                state(composed_result), tampered) &&
                state(composed_result).exactly_equal(state(composed_result)),
            "tampered observation was accepted or canonical state changed");
}

void test_class_quantum_history() {
    const angel::high::UploadFunctor upload;
    const auto initial = upload(angel::high::OrdinarySpecification{5});

    const auto shift = angel::high::NativeFunctor::add(
        TriClassValue::ordinary(1));
    const angel::high::ClassQuantumFunctor positive({
        angel::high::WeightedFunctor{1, 1U, shift}});
    const angel::high::ClassQuantumFunctor negative({
        angel::high::WeightedFunctor{-1, 2U, shift}});
    const auto superposed = angel::high::superpose(positive, negative)(initial);
    require(std::holds_alternative<angel::high::HistorySuperposition>(superposed),
            "class-quantum superposition failed");
    const auto interference = angel::high::interfere_structurally(
        std::get<angel::high::HistorySuperposition>(superposed));
    require(std::holds_alternative<angel::high::InterferenceResult>(interference),
            "structural interference returned a continuation");
    const auto& canceled = std::get<angel::high::InterferenceResult>(interference);
    require(canceled.survivors.branches.empty() &&
                canceled.canceled_branches == 2U &&
                canceled.canceled_endpoints == 1U,
            "opposite histories did not cancel");

    const angel::high::ClassQuantumFunctor same_shadow({
        angel::high::WeightedFunctor{
            1, 3U, angel::high::NativeFunctor::identity()},
        angel::high::WeightedFunctor{
            -1, 4U, angel::high::NativeFunctor::add(
                TriClassValue::history(77U))}});
    const auto distinct_packet = same_shadow(initial);
    require(std::holds_alternative<angel::high::HistorySuperposition>(
                distinct_packet),
            "same-shadow packet construction failed");
    const auto distinct_interference = angel::high::interfere_structurally(
        std::get<angel::high::HistorySuperposition>(distinct_packet));
    const auto& distinct = std::get<angel::high::InterferenceResult>(
        distinct_interference);
    require(distinct.survivors.branches.size() == 2U &&
                distinct.canceled_branches == 0U,
            "ordinary-shadow collision incorrectly canceled complete histories");

    const auto product = angel::high::tensor_product(
        std::vector<Coefficient>{1, 2},
        std::vector<Coefficient>{3, 4});
    require(std::holds_alternative<angel::high::TensorHistory>(product),
            "tensor product construction failed");
    const auto product_certificate = angel::high::analyze_entanglement(
        std::get<angel::high::TensorHistory>(product));
    require(std::holds_alternative<angel::high::EntanglementCertificate>(
                product_certificate) &&
                !std::get<angel::high::EntanglementCertificate>(
                    product_certificate).entangled,
            "separable tensor was marked entangled");

    const auto coupled = angel::high::TensorHistory::exact(
        2U, 2U, std::vector<Coefficient>{1, 0, 0, 1});
    const auto coupled_certificate =
        angel::high::analyze_entanglement(coupled);
    require(std::holds_alternative<angel::high::EntanglementCertificate>(
                coupled_certificate) &&
                std::get<angel::high::EntanglementCertificate>(
                    coupled_certificate).entangled &&
                std::get<angel::high::EntanglementCertificate>(
                    coupled_certificate).nonzero_minor == 1,
            "nonseparable coupled history lacks an exact minor certificate");

    const auto tensor_overflow = angel::high::tensor_product(
        std::vector<Coefficient>{std::numeric_limits<Coefficient>::max()},
        std::vector<Coefficient>{2});
    require(std::holds_alternative<angel::high::TensorContinuation>(
                tensor_overflow) &&
                std::get<angel::high::TensorContinuation>(
                    tensor_overflow).no_false_entanglement_claim,
            "tensor overflow produced a false entanglement claim");
}

} // namespace

int main() {
    static_assert(!std::is_default_constructible_v<MazeState>);
    static_assert(!std::is_constructible_v<
                  MazeState, angel::high::DerivedObservation>);

    test_tri_class_axioms();
    test_totalized_division();
    test_maze_state_and_functor_laws();
    test_class_quantum_history();

    std::cout << "HIGH_DIMENSIONAL_ARITHMETIC_TESTS=PASS\n";
    std::cout << "TRI_CLASS_AXIOM_MODEL=PASS\n";
    std::cout << "HADD_HSUB_HMUL_HDIV=PASS\n";
    std::cout << "FUNCTOR_IDENTITY_COMPOSITION=PASS\n";
    std::cout << "SUPERPOSITION_INTERFERENCE_CANCELLATION=PASS\n";
    std::cout << "ENTANGLEMENT_MINOR_CERTIFICATE=PASS\n";
    std::cout << "OBSERVATION_NON_AUTHORITY=PASS\n";
    return 0;
}
