#pragma once

#include "application/do/CampusDO.hpp"
#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>
#include <optional>

namespace arcane::database {
class UserDAO;
class CharacterDAO;
class InventoryDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Manages the active player session. When DAOs are injected, login is verified
// against the database; otherwise it falls back to an in-memory demo session.
class SessionService final {
public:
    explicit SessionService(std::shared_ptr<database::UserDAO> userDao,
                            std::shared_ptr<database::CharacterDAO> characterDao,
                            std::shared_ptr<database::InventoryDAO> inventoryDao) noexcept;

    // 验证已有用户登录。
    [[nodiscard]] vo::LoginVO enterCampus(const dto::LoginRequestDTO& request);

    // 注册新学生：创建用户 + 角色 + 初始化背包（魔杖+动物+校服+50金加隆）。
    [[nodiscard]] vo::LoginVO registerStudent(const dto::EnrollmentRequestDTO& request);

    [[nodiscard]] std::optional<do_model::PlayerSessionDO> currentSession() const;
    [[nodiscard]] do_model::PlayerSessionDO* activeSession() noexcept;
    void endSession() noexcept;

private:
    std::shared_ptr<database::UserDAO> userDao_;
    std::shared_ptr<database::CharacterDAO> characterDao_;
    std::shared_ptr<database::InventoryDAO> inventoryDao_;
    std::optional<do_model::PlayerSessionDO> session_;
};

} // namespace arcane::application::service
