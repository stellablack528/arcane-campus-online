#pragma once

#include "application/do/CampusDO.hpp"
#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

namespace arcane::application::service {

class CampusService final {
public:
    [[nodiscard]] vo::OperationResultVO moveTo(do_model::PlayerSessionDO& session,
                                                const dto::LocationMoveRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO joinCourse(do_model::PlayerSessionDO& session,
                                                    const dto::CourseActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO describeCourse(const dto::CourseActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO describeProfessor(const std::string& professorId) const;
    [[nodiscard]] vo::OperationResultVO joinActivity(const std::string& activityId) const;
};

} // namespace arcane::application::service
