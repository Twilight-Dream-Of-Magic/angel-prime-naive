#include "angel/prime.hpp"

#include <iostream>

int main() {
    using namespace angel::prime;

    const auto result =
        candidate(1009U)
        | upload_factorial_state()
        | bind_quotient_view()
        | download_wilson();

    if (const auto* value = std::get_if<WilsonObservation>(&result.observation)) {
        std::cout << "candidate=" << value->candidate << '\n';
        std::cout << "factorial_residue=" << value->factorial_residue << '\n';
        std::cout << "prime=" << (value->prime ? "yes" : "no") << '\n';
    } else {
        const auto& limit = std::get<ResourceLimit>(result.observation);
        std::cout << "resource_limit=" << limit.required_block_width << '\n';
    }
    std::cout << "state_preserved="
              << (result.integrity.preserved() ? "yes" : "no") << '\n';
    return 0;
}
