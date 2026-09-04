#include "application/service/SocialService.hpp"

#include "CharacterDAO.hpp"
#include "FriendDAO.hpp"

#include <utility>

namespace arcane::application::service {

SocialService::SocialService(std::shared_ptr<database::FriendDAO> friendDao,
                             std::shared_ptr<database::CharacterDAO> characterDao) noexcept
    : friendDao_(std::move(friendDao))
    , characterDao_(std::move(characterDao))
{
}

vo::OperationResultVO SocialService::startPrivateChat(const std::string& memberId) const
{
    if (memberId.empty()) {
        return {false, "Select a campus member before opening a private chat."};
    }

    // Database-backed path: verify the two characters are friends.
    // Currently we only check that the member id resolves to a character;
    // a full friend-relationship check will be added once sessions carry
    // the active user id into this method.
    if (characterDao_) {
        try {
            const auto id = std::stoull(memberId);
            const auto character = characterDao_->getCharacterById(id);
            if (!character) {
                return {false, "That campus member could not be found."};
            }
            return {true, "Private chat started with " + character->nickname + "."};
        } catch (...) {
            return {false, "Invalid member id."};
        }
    }

    return {true, "Private chat requested for " + memberId + "."};
}

vo::OperationResultVO SocialService::viewProfile(const std::string& memberId) const
{
    if (memberId.empty()) {
        return {false, "Select a campus member before viewing a profile."};
    }

    if (characterDao_) {
        try {
            const auto id = std::stoull(memberId);
            const auto character = characterDao_->getCharacterById(id);
            if (!character) {
                return {false, "That campus member could not be found."};
            }
            const auto houseEmoji = character->house == "Gryffindor" ? "🦁"
                : character->house == "Slytherin" ? "🐍"
                : character->house == "Hufflepuff" ? "🦡"
                : character->house == "Ravenclaw" ? "🦅"
                : "✨";
            return {true, houseEmoji + " " + character->nickname + " | House: " + character->house
                        + " | Level: " + std::to_string(character->level)
                        + " | Title: " + character->title};
        } catch (...) {
            return {false, "Invalid member id."};
        }
    }

    return {true, "Profile requested for " + memberId + "."};
}

} // namespace arcane::application::service
