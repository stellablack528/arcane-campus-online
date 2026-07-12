#include "RoomDAO.hpp"

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
    static_cast<void>(result);
    return std::nullopt;
}

std::vector<RoomRecord> RoomDAO::getAllRooms() const
{
    const auto result = connection_->query("SELECT * FROM rooms ORDER BY room_id ASC");
    static_cast<void>(result);
    return {};
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
