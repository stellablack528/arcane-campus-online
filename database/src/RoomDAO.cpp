#include "RoomDAO.hpp"
#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

RoomDAO::RoomDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool RoomDAO::createRoom(const RoomRecord& room)
{
    return connection_->execute(
        "INSERT INTO rooms (room_name, room_type, description, max_players) VALUES (?, ?, ?, ?)",
        {room.roomName, room.roomType, room.description, std::to_string(room.maxPlayers)});
}

std::optional<RoomRecord> RoomDAO::getRoomById(std::uint64_t roomId) const
{
    const auto result = connection_->query("SELECT * FROM rooms WHERE room_id = ?", {std::to_string(roomId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return RoomRecord{detail::integer<std::uint64_t>(row, "room_id"), detail::value(row, "room_name"),
                      detail::value(row, "room_type"), detail::value(row, "description"),
                      detail::integer<std::uint32_t>(row, "max_players")};
}

std::vector<RoomRecord> RoomDAO::getAllRooms() const
{
    const auto result = connection_->query("SELECT * FROM rooms ORDER BY room_id ASC");
    std::vector<RoomRecord> rooms;
    if (!result) {
        return rooms;
    }
    rooms.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        rooms.push_back(RoomRecord{detail::integer<std::uint64_t>(row, "room_id"),
                                   detail::value(row, "room_name"), detail::value(row, "room_type"),
                                   detail::value(row, "description"),
                                   detail::integer<std::uint32_t>(row, "max_players")});
    }
    return rooms;
}

bool RoomDAO::updateRoom(const RoomRecord& room)
{
    return connection_->execute(
        "UPDATE rooms SET room_name = ?, room_type = ?, description = ?, max_players = ? WHERE room_id = ?",
        {room.roomName,
         room.roomType,
         room.description,
         std::to_string(room.maxPlayers),
         std::to_string(room.roomId)});
}

bool RoomDAO::deleteRoom(std::uint64_t roomId)
{
    return connection_->execute("DELETE FROM rooms WHERE room_id = ?", {std::to_string(roomId)});
}

} // namespace arcane::database
