#include "CharacterDAO.hpp"
#include <utility>

namespace arcane::database {

CharacterDAO::CharacterDAO(std::shared_ptr<DBConnection> connection) noexcept : connection_(std::move(connection)) {}

bool CharacterDAO::createCharacter(const CharacterRecord& c) {
    return connection_->execute(
        "INSERT INTO characters (user_id, nickname, house, level, experience, gold, current_room_id, title) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        {std::to_string(c.userId), c.nickname, c.house, std::to_string(c.level),
         std::to_string(c.experience), std::to_string(c.gold), std::to_string(c.currentRoomId), c.title});
}

std::optional<CharacterRecord> CharacterDAO::getCharacterById(std::uint64_t id) const {
    const auto result = connection_->query("SELECT * FROM characters WHERE character_id = ?", {std::to_string(id)});
    static_cast<void>(result); return std::nullopt;
}

bool CharacterDAO::updateCharacterRoom(std::uint64_t id, std::uint64_t roomId) {
    return connection_->execute("UPDATE characters SET current_room_id = ? WHERE character_id = ?",
                                {std::to_string(roomId), std::to_string(id)});
}

bool CharacterDAO::deleteCharacter(std::uint64_t id) {
    return connection_->execute("DELETE FROM characters WHERE character_id = ?", {std::to_string(id)});
}

} // namespace arcane::database
