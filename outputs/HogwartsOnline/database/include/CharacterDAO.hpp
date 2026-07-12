#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace arcane::database {

struct CharacterRecord {
    std::uint64_t characterId = 0;
    std::uint64_t userId = 0;
    std::string nickname;
    std::string house;
    std::uint32_t level = 1;
    std::uint64_t experience = 0;
    std::uint64_t gold = 0;
    std::uint64_t currentRoomId = 0;
    std::string title;
    std::string createTime;
};

class CharacterDAO final {
public:
    explicit CharacterDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool createCharacter(const CharacterRecord& character);
    [[nodiscard]] std::optional<CharacterRecord> getCharacterById(std::uint64_t characterId) const;
    [[nodiscard]] bool updateCharacterRoom(std::uint64_t characterId, std::uint64_t roomId);
    [[nodiscard]] bool deleteCharacter(std::uint64_t characterId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
