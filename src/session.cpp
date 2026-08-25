#include "angel/boundary/session.hpp"
#include "internal/models.hpp"

#include <memory>

namespace angel::boundary {

SessionAuthority::SessionAuthority(const std::uint64_t authority)
    : model_(std::make_shared<detail::AuthorityModel>(authority)) {}

} // namespace angel::boundary
