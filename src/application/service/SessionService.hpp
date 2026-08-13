#pragma once

#include "application/do/CampusDO.hpp"
#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>
#include <optional>

namespace arcane::database {
class UserDAO;
class CharacterDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Manages the active player session. When DAOs are injected, login is verified
// against the database; otherwise it falls back to an in-memory demo session.
class SessionService final {
public:
    explicit SessionService(std::shared_ptr<database::UserDAO> userDao,
                            std::shared_ptr<database::CharacterDAO> characterDao) noexcept;

    [[nodiscard]] vo::LoginVO enterCampus(const dto::LoginRequestDTO& request);
    [[nodiscard]] std::optional<do_model::PlayerSessionDO> currentSession() const;
    [[nodiscard]] do_model::PlayerSessionDO* activeSession() noexcept;
    void endSession() noexcept;

private:
    std::shared_ptr<database::UserDAO> userDao_;
    std::shared_ptr<database::CharacterDAO> characterDao_;
    std::optional<do_model::PlayerSessionDO> session_;
};

} // namespace arcane::application::service
