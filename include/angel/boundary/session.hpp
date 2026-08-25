#pragma once

#include <cstdint>
#include <memory>

namespace angel::detail {
struct AuthorityModel;
struct BoundaryAccess;
}

namespace angel::boundary {

struct SessionId final {
    std::uint64_t authority{};
    std::uint64_t sequence{};

    [[nodiscard]] bool valid() const noexcept {
        return authority != 0U && sequence != 0U;
    }
    friend bool operator==(const SessionId&, const SessionId&) = default;
};

class SessionAuthority final {
public:
    explicit SessionAuthority(std::uint64_t authority);
    SessionAuthority(const SessionAuthority&) noexcept = default;
    SessionAuthority(SessionAuthority&&) noexcept = default;
    SessionAuthority& operator=(const SessionAuthority&) noexcept = default;
    SessionAuthority& operator=(SessionAuthority&&) noexcept = default;
    ~SessionAuthority() = default;

private:
    std::shared_ptr<detail::AuthorityModel> model_;
    friend struct detail::BoundaryAccess;
};

} // namespace angel::boundary
