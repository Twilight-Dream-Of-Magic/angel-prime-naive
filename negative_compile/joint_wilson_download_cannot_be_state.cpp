#include "angel/joint_wilson.hpp"

int main() {
    using namespace angel::prime;
    const auto result = candidate(101U)
        | upload_factorial_state()
        | bind_native_factorial()
        | project_wilson_jointly();
    FactorialState state = result;
    static_cast<void>(state);
}
