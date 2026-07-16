#include "FriendDAO.hpp"
#include <utility>

namespace arcane::database {

FriendDAO::FriendDAO(std::shared_ptr<DBConnection> connection) noexcept : connection_(std::move(connection)) {}

bool FriendDAO::createFriendship(const FriendshipRecord& f) {
    return connection_->execute(
        "INSERT INTO friendships (user_id, friend_id, relation_status) VALUES (?, ?, ?)",
        {std::to_string(f.userId), std::to_string(f.friendId), f.relationStatus});
}

std::vector<FriendshipRecord> FriendDAO::getFriends(std::uint64_t userId) const {
    const auto r = connection_->query("SELECT * FROM friendships WHERE user_id = ?", {std::to_string(userId)});
    static_cast<void>(r); return {};
}

bool FriendDAO::updateFriendshipStatus(std::uint64_t uid, std::uint64_t fid, const std::string& s) {
    return connection_->execute(
        "UPDATE friendships SET relation_status = ? WHERE user_id = ? AND friend_id = ?",
        {s, std::to_string(uid), std::to_string(fid)});
}

bool FriendDAO::deleteFriendship(std::uint64_t uid, std::uint64_t fid) {
    return connection_->execute("DELETE FROM friendships WHERE user_id = ? AND friend_id = ?",
                                {std::to_string(uid), std::to_string(fid)});
}

} // namespace arcane::database
