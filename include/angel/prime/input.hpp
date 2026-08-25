#pragma once

#include <cstdint>
#include <stdexcept>

namespace angel::prime {

struct Candidate final {
    std::uint64_t value{};

    explicit Candidate(const std::uint64_t candidate) : value(candidate) {
        if (value < 2U)
            throw std::invalid_argument("a Wilson candidate must be at least two");
    }
};

[[nodiscard]] inline Candidate candidate(const std::uint64_t value) {
    return Candidate{value};
}

} // namespace angel::prime
