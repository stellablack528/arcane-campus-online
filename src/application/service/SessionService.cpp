#include "application/service/SessionService.hpp"

#include "UserDAO.hpp"
#include "CharacterDAO.hpp"
#include "InventoryDAO.hpp"

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
                               std::shared_ptr<database::CharacterDAO> characterDao,
                               std::shared_ptr<database::InventoryDAO> inventoryDao) noexcept
    : userDao_(std::move(userDao))
    , characterDao_(std::move(characterDao))
    , inventoryDao_(std::move(inventoryDao))
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
                        + "'. You can enroll as a new student via the Diagon Alley entrance.",
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

        (void)userDao_->updateLastLogin(user->userId);

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

vo::LoginVO SessionService::registerStudent(const dto::EnrollmentRequestDTO& request)
{
    if (request.username.empty() || request.nickname.empty()) {
        return {false, "Please provide a username and a character nickname.", {}, {}, {}};
    }
    if (!isValidHouse(request.house)) {
        return {false, "Please select a valid house.", {}, {}, {}};
    }
    if (!userDao_ || !characterDao_) {
        return {false, "Enrollment is not available in demo mode. Configure a database to enroll.", {}, {}, {}};
    }

    // 1. Check that the username is not already taken.
    if (userDao_->getUserByName(request.username)) {
        return {false, "That username is already taken. Choose another.", {}, {}, {}};
    }

    // 2. Create the user account.
    database::UserRecord userRecord;
    userRecord.username = request.username;
    userRecord.passwordHash = "plaintext_placeholder";  // TODO: real password hashing
    userRecord.email = request.username + "@hogwarts.edu";
    userRecord.avatar.clear();
    userRecord.status = "active";
    if (!userDao_->createUser(userRecord)) {
        return {false, "Failed to create your student record. Please try again.", {}, {}, {}};
    }
    const auto user = userDao_->getUserByName(request.username);
    if (!user) {
        return {false, "Your student record was created but could not be found.", {}, {}, {}};
    }

    // 3. Create the character with appearance and blood status.
    database::CharacterRecord charRecord;
    charRecord.userId = user->userId;
    charRecord.nickname = request.nickname;
    charRecord.house = request.house;
    charRecord.level = 1;
    charRecord.experience = 0;
    charRecord.gold = 50;           // initial pocket money from Gringotts
    charRecord.currentRoomId = 1;   // Great Hall
    charRecord.title = "First Year";
    charRecord.gender = request.gender;
    charRecord.hairColor = request.hairColor;
    charRecord.eyeColor = request.eyeColor;
    charRecord.bloodStatus = request.bloodStatus;
    if (!characterDao_->createCharacter(charRecord)) {
        return {false, "Failed to create your character. Please try again.", {}, {}, {}};
    }
    const auto character = characterDao_->getCharacterByUserId(user->userId);
    if (!character) {
        return {false, "Your character was created but could not be found.", {}, {}, {}};
    }

    // 4. Initialize the inventory: wand + pet + school robe.
    if (inventoryDao_) {
        // Wand (item_id = 1)
        inventoryDao_->addInventoryItem(
            database::InventoryRecord{0, character->characterId, 1, 1});
        // Pet
        std::uint64_t petItemId = 0;
        if (request.petType == "owl") petItemId = 8;
        else if (request.petType == "cat") petItemId = 9;
        else if (request.petType == "toad") petItemId = 10;
        if (petItemId != 0) {
            inventoryDao_->addInventoryItem(
                database::InventoryRecord{0, character->characterId, petItemId, 1});
        }
        // School Robe (item_id = 11)
        inventoryDao_->addInventoryItem(
            database::InventoryRecord{0, character->characterId, 11, 1});
    }

    // 5. Update last login and establish the session.
    (void)userDao_->updateLastLogin(user->userId);

    session_ = do_model::PlayerSessionDO{user->userId,
                                         character->characterId,
                                         character->nickname,
                                         character->house,
                                         "Great Hall",
                                         "Breakfast",
                                         true};
    return {true, "Welcome to Hogwarts! Your enrollment is complete. You received 50 Galleons, "
                  "a wand, a school robe, and your chosen pet.",
            session_->nickname, session_->house, session_->currentLocation};
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
