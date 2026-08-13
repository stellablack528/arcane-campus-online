#include "UserDAO.hpp"
#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

UserDAO::UserDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool UserDAO::createUser(const UserRecord& user)
{
    return connection_->execute(
        "INSERT INTO users (username, password_hash, email, avatar, status) VALUES (?, ?, ?, ?, ?)",
        {user.username, user.passwordHash, user.email, user.avatar, user.status});
}

std::optional<UserRecord> UserDAO::getUserById(std::uint64_t userId) const
{
    const auto result = connection_->query("SELECT * FROM users WHERE user_id = ?", {std::to_string(userId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return UserRecord{detail::integer<std::uint64_t>(row, "user_id"), detail::value(row, "username"),
                      detail::value(row, "password_hash"), detail::value(row, "email"),
                      detail::value(row, "avatar"), detail::value(row, "created_at"),
                      detail::value(row, "last_login_at"), detail::value(row, "status")};
}

std::optional<UserRecord> UserDAO::getUserByName(const std::string& username) const
{
    const auto result = connection_->query("SELECT * FROM users WHERE username = ?", {username});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return UserRecord{detail::integer<std::uint64_t>(row, "user_id"), detail::value(row, "username"),
                      detail::value(row, "password_hash"), detail::value(row, "email"),
                      detail::value(row, "avatar"), detail::value(row, "created_at"),
                      detail::value(row, "last_login_at"), detail::value(row, "status")};
}

bool UserDAO::updateLastLogin(std::uint64_t userId)
{
    return connection_->execute("UPDATE users SET last_login_at = CURRENT_TIMESTAMP WHERE user_id = ?",
                                {std::to_string(userId)});
}

bool UserDAO::deleteUser(std::uint64_t userId)
{
    return connection_->execute("DELETE FROM users WHERE user_id = ?", {std::to_string(userId)});
}

} // namespace arcane::database
