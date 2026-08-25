#include "angel/prime.hpp"

void forbidden(const angel::prime::Download& ordinary) {
    static_cast<void>(ordinary | angel::prime::bind_quotient_view());
}
