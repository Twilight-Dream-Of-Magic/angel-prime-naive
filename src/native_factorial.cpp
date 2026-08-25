#include "angel/native_factorial.hpp"
#include "internal/conversions.hpp"
#include "internal/native_factorial_runtime.hpp"

#include <bit>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <variant>

namespace angel::prime {

namespace {

struct NativeBinding final {
    std::uint64_t factorial_argument{};
    std::uint64_t coefficient{};
    std::uint64_t result_seal{};
    std::uint64_t certificate_seal{};
    bool verified{};
};

[[nodiscard]] NativeBinding verify_native_binding(
    const detail::frozen::factorial_boundary::CertifiedWilsonRequest& request)
    noexcept {
    NativeBinding binding{};
    const auto request_report = detail::frozen::factorial_boundary::
        WilsonRequestVerifier::verify(request);
    if (!request_report.accepted) return binding;

    const auto& execution = request.factorial_execution();
    const auto& native = execution.r56_factorial;
    const auto native_report = detail::frozen::hashing::IndependentVerifier::verify(
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
    const detail::frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const NativeBinding& binding) noexcept {
    const auto& state = request.factorial_execution().principal_jet;
    return detail::frozen::hashing::mix64(
        request.binding_seal() ^
        std::rotl(state.seal(), 11) ^
        std::rotl(state.source_program().program().seal(), 23) ^
        std::rotl(binding.factorial_argument, 37) ^
        std::rotl(binding.coefficient, 43) ^
        std::rotl(binding.result_seal, 51) ^
        std::rotl(binding.certificate_seal, 59) ^
        0x4e41544641435631ULL);
}

[[nodiscard]] std::uint64_t exact_value_seal(
    const std::uint64_t view_seal,
    const std::uint64_t argument,
    const detail::BigUnsigned& value,
    const detail::BigUnsignedLedger& sequential,
    const detail::BigUnsignedLedger& tree) noexcept {
    return detail::frozen::hashing::mix64(
        view_seal ^ std::rotl(argument, 7) ^
        std::rotl(value.stable_hash(), 19) ^
        std::rotl(static_cast<std::uint64_t>(value.bit_length()), 31) ^
        std::rotl(sequential.small_limb_updates, 43) ^
        std::rotl(tree.limb_product_accumulations, 53) ^
        0x4558414354464143ULL);
}

[[nodiscard]] StateIntegrity state_integrity(
    const detail::frozen::factorial_boundary::CertifiedWilsonRequest& request,
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

[[nodiscard]] ExactFactorialLedger exact_ledger(
    const detail::ExactFactorialValueModel& model,
    const detail::BigUnsignedLedger* decimal = nullptr) noexcept {
    return ExactFactorialLedger{
        model.sequential_ledger.small_multiplications,
        model.sequential_ledger.small_limb_updates,
        model.product_tree_ledger.wide_multiplications,
        model.product_tree_ledger.limb_product_accumulations,
        decimal ? decimal->decimal_divisions : 0U,
        decimal ? decimal->decimal_limb_updates : 0U,
        static_cast<std::uint64_t>(model.value.limb_count()),
        static_cast<std::uint64_t>(model.value.bit_length())};
}

[[nodiscard]] NativeWilsonEvidence evidence_from(
    const NativeBinding& binding,
    const WilsonConsumptionMode mode) noexcept {
    NativeWilsonEvidence evidence{};
    evidence.mode = mode;
    evidence.factorial_argument = binding.factorial_argument;
    evidence.native_coefficient = binding.coefficient;
    evidence.native_result_seal = binding.result_seal;
    evidence.native_certificate_seal = binding.certificate_seal;
    evidence.native_coordinate_verified = binding.verified;
    evidence.factor_count_loaded_from_native_coordinate = true;
    evidence.candidate_used_only_as_modulus = true;
    return evidence;
}

} // namespace

NativeFactorialView operator|(
    const FactorialState& state, const BindNativeFactorial) {
    const auto& request = detail::PrimeAccess::model(state).request;
    const auto binding = verify_native_binding(request);
    if (!binding.verified)
        throw std::invalid_argument("native factorial coordinate rejected");
    return detail::NativeFactorialAccess::make_view(
        request, native_view_seal(request, binding));
}

ExactFactorialValue operator|(
    const NativeFactorialView& public_view,
    const DeriveExactFactorial operation) {
    const auto& model = detail::NativeFactorialAccess::model(public_view);
    const auto binding = verify_native_binding(model.request);
    if (!binding.verified ||
        model.view_seal != native_view_seal(model.request, binding))
        throw std::invalid_argument("native factorial view rejected");
    if (binding.factorial_argument >
        operation.policy.maximum_factorial_argument)
        throw std::length_error(
            "exact factorial argument exceeds explicit policy limit");

    detail::BigUnsignedLedger sequential_ledger{};
    auto sequential = detail::factorial_sequential(
        binding.factorial_argument, sequential_ledger);
    detail::BigUnsignedLedger tree_ledger{};
    const auto independent = detail::factorial_product_tree(
        binding.factorial_argument, tree_ledger);
    const bool equal = sequential == independent;
    if (!equal)
        throw std::logic_error("independent exact factorial derivations differ");
    const auto seal = exact_value_seal(
        model.view_seal, binding.factorial_argument, sequential,
        sequential_ledger, tree_ledger);
    return detail::NativeFactorialAccess::make_exact(
        model.request, model.view_seal, std::move(sequential),
        sequential_ledger, tree_ledger, seal, equal);
}

ExactFactorialDownload operator|(
    const ExactFactorialValue& public_value,
    const DownloadExactFactorial) {
    const auto& model = detail::NativeFactorialAccess::model(public_value);
    const auto binding = verify_native_binding(model.request);
    detail::BigUnsignedLedger decimal_ledger{};
    const auto decimal = model.value.decimal(&decimal_ledger);
    const auto hexadecimal = model.value.hexadecimal();
    const auto expected_seal = exact_value_seal(
        model.native_view_seal, binding.factorial_argument, model.value,
        model.sequential_ledger, model.product_tree_ledger);
    const bool seal_valid = model.exact_value_seal == expected_seal;
    const bool verified = binding.verified && model.independent_equal &&
                          seal_valid;
    return ExactFactorialDownload{
        binding.factorial_argument,
        decimal,
        hexadecimal,
        exact_ledger(model, &decimal_ledger),
        state_integrity(
            model.request, binding.certificate_seal, verified),
        true,
        model.independent_equal,
        seal_valid,
        verified};
}

NativeWilsonDownload operator|(
    const NativeFactorialView& public_view,
    const ProjectWilsonFromNative operation) {
    const auto& model = detail::NativeFactorialAccess::model(public_view);
    const auto binding = verify_native_binding(model.request);
    if (!binding.verified ||
        model.view_seal != native_view_seal(model.request, binding))
        throw std::invalid_argument("native factorial view rejected");
    return detail::execute_wilson_from_native_coordinate(
        model.request, operation.policy);
}

NativeWilsonDownload operator|(
    const ExactFactorialValue& public_value,
    const ObserveWilsonFromExact) {
    const auto& model = detail::NativeFactorialAccess::model(public_value);
    const auto binding = verify_native_binding(model.request);
    const auto expected_seal = exact_value_seal(
        model.native_view_seal, binding.factorial_argument, model.value,
        model.sequential_ledger, model.product_tree_ledger);
    const bool exact_valid = binding.verified && model.independent_equal &&
        model.exact_value_seal == expected_seal;
    const auto modulus = model.request.candidate();
    const auto residue = model.value.modulo(modulus);
    const bool prime = residue == modulus - 1U;

    ObservationLedger ledger{};
    const auto& execution = model.request.factorial_execution();
    ledger.input_bits = execution.principal_jet.valuation().bit_length();
    ledger.native_steps = execution.r56_factorial.ledger.total_steps() +
                          execution.fusion_ledger.total_native_steps();
    ledger.state_payload_bytes = execution.principal_jet.payload_bytes();
    ledger.target_factor_count = binding.factorial_argument;
    ledger.ordinary_projection_started = true;
    ledger.ordinary_projection_completed = true;
    ledger.ordinary_feedback = false;

    NativeWilsonEvidence evidence = evidence_from(
        binding, WilsonConsumptionMode::ExactBigIntegerRemainder);
    evidence.exact_value_seal = model.exact_value_seal;
    evidence.exact_big_integer_consumed = true;
    evidence.modular_result_matches_exact_big_integer = exact_valid;

    CoordinateSummary coordinate{};
    coordinate.factor_count = binding.factorial_argument;
    coordinate.total_scalar_slots =
        static_cast<std::uint64_t>(model.value.limb_count());
    const bool verified = exact_valid;
    return NativeWilsonDownload{
        WilsonObservation{modulus, residue, prime, ledger},
        coordinate,
        state_integrity(
            model.request, binding.certificate_seal, verified),
        evidence,
        verified};
}

NativeFactorialSummary NativeFactorialView::summary() const noexcept {
    const auto& model = detail::NativeFactorialAccess::model(*this);
    const auto binding = verify_native_binding(model.request);
    const auto& state = model.request.factorial_execution().principal_jet;
    return NativeFactorialSummary{
        model.request.candidate(),
        binding.factorial_argument,
        binding.coefficient,
        model.request.binding_seal(),
        state.seal(),
        state.source_program().program().seal(),
        binding.result_seal,
        binding.certificate_seal,
        model.view_seal,
        binding.verified &&
            model.view_seal == native_view_seal(model.request, binding),
        false};
}

bool NativeFactorialView::preserves_complete_state_identity() const noexcept {
    return summary().native_coordinate_verified;
}

ExactFactorialSummary ExactFactorialValue::summary() const noexcept {
    const auto& model = detail::NativeFactorialAccess::model(*this);
    const auto binding = verify_native_binding(model.request);
    return ExactFactorialSummary{
        binding.factorial_argument,
        static_cast<std::uint64_t>(model.value.bit_length()),
        static_cast<std::uint64_t>(model.value.limb_count()),
        model.value.stable_hash(),
        model.exact_value_seal,
        binding.verified,
        model.independent_equal};
}

} // namespace angel::prime

namespace angel::detail {

prime::NativeWilsonDownload execute_wilson_from_native_coordinate(
    const frozen::factorial_boundary::CertifiedWilsonRequest& request,
    const prime::ObservationPolicy& public_policy) {
    const auto binding = prime::verify_native_binding(request);
    if (!binding.verified)
        throw std::invalid_argument("native factorial coordinate rejected");

    auto policy = to_frozen(public_policy);
    const auto coordinate = frozen::quotient_view::natural_coordinate_dimension(
        binding.factorial_argument);
    if (policy.block_width_override != 0U &&
        policy.block_width_override != coordinate.block_width)
        throw std::invalid_argument(
            "native factorial coordinate and evaluation schedule differ");

    frozen::wilson::SublinearWilsonLedger ledger{};
    const auto& execution = request.factorial_execution();
    ledger.angel_input_bits = execution.principal_jet.valuation().bit_length();
    ledger.angel_native_steps =
        execution.r56_factorial.ledger.total_steps() +
        execution.fusion_ledger.total_native_steps();
    ledger.angel_state_payload_bytes = execution.principal_jet.payload_bytes();
    ledger.target_factor_count = binding.factorial_argument;
    ledger.ordinary_result_fed_back_to_angel = false;

    prime::NativeWilsonEvidence evidence = prime::evidence_from(
        binding,
        prime::WilsonConsumptionMode::NativeCoordinateModularProjection);
    const auto required_width = frozen::wilson::ceil_square_root(
        binding.factorial_argument);

    const auto make_limit = [&](const prime::LimitReason reason,
                                const std::uint64_t required,
                                const std::uint64_t allowed) {
        const auto verified = binding.verified;
        return prime::NativeWilsonDownload{
            prime::ResourceLimit{
                request.candidate(), reason, required, allowed,
                from_frozen(ledger)},
            from_frozen(coordinate),
            prime::state_integrity(
                request, binding.certificate_seal, verified),
            evidence,
            verified};
    };

    if (policy.block_width_override == 0U &&
        required_width > policy.maximum_block_width)
        return make_limit(
            prime::LimitReason::BlockWidth, required_width,
            policy.maximum_block_width);
    if (policy.block_width_override > policy.maximum_block_width)
        return make_limit(
            prime::LimitReason::BlockWidth,
            policy.block_width_override, policy.maximum_block_width);

    ledger.ordinary_projection_started = true;
    const auto start = std::chrono::steady_clock::now();
    try {
        const auto residue = frozen::wilson::detail::factorial_mod_sublinear(
            binding.factorial_argument, request.candidate(), policy, ledger);
        const auto stop = std::chrono::steady_clock::now();
        ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                stop - start).count());
        ledger.ordinary_projection_completed = true;
        const bool prime_result = residue == request.candidate() - 1U;
        const bool verified = binding.verified &&
            ledger.target_factor_count == binding.factorial_argument &&
            ledger.ordinary_projection_completed &&
            !ledger.ordinary_result_fed_back_to_angel;
        return prime::NativeWilsonDownload{
            prime::WilsonObservation{
                request.candidate(), residue, prime_result,
                from_frozen(ledger)},
            from_frozen(coordinate),
            prime::state_integrity(
                request, binding.certificate_seal, verified),
            evidence,
            verified};
    } catch (const frozen::wilson::PolynomialResourceError&) {
        const auto stop = std::chrono::steady_clock::now();
        ledger.ordinary_elapsed_nanoseconds = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                stop - start).count());
        return make_limit(
            prime::LimitReason::PolynomialEngine,
            ledger.block_width == 0U ? required_width : ledger.block_width,
            policy.maximum_block_width);
    }
}

} // namespace angel::detail
