#include "CharacterDAO.hpp"

#include <utility>

namespace arcane::database {

CharacterDAO::CharacterDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool CharacterDAO::createCharacter(const CharacterRecord& character)
{
    return connection_->execute(
        "INSERT INTO characters (user_id, nickname, house, level, experience, gold, current_room_id, title) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        {std::to_string(character.userId),
         character.nickname,
         character.house,
         std::to_string(character.level),
         std::to_string(character.experience),
         std::to_string(character.gold),
         std::to_string(character.currentRoomId),
         character.title});
}

std::optional<CharacterRecord> CharacterDAO::getCharacterById(std::uint64_t characterId) const
{
    const auto result = connection_->query("SELECT * FROM characters WHERE character_id = ?",
                                           {std::to_string(characterId)});
    static_cast<void>(result);
    return std::nullopt;
}

bool CharacterDAO::updateCharacterRoom(std::uint64_t characterId, std::uint64_t roomId)
{
    return connection_->execute("UPDATE characters SET current_room_id = ? WHERE character_id = ?",
                                {std::to_string(roomId), std::to_string(characterId)});
}

bool CharacterDAO::deleteCharacter(std::uint64_t characterId)
{
    return connection_->execute("DELETE FROM characters WHERE character_id = ?",
                                {std::to_string(characterId)});
}

} // namespace arcane::database
