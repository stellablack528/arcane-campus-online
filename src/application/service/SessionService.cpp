#include "application/service/SessionService.hpp"

#include "UserDAO.hpp"
#include "CharacterDAO.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <utility>

namespace arcane::application::service {

namespace {
constexpr std::array<std::string_view, 4> kHouses{
    "Ravenclaw", "Gryffindor", "Hufflepuff", "Slytherin"};

bool isValidHouse(const std::string& house)
{
    return std::ranges::find(kHouses, house) != kHouses.end();
}
} // namespace

SessionService::SessionService(std::shared_ptr<database::UserDAO> userDao,
                               std::shared_ptr<database::CharacterDAO> characterDao) noexcept
    : userDao_(std::move(userDao))
    , characterDao_(std::move(characterDao))
{
}

vo::LoginVO SessionService::enterCampus(const dto::LoginRequestDTO& request)
{
    if (request.studentName.empty() || !isValidHouse(request.house)) {
        return {false, "Please provide a student name and a valid house.", {}, {}, {}};
    }

    // Database-backed login: verify the student exists and matches the selected house.
    if (userDao_ && characterDao_) {
        const auto user = userDao_->getUserByName(request.studentName);
        if (!user) {
            return {false,
                    "No student record found for '" + request.studentName
                        + "'. Contact the headmaster to enroll.",
                    {},
                    {},
                    {}};
        }
        const auto character = characterDao_->getCharacterByUserId(user->userId);
        if (!character) {
            return {false,
                    "The student record exists but no character has been created yet.",
                    {},
                    {},
                    {}};
        }
        if (character->house != request.house) {
            return {false,
                    "This student belongs to " + character->house + ", not " + request.house + ".",
                    {},
                    {},
                    {}};
        }

        userDao_->updateLastLogin(user->userId);

        session_ = do_model::PlayerSessionDO{user->userId,
                                             character->characterId,
                                             character->nickname,
                                             character->house,
                                             "Great Hall",
                                             "Breakfast",
                                             true};
        return {true, "Welcome back to Hogwarts Online.", session_->nickname, session_->house,
                session_->currentLocation};
    }

    // Demo fallback: no DAO wired in, accept any valid name/house combination.
    session_ = do_model::PlayerSessionDO{0, 0, request.studentName, request.house,
                                         "Great Hall", "Breakfast", true};
    return {true, "Welcome to Hogwarts Online.", session_->nickname, session_->house,
            session_->currentLocation};
}

std::optional<do_model::PlayerSessionDO> SessionService::currentSession() const
{
    return session_;
}

do_model::PlayerSessionDO* SessionService::activeSession() noexcept
{
    return session_ ? &*session_ : nullptr;
}

void SessionService::endSession() noexcept
{
    session_.reset();
}

} // namespace arcane::application::service
