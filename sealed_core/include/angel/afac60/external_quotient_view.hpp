#pragma once

#include "angel/afac59/sublinear_wilson.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace angel::afac60 {

// This is an external coordinate count, not an Angel-state payload count.
// For the natural R59 coordinate s=ceil(sqrt(n)), the formula itself exposes
// Theta(sqrt(n)) materialized scalar slots.  R60 records that fact instead of
// relabelling the coordinate as a polylogarithmic quotient.
struct ExternalCoordinateDimension final {
    std::uint64_t factor_count{};
    std::uint64_t block_width{};
    std::uint64_t full_blocks{};
    std::uint64_t tail_factors{};
    std::uint64_t polynomial_coefficients{};
    std::uint64_t evaluation_values{};
    std::uint64_t total_scalar_slots{};
    bool natural_sqrt_coordinate{};
    bool polylogarithmic_summary_claimed{};
    bool tombstoned_as_polylog_candidate{};

    friend bool operator==(const ExternalCoordinateDimension&,
                           const ExternalCoordinateDimension&) = default;
};

[[nodiscard]] inline ExternalCoordinateDimension natural_coordinate_dimension(
    const std::uint64_t factor_count) {
    if (factor_count == 0U)
        throw std::invalid_argument("factor count must be positive");
    const auto width = angel::afac59::ceil_square_root(factor_count);
    const auto full_blocks = factor_count / width;
    const auto tail = factor_count - full_blocks * width;
    return ExternalCoordinateDimension{
        factor_count,
        width,
        full_blocks,
        tail,
        width + 1U,
        full_blocks,
        width + 1U + full_blocks + tail,
        true,
        false,
        true};
}

[[nodiscard]] inline std::uint64_t external_coordinate_seal(
    const ExternalCoordinateDimension& coordinate) noexcept {
    return angel::afac56::mix64(
        coordinate.factor_count ^
        std::rotl(coordinate.block_width, 7) ^
        std::rotl(coordinate.full_blocks, 17) ^
        std::rotl(coordinate.tail_factors, 27) ^
        std::rotl(coordinate.polynomial_coefficients, 37) ^
        std::rotl(coordinate.evaluation_values, 47) ^
        std::rotl(coordinate.total_scalar_slots, 53) ^
        std::rotl(static_cast<std::uint64_t>(
            coordinate.natural_sqrt_coordinate), 59) ^
        std::rotl(static_cast<std::uint64_t>(
            coordinate.polylogarithmic_summary_claimed), 61) ^
        std::rotl(static_cast<std::uint64_t>(
            coordinate.tombstoned_as_polylog_candidate), 63) ^
        0x434f4f5244363045ULL);
}

class CertifiedExternalQuotientView final {
public:
    CertifiedExternalQuotientView() = delete;
    CertifiedExternalQuotientView(const CertifiedExternalQuotientView&) = default;
    CertifiedExternalQuotientView(CertifiedExternalQuotientView&&) noexcept = default;
    CertifiedExternalQuotientView& operator=(
        const CertifiedExternalQuotientView&) = default;
    CertifiedExternalQuotientView& operator=(
        CertifiedExternalQuotientView&&) noexcept = default;

    [[nodiscard]] std::uint64_t candidate() const noexcept { return candidate_; }
    [[nodiscard]] std::uint64_t modulus() const noexcept { return modulus_; }
    [[nodiscard]] std::uint64_t request_binding() const noexcept {
        return request_binding_;
    }
    [[nodiscard]] std::uint64_t angel_state_seal() const noexcept {
        return angel_state_seal_;
    }
    [[nodiscard]] std::uint64_t source_program_seal() const noexcept {
        return source_program_seal_;
    }
    [[nodiscard]] std::uint64_t source_hash() const noexcept {
        return source_hash_;
    }
    [[nodiscard]] std::uint64_t target_hash() const noexcept {
        return target_hash_;
    }
    [[nodiscard]] std::uint64_t valuation_hash() const noexcept {
        return valuation_hash_;
    }
    [[nodiscard]] const ExternalCoordinateDimension& coordinate() const noexcept {
        return coordinate_;
    }
    [[nodiscard]] std::uint64_t seal() const noexcept { return view_seal_; }

    // These are frozen semantic facts, not mutable accounting flags.
    [[nodiscard]] static constexpr bool contains_angel_state() noexcept {
        return false;
    }
    [[nodiscard]] static constexpr bool compresses_angel_state() noexcept {
        return false;
    }
    [[nodiscard]] static constexpr std::uint64_t angel_nodes_rewritten() noexcept {
        return 0U;
    }
    [[nodiscard]] static constexpr std::uint64_t angel_nodes_merged() noexcept {
        return 0U;
    }
    [[nodiscard]] static constexpr bool can_feed_back_to_angel() noexcept {
        return false;
    }

private:
    std::uint64_t candidate_{};
    std::uint64_t modulus_{};
    std::uint64_t request_binding_{};
    std::uint64_t angel_state_seal_{};
    std::uint64_t source_program_seal_{};
    std::uint64_t source_hash_{};
    std::uint64_t target_hash_{};
    std::uint64_t valuation_hash_{};
    ExternalCoordinateDimension coordinate_{};
    std::uint64_t view_seal_{};

    CertifiedExternalQuotientView(
        const std::uint64_t candidate,
        const std::uint64_t request_binding,
        const angel::afac57::CertifiedPrincipalJetState& state,
        ExternalCoordinateDimension coordinate,
        const std::uint64_t view_seal)
        : candidate_(candidate), modulus_(candidate),
          request_binding_(request_binding), angel_state_seal_(state.seal()),
          source_program_seal_(state.source_program().program().seal()),
          source_hash_(state.source().stable_hash()),
          target_hash_(state.target().stable_hash()),
          valuation_hash_(state.valuation().stable_hash()),
          coordinate_(std::move(coordinate)), view_seal_(view_seal) {}

    friend class ExternalQuotientBoundary;
    friend class ExternalQuotientViewVerifier;
};

[[nodiscard]] inline std::uint64_t external_view_seal(
    const std::uint64_t candidate,
    const std::uint64_t request_binding,
    const angel::afac57::CertifiedPrincipalJetState& state,
    const ExternalCoordinateDimension& coordinate) noexcept {
    return angel::afac56::mix64(
        candidate ^ std::rotl(request_binding, 7) ^
        std::rotl(state.seal(), 17) ^
        std::rotl(state.source_program().program().seal(), 27) ^
        std::rotl(state.source().stable_hash(), 37) ^
        std::rotl(state.target().stable_hash(), 43) ^
        std::rotl(state.valuation().stable_hash(), 51) ^
        std::rotl(external_coordinate_seal(coordinate), 59) ^
        0x4558545156494557ULL);
}

class ExternalQuotientBoundary final {
public:
    [[nodiscard]] CertifiedExternalQuotientView bind(
        const angel::afac58::CertifiedWilsonRequest& request) const {
        if (!angel::afac58::WilsonRequestVerifier::verify(request).accepted)
            throw std::invalid_argument(
                "R60 cannot bind an invalid R58/R57 request");
        const auto coordinate = natural_coordinate_dimension(
            request.candidate() - 1U);
        const auto& state = request.factorial_execution().principal_jet;
        const auto seal = external_view_seal(
            request.candidate(), request.binding_seal(), state, coordinate);
        return CertifiedExternalQuotientView{
            request.candidate(), request.binding_seal(), state,
            coordinate, seal};
    }
};

struct ExternalQuotientViewVerification final {
    bool request_valid{};
    bool request_binding_valid{};
    bool complete_state_identity_valid{};
    bool coordinate_valid{};
    bool seal_valid{};
    bool no_state_compression{};
    bool no_state_rewrite{};
    bool no_state_merge{};
    bool no_feedback_channel{};
    bool accepted{};
};

class ExternalQuotientViewVerifier final {
public:
    [[nodiscard]] static ExternalQuotientViewVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedExternalQuotientView& view) noexcept {
        ExternalQuotientViewVerification out{};
        out.request_valid =
            angel::afac58::WilsonRequestVerifier::verify(request).accepted;
        const auto& state = request.factorial_execution().principal_jet;
        out.request_binding_valid =
            view.candidate_ == request.candidate() &&
            view.modulus_ == request.candidate() &&
            view.request_binding_ == request.binding_seal();
        out.complete_state_identity_valid =
            view.angel_state_seal_ == state.seal() &&
            view.source_program_seal_ == state.source_program().program().seal() &&
            view.source_hash_ == state.source().stable_hash() &&
            view.target_hash_ == state.target().stable_hash() &&
            view.valuation_hash_ == state.valuation().stable_hash();
        const auto expected = natural_coordinate_dimension(
            request.candidate() - 1U);
        out.coordinate_valid = view.coordinate_ == expected &&
                               !view.coordinate_.polylogarithmic_summary_claimed &&
                               view.coordinate_.tombstoned_as_polylog_candidate;
        out.seal_valid = view.view_seal_ == external_view_seal(
            request.candidate(), request.binding_seal(), state,
            view.coordinate_);
        out.no_state_compression = !view.compresses_angel_state() &&
                                   !view.contains_angel_state();
        out.no_state_rewrite = view.angel_nodes_rewritten() == 0U;
        out.no_state_merge = view.angel_nodes_merged() == 0U;
        out.no_feedback_channel = !view.can_feed_back_to_angel();
        out.accepted = out.request_valid && out.request_binding_valid &&
                       out.complete_state_identity_valid &&
                       out.coordinate_valid && out.seal_valid &&
                       out.no_state_compression && out.no_state_rewrite &&
                       out.no_state_merge && out.no_feedback_channel;
        return out;
    }
};

struct AngelStateIntegrityWitness final {
    std::uint64_t request_binding_before{};
    std::uint64_t request_binding_after{};
    std::uint64_t state_seal_before{};
    std::uint64_t state_seal_after{};
    std::uint64_t source_program_seal_before{};
    std::uint64_t source_program_seal_after{};
    std::uint64_t certificate_seal_before{};
    std::uint64_t certificate_seal_after{};
    std::uint64_t payload_bytes_before{};
    std::uint64_t payload_bytes_after{};
    std::uint64_t angel_nodes_rewritten{};
    std::uint64_t angel_nodes_merged{};
    bool angel_state_compressed{};
    bool ordinary_result_fed_back_to_angel{};
    std::uint64_t witness_seal{};

    friend bool operator==(const AngelStateIntegrityWitness&,
                           const AngelStateIntegrityWitness&) = default;
};

[[nodiscard]] inline std::uint64_t integrity_witness_seal(
    const AngelStateIntegrityWitness& witness) noexcept {
    return angel::afac56::mix64(
        witness.request_binding_before ^
        std::rotl(witness.request_binding_after, 5) ^
        std::rotl(witness.state_seal_before, 11) ^
        std::rotl(witness.state_seal_after, 17) ^
        std::rotl(witness.source_program_seal_before, 23) ^
        std::rotl(witness.source_program_seal_after, 29) ^
        std::rotl(witness.certificate_seal_before, 35) ^
        std::rotl(witness.certificate_seal_after, 41) ^
        std::rotl(witness.payload_bytes_before, 47) ^
        std::rotl(witness.payload_bytes_after, 53) ^
        std::rotl(witness.angel_nodes_rewritten, 57) ^
        std::rotl(witness.angel_nodes_merged, 59) ^
        std::rotl(static_cast<std::uint64_t>(
            witness.angel_state_compressed), 61) ^
        std::rotl(static_cast<std::uint64_t>(
            witness.ordinary_result_fed_back_to_angel), 63) ^
        0x494e544547523630ULL);
}

[[nodiscard]] inline bool verify_integrity_witness(
    const angel::afac58::CertifiedWilsonRequest& request,
    const AngelStateIntegrityWitness& witness) noexcept {
    const auto& state = request.factorial_execution().principal_jet;
    return witness.request_binding_before == request.binding_seal() &&
           witness.request_binding_after == request.binding_seal() &&
           witness.state_seal_before == state.seal() &&
           witness.state_seal_after == state.seal() &&
           witness.source_program_seal_before ==
               state.source_program().program().seal() &&
           witness.source_program_seal_after ==
               state.source_program().program().seal() &&
           witness.certificate_seal_before == state.certificate().certificate_seal &&
           witness.certificate_seal_after == state.certificate().certificate_seal &&
           witness.payload_bytes_before == state.payload_bytes() &&
           witness.payload_bytes_after == state.payload_bytes() &&
           witness.angel_nodes_rewritten == 0U &&
           witness.angel_nodes_merged == 0U &&
           !witness.angel_state_compressed &&
           !witness.ordinary_result_fed_back_to_angel &&
           witness.witness_seal == integrity_witness_seal(witness);
}

struct ExternalQuotientDownload final {
    angel::afac59::SublinearWilsonDownload ordinary_download;
    AngelStateIntegrityWitness integrity;
};

class ExternalQuotientObserver final {
public:
    [[nodiscard]] ExternalQuotientDownload download(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedExternalQuotientView& view,
        angel::afac59::SublinearWilsonPolicy policy =
            angel::afac59::SublinearWilsonPolicy::production()) const {
        if (!ExternalQuotientViewVerifier::verify(request, view).accepted)
            throw std::invalid_argument("R60 external view binding rejected");

        // The view is bound to the natural sqrt coordinate.  Any non-zero
        // override must name the same coordinate; this prevents a descriptor
        // from certifying a different evaluation schedule.
        if (policy.block_width_override != 0U &&
            policy.block_width_override != view.coordinate().block_width)
            throw std::invalid_argument("R60 view/policy coordinate mismatch");

        const auto before = snapshot(request);
        angel::afac59::SublinearWilsonObserver observer;
        auto ordinary = observer.download(request, policy);
        auto witness = snapshot(request);
        witness.request_binding_before = before.request_binding_before;
        witness.state_seal_before = before.state_seal_before;
        witness.source_program_seal_before = before.source_program_seal_before;
        witness.certificate_seal_before = before.certificate_seal_before;
        witness.payload_bytes_before = before.payload_bytes_before;
        witness.witness_seal = integrity_witness_seal(witness);
        return ExternalQuotientDownload{
            std::move(ordinary), std::move(witness)};
    }

    ExternalQuotientDownload download(
        const CertifiedExternalQuotientView&) const = delete;

private:
    [[nodiscard]] static AngelStateIntegrityWitness snapshot(
        const angel::afac58::CertifiedWilsonRequest& request) noexcept {
        const auto& state = request.factorial_execution().principal_jet;
        AngelStateIntegrityWitness witness{};
        witness.request_binding_before = request.binding_seal();
        witness.request_binding_after = request.binding_seal();
        witness.state_seal_before = state.seal();
        witness.state_seal_after = state.seal();
        witness.source_program_seal_before =
            state.source_program().program().seal();
        witness.source_program_seal_after =
            state.source_program().program().seal();
        witness.certificate_seal_before = state.certificate().certificate_seal;
        witness.certificate_seal_after = state.certificate().certificate_seal;
        witness.payload_bytes_before = state.payload_bytes();
        witness.payload_bytes_after = state.payload_bytes();
        witness.angel_nodes_rewritten = 0U;
        witness.angel_nodes_merged = 0U;
        witness.angel_state_compressed = false;
        witness.ordinary_result_fed_back_to_angel = false;
        return witness;
    }
};

struct ExternalQuotientVerification final {
    bool view_valid{};
    bool ordinary_download_valid{};
    bool state_integrity_valid{};
    bool no_ordinary_feedback{};
    bool accepted{};
};

class ExternalQuotientVerifier final {
public:
    [[nodiscard]] static ExternalQuotientVerification verify(
        const angel::afac58::CertifiedWilsonRequest& request,
        const CertifiedExternalQuotientView& view,
        const ExternalQuotientDownload& download) noexcept {
        ExternalQuotientVerification out{};
        out.view_valid =
            ExternalQuotientViewVerifier::verify(request, view).accepted;
        if (const auto* observation =
                std::get_if<angel::afac59::SublinearWilsonObservation>(
                    &download.ordinary_download)) {
            out.ordinary_download_valid =
                angel::afac59::SublinearWilsonVerifier::verify(
                    request, *observation).accepted;
            out.no_ordinary_feedback =
                !observation->ledger().ordinary_result_fed_back_to_angel;
        } else {
            const auto& limit =
                std::get<angel::afac59::SublinearWilsonResourceLimit>(
                    download.ordinary_download);
            out.ordinary_download_valid =
                angel::afac59::SublinearWilsonVerifier::verify(request, limit);
            out.no_ordinary_feedback =
                !limit.ledger.ordinary_result_fed_back_to_angel;
        }
        out.state_integrity_valid =
            verify_integrity_witness(request, download.integrity);
        out.accepted = out.view_valid && out.ordinary_download_valid &&
                       out.state_integrity_valid && out.no_ordinary_feedback;
        return out;
    }
};

struct R60ApplicationResult final {
    angel::afac58::CertifiedWilsonRequest request;
    CertifiedExternalQuotientView view;
    ExternalQuotientDownload download;
};

[[nodiscard]] inline R60ApplicationResult ANGEL_WILSON_PRIME_EXTERNAL_VIEW(
    const std::uint64_t candidate,
    angel::afac59::SublinearWilsonPolicy policy =
        angel::afac59::SublinearWilsonPolicy::production()) {
    angel::afac58::WilsonBoundary angel_boundary;
    ExternalQuotientBoundary external_boundary;
    ExternalQuotientObserver observer;
    auto request = angel_boundary.bind(candidate);
    auto view = external_boundary.bind(request);
    auto download = observer.download(request, view, policy);
    return R60ApplicationResult{
        std::move(request), std::move(view), std::move(download)};
}

// Named forbidden bridges.  Their signatures make boundary violations fail
// at compile time instead of becoming a convention hidden in documentation.
angel::afac57::CertifiedPrincipalJetState compress_angel_state(
    const CertifiedExternalQuotientView&) = delete;
angel::afac59::AngelRunningState feed_external_quotient_back_to_angel(
    const ExternalQuotientDownload&) = delete;
CertifiedExternalQuotientView implicit_external_view(
    const angel::afac58::CertifiedWilsonRequest&) = delete;

static_assert(!std::is_default_constructible_v<CertifiedExternalQuotientView>);
static_assert(!std::is_constructible_v<
    angel::afac57::CertifiedPrincipalJetState,
    CertifiedExternalQuotientView>);
static_assert(!std::is_convertible_v<
    CertifiedExternalQuotientView,
    angel::afac57::CertifiedPrincipalJetState>);

} // namespace angel::afac60
