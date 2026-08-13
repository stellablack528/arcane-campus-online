#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace arcane::database {

struct UserRecord {
    std::uint64_t userId = 0;
    std::string username;
    std::string passwordHash;
    std::string email;
    std::string avatar;
    std::string createdAt;
    std::string lastLoginAt;
    std::string status;
};

class UserDAO final {
public:
    explicit UserDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool createUser(const UserRecord& user);
    [[nodiscard]] std::optional<UserRecord> getUserById(std::uint64_t userId) const;
    [[nodiscard]] std::optional<UserRecord> getUserByName(const std::string& username) const;
    [[nodiscard]] bool updateLastLogin(std::uint64_t userId);
    [[nodiscard]] bool deleteUser(std::uint64_t userId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
