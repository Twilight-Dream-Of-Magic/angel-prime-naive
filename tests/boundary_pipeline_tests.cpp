#include "angel/boundary.hpp"
#include "angel/diagnostics.hpp"
#include "angel/observation_integrity.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace angel::boundary;

void require(const bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

EncodedOrder padded_big_endian(const std::uint64_t value) {
    EncodedOrder encoded{};
    encoded.byte_order = ByteOrder::BigEndian;
    encoded.byte_count = 3U;
    encoded.bytes[2U] = static_cast<std::uint8_t>(value);
    return encoded;
}

void verify_normalization() {
    const auto canonical = normalize(EncodedOrder::canonical(15U));
    const auto padded = normalize(padded_big_endian(15U));
    require(canonical.has_value() && padded.has_value(), "normalization failed");
    require(canonical->canonical_order == padded->canonical_order,
            "equivalent encodings diverged");
    require(padded->removed_high_zero_bytes == 2U && padded->endian_reordered,
            "normalization evidence is incomplete");

    EncodedOrder invalid{};
    invalid.bytes[0U] = 1U;
    require(!normalize(invalid).has_value(), "order one was accepted");
}

void verify_native_pipeline() {
    SessionAuthority authority{0x630004U};
    BoundaryLedger ledger{};

    const auto high = EncodedOrder::canonical(24U) | upload(authority, &ledger);
    const auto descended = high | quotient_to(6U, &ledger);
    const auto ready = descended | continue_to(4U, &ledger);

    require(high.summary().cycles == 1U, "fresh upload is not least committed");
    require(descended.summary().cycles == 4U, "ramification was discarded");
    require(descended.summary().session == high.summary().session,
            "native quotient cut the session");
    require(ready.summary().available_jet_order == 4U,
            "continuation did not retain the requested horizon");

    const auto closed = ready | observe_primitive(4U, 998244353U, 1U, &ledger);
    const auto dense_updates = ledger.dense_reference_updates;
    const auto packet = closed | download(&ledger);
    require(validate_download_packet(closed, packet).accepted(),
            "canonical download packet validation failed");
    require(ledger.dense_reference_updates == dense_updates,
            "download performed deferred execution");
    const auto* primitive = std::get_if<PrimitiveObservation>(&packet.observation);
    require(primitive != nullptr && primitive->first_visible_jet == 4U,
            "primitive closure did not bind the exact horizon");
    require(std::any_of(
                primitive->primitive_column.begin(),
                primitive->primitive_column.end(),
                [](const auto value) { return value != 0U; }),
            "primitive payload is zero");
    require(packet.cut.no_resume_capability &&
                packet.cut.fresh_upload_required_for_reentry,
            "ordinary download retained a native resume capability");
    require(ledger.nodes_rewritten == 0U && ledger.nodes_merged == 0U &&
                !ledger.ordinary_feedback,
            "the wrapper modified or fed back into state");

    const auto canonical_before = ready.summary();
    auto tampered = packet;
    auto& tampered_primitive =
        std::get<PrimitiveObservation>(tampered.observation);
    tampered_primitive.primitive_column.front() ^= 1U;
    require(!validate_download_packet(closed, tampered).accepted(),
            "tampered derived observation map was accepted");
    const auto canonical_after = ready.summary();
    require(canonical_before.order == canonical_after.order &&
                canonical_before.cycles == canonical_after.cycles &&
                canonical_before.session == canonical_after.session &&
                canonical_before.available_jet_order ==
                    canonical_after.available_jet_order,
            "derived observation tampering changed canonical state");
}

void verify_presentation_and_checkpoint() {
    SessionAuthority authority{0x630005U};
    BoundaryLedger ledger{};
    const auto standard =
        EncodedOrder::canonical(15U) | upload(authority, &ledger);
    PresentationEvidence evidence{};
    const auto alternate = standard |
        change_presentation(Presentation{15U, 2U, 8U}, &evidence, &ledger);
    require(evidence.accepted && evidence.exact_inverse,
            "presentation evidence failed");
    require(standard.presentation_equivalent(alternate),
            "equivalent presentation was rejected");
    require(!standard.exactly_equal(alternate),
            "different presentations became field-identical");

    const auto continued = alternate | continue_to(3U, &ledger);
    const auto checkpoint = continued | export_checkpoint(&ledger);
    const auto restored = checkpoint | import_checkpoint(&ledger);
    const auto before = continued.summary();
    const auto after = restored.summary();
    require(before.order == after.order && before.cycles == after.cycles &&
                before.session == after.session &&
                before.available_jet_order == after.available_jet_order,
            "native checkpoint did not preserve semantic state");
    require(after.origin == Origin::CheckpointImport,
            "checkpoint import provenance was not recorded");
}

void verify_fresh_reentry_and_evidence() {
    SessionAuthority authority{0x630006U};
    const auto high = EncodedOrder::canonical(24U) | upload(authority);
    const auto descended = high | quotient_to(6U);
    const auto packet = descended | observe_order() | download();
    const auto order = std::get<OrderObservation>(packet.observation).order;
    const auto fresh = EncodedOrder::canonical(order) | upload(authority);
    require(fresh.summary().cycles == 1U && descended.summary().cycles == 4U,
            "fresh reentry resumed ramification");
    require(fresh.summary().session != descended.summary().session,
            "fresh reentry reused a cut session");

    const auto least = verify_least_commitment(fresh, descended);
    require(least.accepted, "least commitment evidence failed");
    const auto transport = verify_ramified_transport(24U, 6U);
    require(transport.accepted && transport.ramification_index == 4U,
            "ramified transport evidence failed");
}

void verify_rejection() {
    SessionAuthority authority{0x630007U};
    const auto state = EncodedOrder::canonical(12U) | upload(authority);
    bool rejected = false;
    try {
        static_cast<void>(state | quotient_to(5U));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "non-divisor quotient was accepted");
}

} // namespace

int main() {
    verify_normalization();
    verify_native_pipeline();
    verify_presentation_and_checkpoint();
    verify_fresh_reentry_and_evidence();
    verify_rejection();

    const auto layout = angel::diagnostics::frozen_layout();
    require(layout.exact_arithmetic_state_held &&
                layout.exact_boundary_state_held &&
                !layout.wrapper_compresses_state,
            "frozen state storage contract failed");

    std::cout << "BOUNDARY_PIPELINE_TESTS=PASS\n";
    std::cout << "EXACT_FROZEN_STATE_HELD=YES\n";
    std::cout << "STATE_COMPRESSION=FALSE\n";
    std::cout << "ORDINARY_FEEDBACK=FALSE\n";
    return 0;
}
