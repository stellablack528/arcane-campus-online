#include "application/service/CampusService.hpp"

#include <unordered_map>

namespace arcane::application::service {
namespace {

const std::unordered_map<std::string, std::string> locations{
    {"great_hall", "Great Hall"}, {"transfiguration_classroom", "Transfiguration Classroom"},
    {"library", "Library"}, {"courtyard", "Courtyard"}, {"hogsmeade", "Hogsmeade"}};
const std::unordered_map<std::string, std::string> courses{
    {"transfiguration", "Transfiguration Classroom"}, {"charms", "Charms Classroom"},
    {"herbology", "Greenhouse Three"}, {"defense", "Defense Classroom"}};

} // namespace

vo::OperationResultVO CampusService::moveTo(do_model::PlayerSessionDO& session,
                                             const dto::LocationMoveRequestDTO& request) const
{
    const auto iterator = locations.find(request.locationId);
    if (iterator == locations.end()) {
        return {false, "That campus location is not available yet."};
    }
    session.currentLocation = iterator->second;
    session.currentState = request.locationId == "library" ? "Studying" : "Exploring";
    return {true, "You arrived at " + session.currentLocation + "."};
}

vo::OperationResultVO CampusService::joinCourse(do_model::PlayerSessionDO& session,
                                                 const dto::CourseActionRequestDTO& request) const
{
    const auto iterator = courses.find(request.courseId);
    if (iterator == courses.end()) {
        return {false, "That course is not on today's timetable."};
    }
    session.currentLocation = iterator->second;
    session.currentState = "Attending class";
    return {true, "You entered " + session.currentLocation + "."};
}

vo::OperationResultVO CampusService::describeCourse(const dto::CourseActionRequestDTO& request) const
{
    const auto iterator = courses.find(request.courseId);
    return iterator == courses.end()
        ? vo::OperationResultVO{false, "Course details are unavailable."}
        : vo::OperationResultVO{true, request.courseId + " is scheduled in " + iterator->second + "."};
}

vo::OperationResultVO CampusService::describeProfessor(const std::string& professorId) const
{
    return professorId.empty()
        ? vo::OperationResultVO{false, "Professor information is unavailable."}
        : vo::OperationResultVO{true, "Professor profile requested: " + professorId + "."};
}

vo::OperationResultVO CampusService::joinActivity(const std::string& activityId) const
{
    return activityId == "hogsmeade_weekend"
        ? vo::OperationResultVO{true, "The Hogsmeade weekend activity has been noted."}
        : vo::OperationResultVO{false, "That activity is unavailable."};
}

} // namespace arcane::application::service
