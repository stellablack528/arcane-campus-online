#include "UserDAO.hpp"
#include <utility>

namespace arcane::database {

UserDAO::UserDAO(std::shared_ptr<DBConnection> connection) noexcept : connection_(std::move(connection)) {}

bool UserDAO::createUser(const UserRecord& user) {
    return connection_->execute(
        "INSERT INTO users (username, password_hash, email, avatar, status) VALUES (?, ?, ?, ?, ?)",
        {user.username, user.passwordHash, user.email, user.avatar, user.status});
}

std::optional<UserRecord> UserDAO::getUserById(std::uint64_t userId) const {
    const auto result = connection_->query("SELECT * FROM users WHERE user_id = ?", {std::to_string(userId)});
    static_cast<void>(result); return std::nullopt;
}

std::optional<UserRecord> UserDAO::getUserByName(const std::string& username) const {
    const auto result = connection_->query("SELECT * FROM users WHERE username = ?", {username});
    static_cast<void>(result); return std::nullopt;
}

bool UserDAO::updateLastLogin(std::uint64_t userId) {
    return connection_->execute("UPDATE users SET last_login_at = CURRENT_TIMESTAMP WHERE user_id = ?", {std::to_string(userId)});
}

bool UserDAO::deleteUser(std::uint64_t userId) {
    return connection_->execute("DELETE FROM users WHERE user_id = ?", {std::to_string(userId)});
}

} // namespace arcane::database
