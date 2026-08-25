#include "angel/cyclic_structure.hpp"

int main() {
    using namespace angel::prime;
    const auto closed = candidate(17U)
                      | upload_factorial_state()
                      | bind_cyclic_action()
                      | evaluate_cyclic_action();
    const auto illegal = closed | download_wilson();
    static_cast<void>(illegal);
}
