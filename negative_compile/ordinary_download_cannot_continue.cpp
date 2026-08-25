#include "angel/boundary.hpp"

void forbidden(
    const angel::boundary::DownloadPacket& ordinary) {
    static_cast<void>(ordinary | angel::boundary::quotient_to(3U));
}
