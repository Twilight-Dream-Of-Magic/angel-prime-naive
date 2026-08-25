#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace angel::r23 {

// Self-contained nonnegative arbitrary-precision substrate counter.
// It has no conversion back to a machine integer and no semantic role in
// class selection; it only charges and transports collective multiplicities.
class BigCount final {
public:
    BigCount() = default;
    explicit BigCount(std::uint64_t value);

    [[nodiscard]] bool is_zero() const noexcept { return limbs_.empty(); }
    [[nodiscard]] std::size_t limb_count() const noexcept {
        return limbs_.size();
    }
    [[nodiscard]] std::size_t bit_length() const noexcept;
    [[nodiscard]] std::string to_decimal() const;

    BigCount& operator++();
    BigCount& operator+=(const BigCount& rhs);
    BigCount& operator*=(const BigCount& rhs);

    friend bool operator==(const BigCount&, const BigCount&) = default;
    friend BigCount operator+(const BigCount& lhs, const BigCount& rhs);
    friend BigCount operator*(const BigCount& lhs, const BigCount& rhs);
    friend BigCount operator<<(const BigCount& value, std::size_t bits);
    friend std::ostream& operator<<(std::ostream& out, const BigCount& value);

private:
    std::vector<std::uint32_t> limbs_;
    void normalize() noexcept;
    [[nodiscard]] std::uint32_t divide_small(std::uint32_t denominator);
};

struct FutureSyntaxState final {
    std::string name;
    // One value per observer syntax node.
    std::vector<std::uint64_t> observations;
    // One destination state per operator syntax node.
    std::vector<std::size_t> transitions;

    friend bool operator==(const FutureSyntaxState&,
                           const FutureSyntaxState&) = default;
};

struct FutureLanguageProgram final {
    std::vector<std::string> observer_names;
    std::vector<std::string> operator_names;
    std::vector<FutureSyntaxState> states;

    friend bool operator==(const FutureLanguageProgram&,
                           const FutureLanguageProgram&) = default;
};

struct FutureQuotientCost final {
    std::uint64_t refinement_rounds{};
    std::uint64_t syntax_states_visited{};
    std::uint64_t syntax_transitions_visited{};
    std::uint64_t denotation_events_visited{};
};

class CertifiedFutureQuotient final {
public:
    CertifiedFutureQuotient(const CertifiedFutureQuotient&) = default;
    CertifiedFutureQuotient(CertifiedFutureQuotient&&) noexcept = default;
    CertifiedFutureQuotient& operator=(const CertifiedFutureQuotient&) = default;
    CertifiedFutureQuotient& operator=(CertifiedFutureQuotient&&) noexcept = default;

    [[nodiscard]] const std::vector<std::size_t>& class_of_state() const noexcept {
        return class_of_state_;
    }
    // transfer[operator][class] -> destination class.
    [[nodiscard]] const std::vector<std::vector<std::size_t>>&
    transfer() const noexcept {
        return transfer_;
    }
    [[nodiscard]] std::size_t class_count() const noexcept {
        return class_count_;
    }
    [[nodiscard]] const FutureQuotientCost& cost() const noexcept {
        return cost_;
    }
    [[nodiscard]] const FutureLanguageProgram& language() const noexcept {
        return language_;
    }

private:
    CertifiedFutureQuotient() = default;

    FutureLanguageProgram language_;
    std::vector<std::size_t> class_of_state_;
    std::vector<std::vector<std::size_t>> transfer_;
    std::size_t class_count_{};
    FutureQuotientCost cost_;
    bool canonical_first_occurrence_numbering_{};

    friend class FutureQuotientCompiler;
};

struct QuotientVerification final {
    bool accepted{};
    std::string failure;
    bool observers_constant_on_classes{};
    bool operators_congruent_on_classes{};
    bool class_ids_contiguous{};
};

enum class LanguageBridgeResult : std::uint8_t {
    Identical,
    Refined,
    Rejected
};

class FutureQuotientCompiler final {
public:
    [[nodiscard]] CertifiedFutureQuotient
    compile(const FutureLanguageProgram& language) const;

    [[nodiscard]] QuotientVerification
    verify(const FutureLanguageProgram& language,
           const CertifiedFutureQuotient& quotient) const;

    [[nodiscard]] LanguageBridgeResult
    bridge_language_extension(const CertifiedFutureQuotient& old_quotient,
                              const CertifiedFutureQuotient& new_quotient) const;
};

struct ClassFibreVector final {
    std::vector<BigCount> weight;
};

struct TransferFoldCost final {
    std::uint64_t transfer_sum_updates{};
    std::uint64_t matrix_matrix_multiplications{};
    std::uint64_t matrix_vector_multiplications{};
    std::uint64_t matrix_scalar_multiply_adds{};
    std::uint64_t bigcount_addition_calls{};
    std::uint64_t bigcount_multiplication_calls{};
    std::uint64_t bigcount_limb_add_iterations{};
    std::uint64_t bigcount_limb_product_iterations{};
    std::uint64_t count_power_bigcount_multiplications{};
    std::uint64_t exponent_bits_consumed{};
    std::uint64_t count_exponent_bits_consumed{};
    std::uint64_t empty_class_scans{};
    std::uint64_t denotation_operator_words_visited{};
};

struct CollectiveFoldResult final {
    ClassFibreVector fibres;
    TransferFoldCost cost;
    BigCount represented_operator_word_occurrences;
    std::vector<std::size_t> empty_classes;
};

class FutureClassTransferFold final {
public:
    // Every operator in the FutureLanguage is one legal branch action.
    // The result is the exact aggregate after all operator words of `depth`.
    [[nodiscard]] CollectiveFoldResult
    fold_all_operator_words(const CertifiedFutureQuotient& quotient,
                            const ClassFibreVector& initial,
                            std::uint64_t depth) const;
};

}  // namespace angel::r23
