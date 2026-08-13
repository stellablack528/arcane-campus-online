#include "LocationDAO.hpp"
#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

namespace {
inline std::string formatDouble(double value)
{
    std::string text = std::to_string(value);
    // Trim trailing zeros so MySQL receives a clean numeric string.
    if (text.find('.') != std::string::npos) {
        text.erase(text.find_last_not_of('0') + 1, std::string::npos);
        if (!text.empty() && text.back() == '.') {
            text.pop_back();
        }
    }
    return text;
}
} // namespace

LocationDAO::LocationDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool LocationDAO::upsertLocation(std::uint64_t characterId,
                                 std::uint64_t roomId,
                                 double posX,
                                 double posY,
                                 bool isOnline)
{
    return connection_->execute(
        "INSERT INTO player_locations (character_id, room_id, pos_x, pos_y, is_online) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON DUPLICATE KEY UPDATE room_id = VALUES(room_id), pos_x = VALUES(pos_x), "
        "pos_y = VALUES(pos_y), is_online = VALUES(is_online)",
        {std::to_string(characterId), std::to_string(roomId),
         formatDouble(posX), formatDouble(posY), isOnline ? "1" : "0"});
}

bool LocationDAO::setOnline(std::uint64_t characterId, bool isOnline)
{
    return connection_->execute(
        "UPDATE player_locations SET is_online = ? WHERE character_id = ?",
        {isOnline ? "1" : "0", std::to_string(characterId)});
}

std::vector<PlayerLocationRecord> LocationDAO::getOnlinePlayers() const
{
    std::vector<PlayerLocationRecord> records;
    const auto result = connection_->query(
        "SELECT pl.character_id, c.nickname, c.house, pl.room_id, r.room_name, "
        "pl.pos_x, pl.pos_y, pl.is_online, pl.last_update "
        "FROM player_locations pl "
        "JOIN characters c ON c.character_id = pl.character_id "
        "JOIN rooms r ON r.room_id = pl.room_id "
        "WHERE pl.is_online = 1 "
        "ORDER BY r.room_name, c.nickname",
        {});
    if (!result) {
        return records;
    }
    records.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        PlayerLocationRecord record;
        record.characterId = detail::integer<std::uint64_t>(row, "character_id");
        record.nickname = detail::value(row, "nickname");
        record.house = detail::value(row, "house");
        record.roomId = detail::integer<std::uint64_t>(row, "room_id");
        record.roomName = detail::value(row, "room_name");
        try {
            record.posX = std::stod(detail::value(row, "pos_x"));
            record.posY = std::stod(detail::value(row, "pos_y"));
        } catch (...) {
            record.posX = 0.0;
            record.posY = 0.0;
        }
        record.isOnline = detail::boolean(row, "is_online");
        record.lastUpdate = detail::value(row, "last_update");
        records.push_back(std::move(record));
    }
    return records;
}

std::vector<PlayerLocationRecord> LocationDAO::getOnlinePlayersInRoom(std::uint64_t roomId) const
{
    std::vector<PlayerLocationRecord> records;
    const auto result = connection_->query(
        "SELECT pl.character_id, c.nickname, c.house, pl.room_id, r.room_name, "
        "pl.pos_x, pl.pos_y, pl.is_online, pl.last_update "
        "FROM player_locations pl "
        "JOIN characters c ON c.character_id = pl.character_id "
        "JOIN rooms r ON r.room_id = pl.room_id "
        "WHERE pl.is_online = 1 AND pl.room_id = ? "
        "ORDER BY c.nickname",
        {std::to_string(roomId)});
    if (!result) {
        return records;
    }
    records.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        PlayerLocationRecord record;
        record.characterId = detail::integer<std::uint64_t>(row, "character_id");
        record.nickname = detail::value(row, "nickname");
        record.house = detail::value(row, "house");
        record.roomId = detail::integer<std::uint64_t>(row, "room_id");
        record.roomName = detail::value(row, "room_name");
        try {
            record.posX = std::stod(detail::value(row, "pos_x"));
            record.posY = std::stod(detail::value(row, "pos_y"));
        } catch (...) {
            record.posX = 0.0;
            record.posY = 0.0;
        }
        record.isOnline = detail::boolean(row, "is_online");
        record.lastUpdate = detail::value(row, "last_update");
        records.push_back(std::move(record));
    }
    return records;
}

std::optional<PlayerLocationRecord> LocationDAO::getPlayerLocation(std::uint64_t characterId) const
{
    const auto result = connection_->query(
        "SELECT pl.character_id, c.nickname, c.house, pl.room_id, r.room_name, "
        "pl.pos_x, pl.pos_y, pl.is_online, pl.last_update "
        "FROM player_locations pl "
        "JOIN characters c ON c.character_id = pl.character_id "
        "JOIN rooms r ON r.room_id = pl.room_id "
        "WHERE pl.character_id = ?",
        {std::to_string(characterId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    PlayerLocationRecord record;
    record.characterId = detail::integer<std::uint64_t>(row, "character_id");
    record.nickname = detail::value(row, "nickname");
    record.house = detail::value(row, "house");
    record.roomId = detail::integer<std::uint64_t>(row, "room_id");
    record.roomName = detail::value(row, "room_name");
    try {
        record.posX = std::stod(detail::value(row, "pos_x"));
        record.posY = std::stod(detail::value(row, "pos_y"));
    } catch (...) {
        record.posX = 0.0;
        record.posY = 0.0;
    }
    record.isOnline = detail::boolean(row, "is_online");
    record.lastUpdate = detail::value(row, "last_update");
    return record;
}

bool LocationDAO::deleteLocation(std::uint64_t characterId)
{
    return connection_->execute("DELETE FROM player_locations WHERE character_id = ?",
                                {std::to_string(characterId)});
}

} // namespace arcane::database
