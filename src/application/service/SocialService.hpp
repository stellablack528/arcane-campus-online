#pragma once

#include "application/vo/CampusVO.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace arcane::database {
class FriendDAO;
class CharacterDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Handles social interactions: starting private chats and viewing profiles.
// When DAOs are injected, friend relationships and character data are read
// from the database; otherwise a simple validation is performed.
class SocialService final {
public:
    explicit SocialService(std::shared_ptr<database::FriendDAO> friendDao,
                           std::shared_ptr<database::CharacterDAO> characterDao) noexcept;

    // Check whether two users are friends (or have a pending request) before
    // allowing a private chat session to start.
    [[nodiscard]] vo::OperationResultVO startPrivateChat(const std::string& memberId) const;

    // Look up a character profile by its id. Falls back to a stub message
    // when no DAO is available.
    [[nodiscard]] vo::OperationResultVO viewProfile(const std::string& memberId) const;

private:
    std::shared_ptr<database::FriendDAO> friendDao_;
    std::shared_ptr<database::CharacterDAO> characterDao_;
};

} // namespace arcane::application::service
