#include "LocationDAO.hpp"
#include <utility>

namespace arcane::database {

LocationDAO::LocationDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection)) {}

std::optional<PlayerLocationDO> LocationDAO::findByCharacterId(std::uint64_t characterId) const {
    const auto result = connection_->query(
        "SELECT character_id, room_name, x_pos, y_pos, is_online, last_seen "
        "FROM player_location WHERE character_id = ?",
        {std::to_string(characterId)});
    if (!result || result->rows.empty()) return std::nullopt;
    return parseRow(result->rows[0]);
}

std::vector<PlayerLocationDO> LocationDAO::findAllOnline() const {
    const auto result = connection_->query(
        "SELECT character_id, room_name, x_pos, y_pos, is_online, last_seen "
        "FROM player_location WHERE is_online = 1 ORDER BY last_seen DESC", {});
    std::vector<PlayerLocationDO> locations;
    if (!result) return locations;
    for (const auto& row : result->rows) locations.push_back(parseRow(row));
    return locations;
}

std::vector<PlayerLocationDO> LocationDAO::findOnlineByRoom(const std::string& roomName) const {
    const auto result = connection_->query(
        "SELECT character_id, room_name, x_pos, y_pos, is_online, last_seen "
        "FROM player_location WHERE is_online = 1 AND room_name = ?", {roomName});
    std::vector<PlayerLocationDO> locations;
    if (!result) return locations;
    for (const auto& row : result->rows) locations.push_back(parseRow(row));
    return locations;
}

bool LocationDAO::updatePosition(std::uint64_t characterId,
                                  const std::string& roomName,
                                  std::int32_t xPos, std::int32_t yPos) {
    return connection_->execute(
        "INSERT INTO player_location (character_id, room_name, x_pos, y_pos, is_online, last_seen) "
        "VALUES (?, ?, ?, ?, 1, NOW()) "
        "ON DUPLICATE KEY UPDATE room_name = VALUES(room_name), "
        "x_pos = VALUES(x_pos), y_pos = VALUES(y_pos), "
        "is_online = 1, last_seen = NOW()",
        {std::to_string(characterId), roomName, std::to_string(xPos), std::to_string(yPos)});
}

PlayerLocationDO LocationDAO::parseRow(const QueryRow& row) {
    PlayerLocationDO loc{};
    if (auto it = row.find("character_id"); it != row.end()) loc.characterId = std::stoull(it->second);
    if (auto it = row.find("room_name"); it != row.end())    loc.roomName = it->second;
    if (auto it = row.find("x_pos"); it != row.end())        loc.xPos = std::stoi(it->second);
    if (auto it = row.find("y_pos"); it != row.end())        loc.yPos = std::stoi(it->second);
    if (auto it = row.find("is_online"); it != row.end())    loc.isOnline = (it->second == "1");
    if (auto it = row.find("last_seen"); it != row.end())    loc.lastSeen = it->second;
    return loc;
}

} // namespace arcane::database
