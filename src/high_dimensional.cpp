#include "angel/high_dimensional.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace angel::high {
namespace {

[[nodiscard]] std::uint64_t mix64(std::uint64_t value) noexcept {
    value ^= value >> 30U;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27U;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31U;
    return value;
}

[[nodiscard]] std::uint64_t combine(
    const std::uint64_t seed,
    const std::uint64_t value) noexcept {
    return mix64(seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6U) +
                         (seed >> 2U)));
}

[[nodiscard]] bool checked_add(
    const Coefficient left,
    const Coefficient right,
    Coefficient& out) noexcept {
    constexpr auto maximum = std::numeric_limits<Coefficient>::max();
    constexpr auto minimum = std::numeric_limits<Coefficient>::min();
    if ((right > 0 && left > maximum - right) ||
        (right < 0 && left < minimum - right))
        return false;
    out = static_cast<Coefficient>(left + right);
    return true;
}

[[nodiscard]] bool checked_subtract(
    const Coefficient left,
    const Coefficient right,
    Coefficient& out) noexcept {
    constexpr auto maximum = std::numeric_limits<Coefficient>::max();
    constexpr auto minimum = std::numeric_limits<Coefficient>::min();
    if ((right < 0 && left > maximum + right) ||
        (right > 0 && left < minimum + right))
        return false;
    out = static_cast<Coefficient>(left - right);
    return true;
}

[[nodiscard]] bool checked_multiply(
    const Coefficient left,
    const Coefficient right,
    Coefficient& out) noexcept {
    constexpr auto maximum = std::numeric_limits<Coefficient>::max();
    constexpr auto minimum = std::numeric_limits<Coefficient>::min();
    if (left == 0 || right == 0) {
        out = 0;
        return true;
    }
    if ((left == -1 && right == minimum) ||
        (right == -1 && left == minimum))
        return false;
    if (left > 0) {
        if (right > 0) {
            if (left > maximum / right) return false;
        } else if (right < minimum / left) {
            return false;
        }
    } else if (right > 0) {
        if (left < minimum / right) return false;
    } else if (left < maximum / right) {
        return false;
    }
    out = static_cast<Coefficient>(left * right);
    return true;
}

void normalize(SparseAxis& axis) {
    for (auto it = axis.begin(); it != axis.end();) {
        if (it->second == 0)
            it = axis.erase(it);
        else
            ++it;
    }
}

[[nodiscard]] HistoryWord concatenate(
    const HistoryWord& left,
    const HistoryWord& right) {
    HistoryWord out;
    out.reserve(left.size() + right.size());
    out.insert(out.end(), left.begin(), left.end());
    out.insert(out.end(), right.begin(), right.end());
    return out;
}

[[nodiscard]] bool add_term(
    SparseAxis& destination,
    HistoryWord word,
    const Coefficient coefficient) {
    if (coefficient == 0) return true;
    const auto found = destination.find(word);
    if (found == destination.end()) {
        destination.emplace(std::move(word), coefficient);
        return true;
    }
    Coefficient sum{};
    if (!checked_add(found->second, coefficient, sum)) return false;
    if (sum == 0)
        destination.erase(found);
    else
        found->second = sum;
    return true;
}

[[nodiscard]] bool add_scaled(
    SparseAxis& destination,
    const SparseAxis& source,
    const Coefficient scalar) {
    for (const auto& [word, coefficient] : source) {
        Coefficient product{};
        if (!checked_multiply(coefficient, scalar, product) ||
            !add_term(destination, word, product))
            return false;
    }
    return true;
}

[[nodiscard]] bool add_product(
    SparseAxis& destination,
    const SparseAxis& left,
    const SparseAxis& right) {
    for (const auto& [left_word, left_coefficient] : left) {
        for (const auto& [right_word, right_coefficient] : right) {
            Coefficient product{};
            if (!checked_multiply(
                    left_coefficient, right_coefficient, product) ||
                !add_term(
                    destination,
                    concatenate(left_word, right_word),
                    product))
                return false;
        }
    }
    return true;
}

[[nodiscard]] ArithmeticContinuation continuation(
    const ArithmeticOpcode opcode,
    const ContinuationReason reason,
    const TriClassValue& left,
    const TriClassValue& right) {
    ArithmeticContinuation out{};
    out.attempted = opcode;
    out.reason = reason;
    out.left = left;
    out.right = right;
    out.left_parent_seal = left.seal();
    out.right_parent_seal = right.seal();
    out.operands_retained = true;
    out.no_false_scalar_collapse = true;
    out.future_live = true;
    out.certificate_seal = combine(
        combine(
            combine(0x414e47454c434f4eULL,
                    static_cast<std::uint64_t>(opcode)),
            static_cast<std::uint64_t>(reason)),
        combine(out.left_parent_seal, out.right_parent_seal));
    return out;
}

[[nodiscard]] std::uint64_t division_certificate(
    const TriClassValue& numerator,
    const TriClassValue& denominator,
    const TriClassValue& quotient,
    const TriClassValue& residual) noexcept {
    auto seal = combine(0x414e47454c444956ULL, numerator.seal());
    seal = combine(seal, denominator.seal());
    seal = combine(seal, quotient.seal());
    return combine(seal, residual.seal());
}

[[nodiscard]] bool divide_axis(
    const SparseAxis& input,
    const Coefficient denominator,
    SparseAxis& quotient,
    SparseAxis& residual) {
    constexpr auto minimum = std::numeric_limits<Coefficient>::min();
    for (const auto& [word, coefficient] : input) {
        if (coefficient == minimum && denominator == -1) return false;
        const auto q = static_cast<Coefficient>(coefficient / denominator);
        const auto r = static_cast<Coefficient>(coefficient % denominator);
        if (q != 0) quotient.emplace(word, q);
        if (r != 0) residual.emplace(word, r);
    }
    return true;
}

[[nodiscard]] bool checked_increment(
    const std::uint64_t value,
    std::uint64_t& out) noexcept {
    if (value == std::numeric_limits<std::uint64_t>::max()) return false;
    out = value + 1U;
    return true;
}

[[nodiscard]] std::uint64_t state_seal(
    const TriClassValue& value,
    const MazeAddress& address,
    const std::vector<CausalEvent>& history) noexcept {
    auto seal = combine(0x414e47454c4d415aULL, value.seal());
    seal = combine(seal, address.room);
    seal = combine(seal, address.road);
    seal = combine(seal, address.frame);
    seal = combine(seal, address.causal_epoch);
    seal = combine(seal, address.history_depth);
    seal = combine(seal, address.holonomy);
    seal = combine(seal, address.singular_generation);
    seal = combine(seal, address.higher_cell);
    for (const auto& event : history) {
        seal = combine(seal, static_cast<std::uint64_t>(event.opcode));
        seal = combine(seal, event.left_parent_seal);
        seal = combine(seal, event.right_parent_seal);
        seal = combine(seal, event.event_seal);
    }
    return seal;
}

[[nodiscard]] std::uint64_t event_seal(
    const ArithmeticOpcode opcode,
    const std::uint64_t left,
    const std::uint64_t right,
    const std::uint64_t value) noexcept {
    auto seal = combine(0x414e47454c45564eULL,
                        static_cast<std::uint64_t>(opcode));
    seal = combine(seal, left);
    seal = combine(seal, right);
    return combine(seal, value);
}

[[nodiscard]] bool prepare_binary_address(
    const MazeState& left,
    const MazeState& right,
    const ArithmeticOpcode opcode,
    const TriClassValue& value,
    MazeAddress& address) {
    if (left.address().room != right.address().room ||
        left.address().frame != right.address().frame)
        return false;
    address = left.address();
    std::uint64_t next_epoch{};
    std::uint64_t next_depth{};
    if (!checked_increment(
            std::max(left.address().causal_epoch,
                     right.address().causal_epoch),
            next_epoch) ||
        !checked_increment(
            std::max(left.address().history_depth,
                     right.address().history_depth),
            next_depth))
        return false;
    address.causal_epoch = next_epoch;
    address.history_depth = next_depth;
    address.road = combine(
        combine(left.address().road, right.address().road),
        static_cast<std::uint64_t>(opcode));
    address.holonomy = combine(
        combine(left.address().holonomy, right.address().holonomy),
        value.seal());
    address.higher_cell = combine(
        combine(left.address().higher_cell, right.address().higher_cell),
        next_depth);
    const auto base_generation = std::max(
        left.address().singular_generation,
        right.address().singular_generation);
    if (!value.singular_coordinate().empty()) {
        if (!checked_increment(base_generation, address.singular_generation))
            return false;
    } else {
        address.singular_generation = base_generation;
    }
    return true;
}

} // namespace

StateResult detail_lift_binary(
    const MazeState& left,
    const MazeState& right,
    const ArithmeticOpcode opcode,
    const ArithmeticResult& value_result) {
    if (const auto* unresolved =
            std::get_if<ArithmeticContinuation>(&value_result))
        return *unresolved;
    const auto& value = std::get<TriClassValue>(value_result);
    if (left.address().room != right.address().room ||
        left.address().frame != right.address().frame)
        return continuation(
            opcode, ContinuationReason::FrameMismatch,
            left.value(), right.value());
    MazeAddress address{};
    if (!prepare_binary_address(left, right, opcode, value, address))
        return continuation(
            opcode, ContinuationReason::CausalCoordinateOverflow,
            left.value(), right.value());
    auto events = left.history();
    events.insert(events.end(), right.history().begin(), right.history().end());
    const auto e_seal = event_seal(
        opcode, left.seal(), right.seal(), value.seal());
    events.push_back(CausalEvent{opcode, left.seal(), right.seal(), e_seal});
    return MazeState(value, address, std::move(events));
}

TriClassValue::TriClassValue(
    const Coefficient ordinary,
    SparseAxis history,
    SparseAxis singular)
    : ordinary_(ordinary),
      history_(std::move(history)),
      singular_(std::move(singular)) {
    normalize(history_);
    normalize(singular_);
}

TriClassValue TriClassValue::ordinary(const Coefficient value) {
    return TriClassValue(value, {}, {});
}

TriClassValue TriClassValue::history(
    const HistoryToken token,
    const Coefficient coefficient) {
    SparseAxis axis;
    if (coefficient != 0) axis.emplace(HistoryWord{token}, coefficient);
    return TriClassValue(0, std::move(axis), {});
}

TriClassValue TriClassValue::singular(
    const HistoryToken token,
    const Coefficient coefficient) {
    SparseAxis axis;
    if (coefficient != 0) axis.emplace(HistoryWord{token}, coefficient);
    return TriClassValue(0, {}, std::move(axis));
}

TriClassValue TriClassValue::exact(
    const Coefficient ordinary,
    SparseAxis history,
    SparseAxis singular) {
    return TriClassValue(
        ordinary, std::move(history), std::move(singular));
}

bool TriClassValue::is_zero() const noexcept {
    return ordinary_ == 0 && history_.empty() && singular_.empty();
}

bool TriClassValue::is_pure_ordinary() const noexcept {
    return history_.empty() && singular_.empty();
}

std::uint64_t TriClassValue::seal() const noexcept {
    auto out = combine(0x414e47454c545249ULL,
                       static_cast<std::uint64_t>(ordinary_));
    const auto fold = [&out](const SparseAxis& axis, const std::uint64_t tag) {
        out = combine(out, tag);
        for (const auto& [word, coefficient] : axis) {
            out = combine(out, static_cast<std::uint64_t>(word.size()));
            for (const auto token : word) out = combine(out, token);
            out = combine(out, static_cast<std::uint64_t>(coefficient));
        }
    };
    fold(history_, 0x484953544f525900ULL);
    fold(singular_, 0x53494e47554c4152ULL);
    return out;
}

ArithmeticResult hadd(
    const TriClassValue& left,
    const TriClassValue& right) {
    Coefficient ordinary{};
    if (!checked_add(
            left.ordinary_coordinate(), right.ordinary_coordinate(), ordinary))
        return continuation(
            ArithmeticOpcode::HAdd,
            ContinuationReason::CoefficientOverflow,
            left, right);
    auto history = left.history_coordinate();
    auto singular = left.singular_coordinate();
    if (!add_scaled(history, right.history_coordinate(), 1) ||
        !add_scaled(singular, right.singular_coordinate(), 1))
        return continuation(
            ArithmeticOpcode::HAdd,
            ContinuationReason::CoefficientOverflow,
            left, right);
    return TriClassValue::exact(
        ordinary, std::move(history), std::move(singular));
}

ArithmeticResult hsub(
    const TriClassValue& left,
    const TriClassValue& right) {
    Coefficient ordinary{};
    if (!checked_subtract(
            left.ordinary_coordinate(), right.ordinary_coordinate(), ordinary))
        return continuation(
            ArithmeticOpcode::HSub,
            ContinuationReason::CoefficientOverflow,
            left, right);
    auto history = left.history_coordinate();
    auto singular = left.singular_coordinate();
    if (!add_scaled(history, right.history_coordinate(), -1) ||
        !add_scaled(singular, right.singular_coordinate(), -1))
        return continuation(
            ArithmeticOpcode::HSub,
            ContinuationReason::CoefficientOverflow,
            left, right);
    return TriClassValue::exact(
        ordinary, std::move(history), std::move(singular));
}

ArithmeticResult hmul(
    const TriClassValue& left,
    const TriClassValue& right) {
    Coefficient ordinary{};
    if (!checked_multiply(
            left.ordinary_coordinate(), right.ordinary_coordinate(), ordinary))
        return continuation(
            ArithmeticOpcode::HMul,
            ContinuationReason::CoefficientOverflow,
            left, right);

    SparseAxis history;
    SparseAxis singular;
    const bool accepted =
        add_scaled(
            history, right.history_coordinate(), left.ordinary_coordinate()) &&
        add_scaled(
            history, left.history_coordinate(), right.ordinary_coordinate()) &&
        add_product(
            history, left.history_coordinate(), right.history_coordinate()) &&
        add_scaled(
            singular, right.singular_coordinate(), left.ordinary_coordinate()) &&
        add_scaled(
            singular, left.singular_coordinate(), right.ordinary_coordinate()) &&
        add_product(
            singular, left.history_coordinate(), right.singular_coordinate()) &&
        add_product(
            singular, left.singular_coordinate(), right.history_coordinate()) &&
        add_product(
            singular, left.singular_coordinate(), right.singular_coordinate());
    if (!accepted)
        return continuation(
            ArithmeticOpcode::HMul,
            ContinuationReason::CoefficientOverflow,
            left, right);
    return TriClassValue::exact(
        ordinary, std::move(history), std::move(singular));
}

DivisionResult hdiv(
    const TriClassValue& numerator,
    const TriClassValue& denominator) {
    if (!denominator.is_pure_ordinary())
        return continuation(
            ArithmeticOpcode::HDiv,
            ContinuationReason::NonCentralDenominator,
            numerator, denominator);

    const auto divisor = denominator.ordinary_coordinate();
    TriClassValue quotient;
    TriClassValue residual;
    if (divisor == 0) {
        quotient = TriClassValue::ordinary(0);
        residual = numerator;
    } else {
        constexpr auto minimum = std::numeric_limits<Coefficient>::min();
        if (numerator.ordinary_coordinate() == minimum && divisor == -1)
            return continuation(
                ArithmeticOpcode::HDiv,
                ContinuationReason::CoefficientOverflow,
                numerator, denominator);
        const auto q_ordinary = static_cast<Coefficient>(
            numerator.ordinary_coordinate() / divisor);
        const auto r_ordinary = static_cast<Coefficient>(
            numerator.ordinary_coordinate() % divisor);
        SparseAxis q_history;
        SparseAxis r_history;
        SparseAxis q_singular;
        SparseAxis r_singular;
        if (!divide_axis(
                numerator.history_coordinate(), divisor,
                q_history, r_history) ||
            !divide_axis(
                numerator.singular_coordinate(), divisor,
                q_singular, r_singular))
            return continuation(
                ArithmeticOpcode::HDiv,
                ContinuationReason::CoefficientOverflow,
                numerator, denominator);
        quotient = TriClassValue::exact(
            q_ordinary, std::move(q_history), std::move(q_singular));
        residual = TriClassValue::exact(
            r_ordinary, std::move(r_history), std::move(r_singular));
    }

    DivisionPacket packet{};
    packet.numerator = numerator;
    packet.denominator = denominator;
    packet.quotient = quotient;
    packet.residual = SingularPayload{residual, true};
    packet.numerator_seal = numerator.seal();
    packet.denominator_seal = denominator.seal();
    packet.certificate_seal = division_certificate(
        numerator, denominator, quotient, residual);
    packet.reconstruction_verified = verify(packet);
    if (!packet.reconstruction_verified)
        return continuation(
            ArithmeticOpcode::HDiv,
            ContinuationReason::CoefficientOverflow,
            numerator, denominator);
    return packet;
}

bool verify(const ArithmeticContinuation& packet) noexcept {
    const auto expected = continuation(
        packet.attempted, packet.reason, packet.left, packet.right);
    return packet.left_parent_seal == expected.left_parent_seal &&
           packet.right_parent_seal == expected.right_parent_seal &&
           packet.certificate_seal == expected.certificate_seal &&
           packet.operands_retained && packet.no_false_scalar_collapse &&
           packet.future_live;
}

bool verify(const DivisionPacket& packet) {
    if (!packet.residual.typed_as_singular_payload ||
        packet.numerator_seal != packet.numerator.seal() ||
        packet.denominator_seal != packet.denominator.seal() ||
        packet.certificate_seal != division_certificate(
            packet.numerator, packet.denominator, packet.quotient,
            packet.residual.unresolved))
        return false;
    const auto product = hmul(packet.denominator, packet.quotient);
    if (!std::holds_alternative<TriClassValue>(product)) return false;
    const auto reconstructed = hadd(
        std::get<TriClassValue>(product), packet.residual.unresolved);
    return std::holds_alternative<TriClassValue>(reconstructed) &&
           std::get<TriClassValue>(reconstructed) == packet.numerator;
}

MazeState::MazeState(
    TriClassValue value,
    MazeAddress address,
    std::vector<CausalEvent> history)
    : value_(std::move(value)),
      address_(address),
      history_(std::move(history)),
      seal_(state_seal(value_, address_, history_)) {}

bool MazeState::exactly_equal(const MazeState& other) const noexcept {
    return seal_ == other.seal_ && value_ == other.value_ &&
           address_ == other.address_ && history_ == other.history_;
}

MazeState upload_complete(TriClassValue value, MazeAddress address) {
    return MazeState(std::move(value), address, {});
}

MazeState UploadFunctor::operator()(const OrdinarySpecification& input) const {
    auto address = input.initial_address;
    address.higher_cell = combine(address.higher_cell, input.schema_version);
    return upload_complete(TriClassValue::ordinary(input.value), address);
}

StateResult hadd(const MazeState& left, const MazeState& right) {
    return detail_lift_binary(
        left, right, ArithmeticOpcode::HAdd,
        hadd(left.value(), right.value()));
}

StateResult hsub(const MazeState& left, const MazeState& right) {
    return detail_lift_binary(
        left, right, ArithmeticOpcode::HSub,
        hsub(left.value(), right.value()));
}

StateResult hmul(const MazeState& left, const MazeState& right) {
    return detail_lift_binary(
        left, right, ArithmeticOpcode::HMul,
        hmul(left.value(), right.value()));
}

NativeFunctor::NativeFunctor(std::vector<NativeInstruction> instructions)
    : instructions_(std::move(instructions)) {}

NativeFunctor NativeFunctor::identity() { return NativeFunctor{}; }

NativeFunctor NativeFunctor::add(TriClassValue operand) {
    return NativeFunctor({NativeInstruction{
        ArithmeticOpcode::HAdd, std::move(operand)}});
}

NativeFunctor NativeFunctor::subtract(TriClassValue operand) {
    return NativeFunctor({NativeInstruction{
        ArithmeticOpcode::HSub, std::move(operand)}});
}

NativeFunctor NativeFunctor::multiply(TriClassValue operand) {
    return NativeFunctor({NativeInstruction{
        ArithmeticOpcode::HMul, std::move(operand)}});
}

NativeFunctor NativeFunctor::divide_exact(TriClassValue operand) {
    return NativeFunctor({NativeInstruction{
        ArithmeticOpcode::HDiv, std::move(operand)}});
}

StateResult NativeFunctor::operator()(const MazeState& state) const {
    auto current = state;
    for (const auto& instruction : instructions_) {
        if (instruction.opcode == ArithmeticOpcode::HDiv) {
            const auto divided = hdiv(current.value(), instruction.operand);
            if (const auto* unresolved =
                    std::get_if<ArithmeticContinuation>(&divided))
                return *unresolved;
            const auto& packet = std::get<DivisionPacket>(divided);
            if (!packet.residual.unresolved.is_zero())
                return continuation(
                    ArithmeticOpcode::HDiv,
                    ContinuationReason::ExactDivisionRequired,
                    current.value(), instruction.operand);

            MazeAddress address = current.address();
            std::uint64_t next_epoch{};
            std::uint64_t next_depth{};
            if (!checked_increment(address.causal_epoch, next_epoch) ||
                !checked_increment(address.history_depth, next_depth))
                return continuation(
                    ArithmeticOpcode::HDiv,
                    ContinuationReason::CausalCoordinateOverflow,
                    current.value(), instruction.operand);
            address.causal_epoch = next_epoch;
            address.history_depth = next_depth;
            address.road = combine(address.road, instruction.operand.seal());
            address.holonomy = combine(address.holonomy, packet.quotient.seal());
            address.higher_cell = combine(address.higher_cell, next_depth);
            auto events = current.history();
            const auto right_seal = instruction.operand.seal();
            const auto e_seal = event_seal(
                ArithmeticOpcode::HDiv, current.seal(), right_seal,
                packet.quotient.seal());
            events.push_back(CausalEvent{
                ArithmeticOpcode::HDiv, current.seal(), right_seal, e_seal});
            current = MazeState(
                packet.quotient, address, std::move(events));
            continue;
        }

        const MazeState operand(
            instruction.operand, current.address(), {});
        StateResult result = continuation(
            instruction.opcode,
            ContinuationReason::CoefficientOverflow,
            current.value(), instruction.operand);
        if (instruction.opcode == ArithmeticOpcode::HAdd)
            result = hadd(current, operand);
        else if (instruction.opcode == ArithmeticOpcode::HSub)
            result = hsub(current, operand);
        else if (instruction.opcode == ArithmeticOpcode::HMul)
            result = hmul(current, operand);
        if (const auto* unresolved =
                std::get_if<ArithmeticContinuation>(&result))
            return *unresolved;
        current = std::get<MazeState>(std::move(result));
    }
    return current;
}

NativeFunctor compose(
    const NativeFunctor& after,
    const NativeFunctor& before) {
    auto instructions = before.instructions();
    instructions.insert(
        instructions.end(),
        after.instructions().begin(), after.instructions().end());
    return NativeFunctor(std::move(instructions));
}

DerivedObservation DownloadFunctor::operator()(const MazeState& state) const noexcept {
    return DerivedObservation{
        state.value().ordinary_coordinate(), state.seal(),
        state.address().history_depth, 1U, true};
}

bool validate_observation(
    const MazeState& canonical,
    const DerivedObservation& derived) noexcept {
    return derived.schema_version == 1U && derived.derived_non_authoritative &&
           derived.ordinary_coordinate ==
               canonical.value().ordinary_coordinate() &&
           derived.state_seal == canonical.seal() &&
           derived.history_depth == canonical.address().history_depth;
}

ClassQuantumFunctor::ClassQuantumFunctor(
    std::vector<WeightedFunctor> branches)
    : branches_(std::move(branches)) {}

SuperpositionResult ClassQuantumFunctor::operator()(
    const MazeState& input) const {
    HistorySuperposition packet{};
    packet.branches.reserve(branches_.size());
    for (const auto& branch : branches_) {
        if (branch.weight == 0) continue;
        const auto result = branch.functor(input);
        if (const auto* unresolved =
                std::get_if<ArithmeticContinuation>(&result))
            return *unresolved;
        packet.branches.push_back(WeightedHistory{
            branch.weight, branch.branch_label, {branch.branch_label},
            std::get<MazeState>(result)});
    }
    return packet;
}

ClassQuantumFunctor superpose(
    const ClassQuantumFunctor& left,
    const ClassQuantumFunctor& right) {
    auto branches = left.branches();
    branches.insert(
        branches.end(), right.branches().begin(), right.branches().end());
    return ClassQuantumFunctor(std::move(branches));
}

std::variant<InterferenceResult, ArithmeticContinuation>
interfere_structurally(const HistorySuperposition& input) {
    struct Group final {
        Coefficient weight{};
        std::vector<std::size_t> members;
        WeightedHistory exemplar;
    };
    std::vector<Group> groups;
    for (std::size_t index = 0U; index < input.branches.size(); ++index) {
        const auto& branch = input.branches[index];
        auto found = std::find_if(
            groups.begin(), groups.end(),
            [&branch](const Group& group) {
                return group.exemplar.endpoint.exactly_equal(branch.endpoint);
            });
        if (found == groups.end()) {
            groups.push_back(Group{
                branch.weight, {index}, branch});
            continue;
        }
        Coefficient sum{};
        if (!checked_add(found->weight, branch.weight, sum))
            return continuation(
                ArithmeticOpcode::HAdd,
                ContinuationReason::CoefficientOverflow,
                found->exemplar.endpoint.value(), branch.endpoint.value());
        found->weight = sum;
        found->members.push_back(index);
    }

    InterferenceResult out{};
    out.input_branches = static_cast<std::uint64_t>(input.branches.size());
    out.structural_key_only = true;
    for (auto& group : groups) {
        if (group.weight == 0) {
            out.canceled_branches +=
                static_cast<std::uint64_t>(group.members.size());
            ++out.canceled_endpoints;
            continue;
        }
        group.exemplar.weight = group.weight;
        out.survivors.branches.push_back(std::move(group.exemplar));
    }
    return out;
}

TensorHistory::TensorHistory(
    const std::size_t left_dimension,
    const std::size_t right_dimension,
    std::vector<Coefficient> coefficients)
    : left_dimension_(left_dimension),
      right_dimension_(right_dimension),
      coefficients_(std::move(coefficients)) {}

TensorHistory TensorHistory::exact(
    const std::size_t left_dimension,
    const std::size_t right_dimension,
    std::vector<Coefficient> coefficients) {
    if (left_dimension == 0U || right_dimension == 0U ||
        left_dimension >
            std::numeric_limits<std::size_t>::max() / right_dimension ||
        coefficients.size() != left_dimension * right_dimension)
        throw std::invalid_argument("invalid class-quantum tensor dimensions");
    return TensorHistory(
        left_dimension, right_dimension, std::move(coefficients));
}

Coefficient TensorHistory::at(
    const std::size_t left,
    const std::size_t right) const {
    if (left >= left_dimension_ || right >= right_dimension_)
        throw std::out_of_range("class-quantum tensor coordinate out of range");
    return coefficients_[left * right_dimension_ + right];
}

TensorBuildResult tensor_product(
    const std::vector<Coefficient>& left,
    const std::vector<Coefficient>& right) {
    if (left.empty() || right.empty() ||
        left.size() > std::numeric_limits<std::size_t>::max() / right.size())
        return TensorContinuation{false, true};
    std::vector<Coefficient> coefficients;
    coefficients.reserve(left.size() * right.size());
    for (const auto left_value : left) {
        for (const auto right_value : right) {
            Coefficient product{};
            if (!checked_multiply(left_value, right_value, product))
                return TensorContinuation{true, true};
            coefficients.push_back(product);
        }
    }
    return TensorHistory::exact(
        left.size(), right.size(), std::move(coefficients));
}

EntanglementResult analyze_entanglement(const TensorHistory& tensor) {
    for (std::size_t row_a = 0U;
         row_a < tensor.left_dimension(); ++row_a) {
        for (std::size_t row_b = row_a + 1U;
             row_b < tensor.left_dimension(); ++row_b) {
            for (std::size_t column_a = 0U;
                 column_a < tensor.right_dimension(); ++column_a) {
                for (std::size_t column_b = column_a + 1U;
                     column_b < tensor.right_dimension(); ++column_b) {
                    Coefficient diagonal{};
                    Coefficient off_diagonal{};
                    Coefficient minor{};
                    if (!checked_multiply(
                            tensor.at(row_a, column_a),
                            tensor.at(row_b, column_b), diagonal) ||
                        !checked_multiply(
                            tensor.at(row_a, column_b),
                            tensor.at(row_b, column_a), off_diagonal) ||
                        !checked_subtract(diagonal, off_diagonal, minor))
                        return TensorContinuation{true, true};
                    if (minor != 0)
                        return EntanglementCertificate{
                            true, true, row_a, row_b,
                            column_a, column_b, minor};
                }
            }
        }
    }
    return EntanglementCertificate{true, false, 0U, 0U, 0U, 0U, 0};
}

} // namespace angel::high
