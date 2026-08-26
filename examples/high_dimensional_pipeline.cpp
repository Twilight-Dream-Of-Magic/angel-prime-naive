#include "angel/high_dimensional.hpp"

#include <iostream>
#include <variant>

int main() {
    using namespace angel::high;

    const UploadFunctor upload;
    const auto source = upload(OrdinarySpecification{6});

    const auto history_shift = NativeFunctor::add(TriClassValue::history(17U));
    const auto scale = NativeFunctor::multiply(TriClassValue::ordinary(3));
    const auto causal_program = compose(scale, history_shift);
    const auto result = causal_program(source);
    if (!std::holds_alternative<MazeState>(result)) return 1;

    const auto& state = std::get<MazeState>(result);
    const DownloadFunctor download;
    const auto observation = download(state);
    if (!validate_observation(state, observation)) return 2;

    std::cout << "ordinary_coordinate=" << observation.ordinary_coordinate << '\n';
    std::cout << "history_depth=" << observation.history_depth << '\n';
    std::cout << "derived_non_authoritative="
              << (observation.derived_non_authoritative ? "YES" : "NO") << '\n';
    return 0;
}
