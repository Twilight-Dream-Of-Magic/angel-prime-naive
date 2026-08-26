#include "angel/high_dimensional.hpp"

int main() {
    angel::high::DerivedObservation observation{};
    auto illegal_state = angel::high::promote_observation(observation);
    (void)illegal_state;
}
