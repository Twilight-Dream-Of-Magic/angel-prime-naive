#include "angel/joint_wilson.hpp"

int main() {
    using namespace angel::prime;
    const auto state = candidate(101U) | upload_factorial_state();
    const auto result = state | project_wilson_jointly();
    static_cast<void>(result);
}
