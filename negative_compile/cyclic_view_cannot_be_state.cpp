#include "angel/cyclic_structure.hpp"

int main() {
    using namespace angel::prime;
    const auto state = candidate(17U) | upload_factorial_state();
    const auto view = state | bind_cyclic_action();
    const FactorialState illegal = view;
    static_cast<void>(illegal);
}
