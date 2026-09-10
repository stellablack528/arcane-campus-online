#include "application/service/CampusService.hpp"

#include "NPCDAO.hpp"
#include "RoomDAO.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <utility>

namespace arcane::application::service {
namespace {

// Fallback tables used in demo mode (no DAO wired in).
const std::unordered_map<std::string, std::string> fallbackLocations{
    {"great_hall", "Great Hall"}, {"transfiguration_classroom", "Transfiguration Classroom"},
    {"library", "Library"}, {"courtyard", "Courtyard"}, {"hogsmeade", "Hogsmeade"}};
const std::unordered_map<std::string, std::string> fallbackCourses{
    {"transfiguration", "Transfiguration Classroom"}, {"charms", "Charms Classroom"},
    {"herbology", "Greenhouse Three"}, {"defense", "Defense Classroom"}};

} // namespace

CampusService::CampusService(std::shared_ptr<database::RoomDAO> roomDao,
                             std::shared_ptr<database::NPCDAO> npcDao) noexcept
    : roomDao_(std::move(roomDao))
    , npcDao_(std::move(npcDao))
{
}

std::string CampusService::locationIdToRoomName(const std::string& locationId)
{
    // Convert "great_hall" → "Great Hall": replace underscores with spaces,
    // then capitalise the first letter of each word.
    std::string result = locationId;
    for (char& c : result) {
        if (c == '_') {
            c = ' ';
        }
    }
    bool capitalizeNext = true;
    for (char& c : result) {
        if (capitalizeNext && c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
        capitalizeNext = (c == ' ');
    }
    return result;
}

vo::OperationResultVO CampusService::moveTo(do_model::PlayerSessionDO& session,
                                             const dto::LocationMoveRequestDTO& request) const
{
    // Database-backed path: look up the room by its display name.
    if (roomDao_) {
        const auto roomName = locationIdToRoomName(request.locationId);
        const auto room = roomDao_->getRoomByName(roomName);
        if (!room) {
            return {false, "That campus location is not available yet."};
        }
        session.currentLocation = room->roomName;
        session.currentState = room->roomType == "study" ? "Studying" : "Exploring";
        return {true, "You arrived at " + session.currentLocation + "."};
    }

    // Fallback: use the hardcoded table.
    const auto iterator = fallbackLocations.find(request.locationId);
    if (iterator == fallbackLocations.end()) {
        return {false, "That campus location is not available yet."};
    }
    session.currentLocation = iterator->second;
    session.currentState = request.locationId == "library" ? "Studying" : "Exploring";
    return {true, "You arrived at " + session.currentLocation + "."};
}

vo::OperationResultVO CampusService::joinCourse(do_model::PlayerSessionDO& session,
                                                 const dto::CourseActionRequestDTO& request) const
{
    // Fallback only — courses are not yet stored in the database.
    const auto iterator = fallbackCourses.find(request.courseId);
    if (iterator == fallbackCourses.end()) {
        return {false, "That course is not on today's timetable."};
    }
    session.currentLocation = iterator->second;
    session.currentState = "Attending class";
    return {true, "You entered " + session.currentLocation + "."};
}

vo::OperationResultVO CampusService::describeCourse(const dto::CourseActionRequestDTO& request) const
{
    const auto iterator = fallbackCourses.find(request.courseId);
    return iterator == fallbackCourses.end()
        ? vo::OperationResultVO{false, "Course details are unavailable."}
        : vo::OperationResultVO{true, request.courseId + " is scheduled in " + iterator->second + "."};
}

vo::OperationResultVO CampusService::describeProfessor(const std::string& professorId) const
{
    if (professorId.empty()) {
        return {false, "Professor information is unavailable."};
    }

    // Database-backed path: look up the NPC by its numeric id.
    if (npcDao_) {
        try {
            const auto id = std::stoull(professorId);
            const auto npc = npcDao_->getNPCById(id);
            if (!npc) {
                return {false, "No professor found with that id."};
            }
            return {true, npc->npcName + " — " + npc->description};
        } catch (...) {
            return {false, "Professor information is unavailable."};
        }
    }

    return {true, "Professor profile requested: " + professorId + "."};
}

vo::OperationResultVO CampusService::joinActivity(const std::string& activityId) const
{
    return activityId == "hogsmeade_weekend"
        ? vo::OperationResultVO{true, "The Hogsmeade weekend activity has been noted."}
        : vo::OperationResultVO{false, "That activity is unavailable."};
}

} // namespace arcane::application::service
