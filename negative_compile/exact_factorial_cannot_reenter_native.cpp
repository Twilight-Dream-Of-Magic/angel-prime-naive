#include "angel/native_factorial.hpp"

int main() {
    using namespace angel::prime;
    const auto exact = candidate(17U)
                     | upload_factorial_state()
                     | bind_native_factorial()
                     | derive_exact_factorial();
    const auto illegal = exact | bind_quotient_view();
    static_cast<void>(illegal);
}
