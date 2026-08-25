#include "angel/boundary.hpp"

#include <iostream>

int main() {
    using namespace angel::boundary;

    SessionAuthority authority{0xCA550001U};
    BoundaryLedger ledger{};
    const auto packet =
        EncodedOrder::canonical(24U)
        | upload(authority, &ledger)
        | quotient_to(6U, &ledger)
        | continue_to(4U, &ledger)
        | observe_primitive(4U, 998244353U, 1U, &ledger)
        | download(&ledger);

    const auto& observation = std::get<PrimitiveObservation>(packet.observation);
    std::cout << "order=" << observation.order << '\n';
    std::cout << "first_visible_jet=" << observation.first_visible_jet << '\n';
    std::cout << "resume_capability="
              << (packet.cut.no_resume_capability ? "none" : "present") << '\n';
    std::cout << "state_nodes_rewritten=" << ledger.nodes_rewritten << '\n';
    return 0;
}
