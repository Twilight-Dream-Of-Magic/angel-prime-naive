#include "angel/joint_wilson.hpp"

int main() {
    using namespace angel::prime;
    const auto result = candidate(101U) | project_wilson_jointly();
    static_cast<void>(result);
}
