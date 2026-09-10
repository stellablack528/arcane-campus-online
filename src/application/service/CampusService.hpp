#pragma once

#include "application/do/CampusDO.hpp"
#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>

namespace arcane::database {
class RoomDAO;
class NPCDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Manages campus navigation: moving between rooms, joining courses, and
// looking up professor information. When DAOs are injected, room and NPC
// data is read from the database; otherwise a small hardcoded table is used.
class CampusService final {
public:
    explicit CampusService(std::shared_ptr<database::RoomDAO> roomDao,
                           std::shared_ptr<database::NPCDAO> npcDao) noexcept;

    [[nodiscard]] vo::OperationResultVO moveTo(do_model::PlayerSessionDO& session,
                                                const dto::LocationMoveRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO joinCourse(do_model::PlayerSessionDO& session,
                                                    const dto::CourseActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO describeCourse(const dto::CourseActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO describeProfessor(const std::string& professorId) const;
    [[nodiscard]] vo::OperationResultVO joinActivity(const std::string& activityId) const;

private:
    // Helper: convert a UI location id (e.g. "great_hall") to a readable room name ("Great Hall").
    [[nodiscard]] static std::string locationIdToRoomName(const std::string& locationId);

    std::shared_ptr<database::RoomDAO> roomDao_;
    std::shared_ptr<database::NPCDAO> npcDao_;
};

} // namespace arcane::application::service
