#include "angel/cyclic_structure.hpp"

int main() {
    using namespace angel::prime;
    const auto downloaded = candidate(17U)
                          | upload_factorial_state()
                          | bind_cyclic_action()
                          | evaluate_cyclic_action()
                          | download_cyclic_structure();
    const auto illegal = downloaded | bind_cyclic_action();
    static_cast<void>(illegal);
}
