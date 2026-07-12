#include "FriendDAO.hpp"

#include <utility>

namespace arcane::database {

FriendDAO::FriendDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool FriendDAO::createFriendship(const FriendshipRecord& friendship)
{
    return connection_->execute(
        "INSERT INTO friendships (user_id, friend_id, relation_status) VALUES (?, ?, ?)",
        {std::to_string(friendship.userId),
         std::to_string(friendship.friendId),
         friendship.relationStatus});
}

std::vector<FriendshipRecord> FriendDAO::getFriends(std::uint64_t userId) const
{
    const auto result = connection_->query("SELECT * FROM friendships WHERE user_id = ?",
                                           {std::to_string(userId)});
    static_cast<void>(result);
    return {};
}

bool FriendDAO::updateFriendshipStatus(std::uint64_t userId,
                                       std::uint64_t friendId,
                                       const std::string& relationStatus)
{
    return connection_->execute(
        "UPDATE friendships SET relation_status = ? WHERE user_id = ? AND friend_id = ?",
        {relationStatus, std::to_string(userId), std::to_string(friendId)});
}

bool FriendDAO::deleteFriendship(std::uint64_t userId, std::uint64_t friendId)
{
    return connection_->execute("DELETE FROM friendships WHERE user_id = ? AND friend_id = ?",
                                {std::to_string(userId), std::to_string(friendId)});
}

} // namespace arcane::database
