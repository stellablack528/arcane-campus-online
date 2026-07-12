#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace arcane::database {

struct FriendshipRecord {
    std::uint64_t userId = 0;
    std::uint64_t friendId = 0;
    std::string relationStatus;
    std::string createTime;
};

class FriendDAO final {
public:
    explicit FriendDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool createFriendship(const FriendshipRecord& friendship);
    [[nodiscard]] std::vector<FriendshipRecord> getFriends(std::uint64_t userId) const;
    [[nodiscard]] bool updateFriendshipStatus(std::uint64_t userId,
                                              std::uint64_t friendId,
                                              const std::string& relationStatus);
    [[nodiscard]] bool deleteFriendship(std::uint64_t userId, std::uint64_t friendId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
